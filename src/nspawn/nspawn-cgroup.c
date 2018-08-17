/* SPDX-License-Identifier: LGPL-2.1+ */

#include <sys/mount.h>
#include <libmount.h>

#include "alloc-util.h"
#include "dirent-util.h"
#include "escape.h"
#include "fd-util.h"
#include "fileio.h"
#include "fs-util.h"
#include "mkdir.h"
#include "mount-util.h"
#include "nspawn-cgroup.h"
#include "nspawn-mount.h"
#include "path-util.h"
#include "process-util.h"
#include "rm-rf.h"
#include "stdio-util.h"
#include "string-util.h"
#include "strv.h"
#include "user-util.h"
#include "util.h"

/* Code for managing the list of CGMounts ***************************/

typedef enum CGMountType {
        CGMOUNT_SYMLINK,
        CGMOUNT_TMPFS,
        CGMOUNT_CGROUP1,
        CGMOUNT_CGROUP2,
        _CGMOUNT_MAX
} CGMountType;

struct CGMount {
        CGMountType type;
        char *src;
        char *dst;
};

static CGMount *cgmount_add(CGMounts *mounts, CGMountType type, const char *src, const char *dst) {

        char *hsrc = NULL, *hdst = NULL;
        CGMount *c, *ret;

        assert(mounts);
        assert(type >= 0 && type < _CGMOUNT_MAX);
        assert(src);
        assert(dst);

        hsrc = strdup(src);
        hdst = strdup(dst);
        if (!hsrc || !hdst) {
                free(hsrc);
                free(hdst);
                return NULL;
        }

        c = reallocarray(mounts->mounts, mounts->n + 1, sizeof(CGMount));
        if (!c)
                return NULL;

        mounts->mounts = c;
        ret = &(mounts->mounts)[mounts->n];
        (mounts->n)++;

        *ret = (CGMount) {
                .type = type,
                .src = hsrc,
                .dst = hdst,
        };
        return ret;
}

void cgroup_free_mounts(CGMounts *mounts) {

        for (size_t i = 0; i < mounts->n; i++) {
                free(mounts->mounts[i].src);
                free(mounts->mounts[i].dst);
        }
        mounts->mounts = mfree(mounts->mounts);
        mounts->n = 0;
}

/* cgroup-util ******************************************************/

/* Similar to cg_pid_get_path_internal, but take a full list of mount options, rather than a single controller name. */
static int cgfile_get_cgroup(FILE *cgfile, const char *opts, char **ret_cgroup) {

        if (!opts) { /* cgroup-v2 */
                rewind(cgfile);
                return cg_pid_get_path_internal(NULL, cgfile, ret_cgroup);
        } else { /* cgroup-v1 */
                const char *scontroller, *state;
                size_t controller_len;
                FOREACH_WORD_SEPARATOR(scontroller, controller_len, opts, ",", state) {
                        _cleanup_free_ const char *controller = strndup(scontroller, controller_len);
                        rewind(cgfile);
                        if (cg_pid_get_path_internal(controller, cgfile, ret_cgroup) == 0)
                                break;
                }
                if (!*ret_cgroup)
                        return -EBADMSG;
                return 0;
        }
}

static int cgdir_attach(const char *cgdir, pid_t pid) {

        const char *filepath = NULL;
        char c[DECIMAL_STR_MAX(pid_t) + 2];

        assert(cgdir);
        assert(pid >= 0);

        filepath = strjoina(cgdir, "/cgroup.procs");

        if (pid == 0)
                pid = getpid_cached();

        xsprintf(c, PID_FMT "\n", pid);

        return write_string_file(filepath, c, 0);
}

static int cgdir_enable_all(const char *cgdir) {

        const char *filepath = NULL;
        _cleanup_fclose_ FILE *f = NULL;
        _cleanup_free_ char *controllers = NULL;
        const char *controller, *state;
        size_t controller_len;
        int r;

        filepath = strjoina(cgdir, "/cgroup.controllers", NULL);
        r = read_one_line_file(filepath, &controllers);
        if (r < 0)
                return r;

        FOREACH_WORD(controller, controller_len, controllers, state) {
                char s[controller_len+2];

                s[0] = '+';
                memcpy(&s[1], controller, controller_len);
                s[controller_len+1] = '\0';

                if (!f) {
                        filepath = strjoina(cgdir, "/cgroup.subtree_control", NULL);
                        f = fopen(filepath, "we");
                        if (!f) {
                                log_debug_errno(errno, "Failed to open cgroup.subtree_control file of %s: %m", cgdir);
                                break;
                        }
                }

                r = write_string_stream(f, s, 0);
                if (r < 0) {
                        log_debug_errno(r, "Failed to enable controller %s for %s: %m", &s[1], cgdir);
                        clearerr(f);
                }
        }

        return 0;
}

static int cgdir_chown(const char *path, uid_t uid_shift) {

        _cleanup_close_ int fd = -1;
        const char *fn;

        fd = open(path, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
        if (fd < 0)
                return -errno;

        FOREACH_STRING(fn,
                       ".",
                       "cgroup.clone_children",
                       "cgroup.controllers",
                       "cgroup.events",
                       "cgroup.procs",
                       "cgroup.stat",
                       "cgroup.subtree_control",
                       "cgroup.threads",
                       "notify_on_release",
                       "tasks")
                if (fchownat(fd, fn, uid_shift, uid_shift, 0) < 0)
                        log_full_errno(errno == ENOENT ? LOG_DEBUG :  LOG_WARNING, errno,
                                       "Failed to chown \"%s/%s\", ignoring: %m", path, fn);

        return 0;
}

/* cgroup_setup *****************************************************/

static int chown_cgroup(pid_t pid, uid_t uid_shift, const char *cg1sd_mountpoint, const char *cg2_mountpoint) {
        _cleanup_free_ char *cgroup = NULL;
        const char *cgfilename;
        _cleanup_fclose_ FILE *cgfile = NULL;
        int r;

        if (!cg1sd_mountpoint && !cg2_mountpoint)
                return 0;

        /* Determine the cgroup. Thanks to sync_cgroup(), we can get away with only doing this once. */
        cgfilename = procfs_file_alloca(pid, "cgroup");
        cgfile = fopen(cgfilename, "re");
        if (!cgfile)
                log_error_errno(errno, "chown container cgroup: Failed to get cgroup of the container: %m");
        r = cgfile_get_cgroup(cgfile, cg2_mountpoint ? NULL : "name=systemd", &cgroup);
        if (r < 0)
                return log_error_errno(r, "chown container cgroup: Failed to get cgroup of the container: %m");

        /* cgroup-v2 */
        if (cg2_mountpoint) {
                char *cgdir;

                cgdir = strjoina(cg2_mountpoint, cgroup);
                r = cgdir_chown(cgdir, uid_shift);
                if (r < 0)
                        return log_error_errno(r, "chown container cgroup: cgdir_chown(path=\"%s\", uid=" UID_FMT "): %m", cgdir, uid_shift);
        }

        /* cgroup-v1 name=systemd */
        if (cg1sd_mountpoint) {
                char *cgdir;

                cgdir = strjoina(cg1sd_mountpoint, cgroup);
                r = cgdir_chown(cgdir, uid_shift);
                if (r < 0)
                        return log_error_errno(r, "chown container cgroup: cgdir_chown(path=\"%s\", uid=" UID_FMT "): %m", cgdir, uid_shift);
        }

        return 0;
}

static int sync_cgroup(pid_t pid, uid_t uid_shift, const char *mountpoint) {
        _cleanup_free_ char *cgroup = NULL;
        const char *fn;
        int r;

        /* When the host uses the legacy cgroup setup, but the
         * container shall use the unified hierarchy, let's make sure
         * we copy the path from the name=systemd hierarchy into the
         * unified hierarchy. Similar for the reverse situation. */

        r = cg_pid_get_path(SYSTEMD_CGROUP_CONTROLLER, pid, &cgroup);
        if (r < 0)
                return log_error_errno(r, "sync host cgroup -> container cgroup: Failed to determine cgroup of the container: %m");

        /* If nspawn dies abruptly the cgroup hierarchy created below
         * its unit isn't cleaned up. So, let's remove it
         * https://github.com/systemd/systemd/pull/4223#issuecomment-252519810 */
        fn = strjoina(mountpoint, cgroup);
        (void) rm_rf(fn, REMOVE_ROOT|REMOVE_ONLY_DIRECTORIES);
        (void) mkdir_p(fn, 0755);

        r = cgdir_attach(fn, pid);
        if (r < 0)
                return log_error_errno(r, "sync host cgroup -> container cgroup: Failed to move process to %s: %m", fn);

        return 0;
}

static int create_subcgroup(pid_t pid, bool keep_unit, const char *cg1sd_mountpoint, const char *cg2_mountpoint) {

        /* In the unified hierarchy inner nodes may only contain subgroups, but not processes. Hence, if we running in
         * the unified hierarchy and the container does the same, and we did not create a scope unit for the container
         * move us and the container into two separate subcgroups.
         *
         * Moreover, container payloads such as systemd try to manage the cgroup they run in in full (i.e. including
         * its attributes), while the host systemd will only delegate cgroups for children of the cgroup created for a
         * delegation unit, instead of the cgroup itself. This means, if we'd pass on the cgroup allocated from the
         * host systemd directly to the payload, the host and payload systemd might fight for the cgroup
         * attributes. Hence, let's insert an intermediary cgroup to cover that case too.
         *
         * Note that we only bother with the main hierarchy here, not with any secondary ones. On the unified setup
         * that's fine because there's only one hiearchy anyway and controllers are enabled directly on it. On the
         * legacy setup, this is fine too, since delegation of controllers is generally not safe there, hence we won't
         * do it. */

        const char *cgfilename;
        _cleanup_fclose_ FILE *cgfile = NULL;
        _cleanup_free_ char *cgroup = NULL;
        int r;

        assert(pid > 1);

        if (!cg1sd_mountpoint && !cg2_mountpoint)
                return 0;

        cgfilename = procfs_file_alloca(pid, "cgroup");
        cgfile = fopen(cgfilename, "re");
        if (!cgfile)
                log_error_errno(errno, "create subcgroup: Failed to get cgroup of the container: %m");
        r = cgfile_get_cgroup(cgfile, cg2_mountpoint ? NULL : "name=systemd", &cgroup);
        if (r < 0)
                return log_error_errno(r, "create subcgroup: Failed to get cgroup of the container: %m");

        if (cg2_mountpoint) {
                const char *cgdir;
                const char *payload;

                cgdir = strjoina(cg2_mountpoint, cgroup);
                payload = strjoina(cgdir, "/payload");

                r = mkdir_errno_wrapper(payload, 0755);
                if (r < 0 && r != -EEXIST)
                        return log_error_errno(r, "create payload cgroup: Failed to create subcgroup %s: %m", payload);

                r = cgdir_attach(payload, pid);
                if (r < 0)
                        return log_error_errno(r, "create payload cgroup: Failed to move process to subcgroup %s: %m", payload);

                if (keep_unit) {
                        const char *supervisor;

                        supervisor = strjoina(cgdir, "/supervisor");

                        r = mkdir_errno_wrapper(supervisor, 0755);
                        if (r < 0 && r != -EEXIST)
                                return log_error_errno(r, "create supervisor cgroup: Failed to create subcgroup %s: %m", supervisor);

                        r = cgdir_attach(supervisor, 0);
                        if (r < 0)
                                return log_error_errno(r, "create supervisor cgroup: Failed to move process to subcgroup %s: %m", supervisor);
                }

                (void) cgdir_enable_all(cgdir);
        }

        if (cg1sd_mountpoint) {
                const char *cgdir;
                const char *payload;

                cgdir = strjoina(cg1sd_mountpoint, cgroup);
                payload = strjoina(cgdir, "/payload");

                r = mkdir_errno_wrapper(payload, 0755);
                if (r < 0 && r != -EEXIST)
                        return log_error_errno(r, "create payload cgroup: Failed to create subcgroup %s: %m", payload);

                r = cgdir_attach(payload, pid);
                if (r < 0)
                        return log_error_errno(r, "create payload cgroup: Failed to move process to subcgroup %s: %m", payload);

                if (keep_unit) {
                        const char *supervisor;

                        supervisor = strjoina(cgdir, "/supervisor");

                        r = mkdir_errno_wrapper(supervisor, 0755);
                        if (r < 0 && r != -EEXIST)
                                return log_error_errno(r, "create supervisor cgroup: Failed to create subcgroup %s: %m", supervisor);

                        r = cgdir_attach(supervisor, 0);
                        if (r < 0)
                                return log_error_errno(r, "create supervisor cgroup: Failed to move process to subcgroup %s: %m", supervisor);
                }
        }

        return 0;
}

static int cgroup_setup_internal(pid_t pid, CGroupUnified outer_cgver, CGroupUnified inner_cgver, uid_t uid_shift, bool keep_unit, const char *cg1sd_mountpoint, const char *cg2_mountpoint) {

        int r;

        if ((outer_cgver >= CGROUP_UNIFIED_SYSTEMD232) != (inner_cgver >= CGROUP_UNIFIED_SYSTEMD232)) {
                /* sync the name=systemd hierarchy with the unified hierarchy */
                r = sync_cgroup(pid, uid_shift, outer_cgver == CGROUP_UNIFIED_NONE ? cg2_mountpoint : cg1sd_mountpoint);
                if (r < 0)
                        return r;
        }

        r = create_subcgroup(pid, keep_unit, cg1sd_mountpoint, cg2_mountpoint);
        if (r < 0)
                return r;

        r = chown_cgroup(pid, uid_shift, cg1sd_mountpoint, cg2_mountpoint);
        if (r < 0)
                return r;

        return 0;
}

int cgroup_setup(pid_t pid, CGroupUnified outer_cgver, CGroupUnified inner_cgver, uid_t uid_shift, bool keep_unit) {

        /* The main purpose of this function is to properly "delegate" parts of the cgroup hierarchies to the container
         * (see doc/CGROUP_DELEGATION.md). In general, delegating cgroup-v1 hierarchies is *not safe*, so we
         * don't. However, systemd takes extra measures to make delegating the name=systemd hierarchy safe, so we make
         * an exception for it. */

        bool cg1sd_used, cg2_used;
        _cleanup_free_ char *cg2_mountpoint = NULL;
        _cleanup_free_ char *cg1sd_mountpoint = NULL;
        int r, q;

        /* For purposes of version comparison */
        if (inner_cgver == CGROUP_UNIFIED_INHERIT)
                inner_cgver = outer_cgver;

        cg1sd_used = inner_cgver == CGROUP_UNIFIED_NONE || inner_cgver == CGROUP_UNIFIED_SYSTEMD233;
        cg2_used = inner_cgver >= CGROUP_UNIFIED_SYSTEMD232; /* or ... */
        if (inner_cgver == CGROUP_UNIFIED_UNKNOWN) {
                _cleanup_fclose_ FILE *proc_self_mountinfo = NULL;

                proc_self_mountinfo = fopen("/proc/self/mountinfo", "re");
                if (!proc_self_mountinfo)
                        return -errno;

                for (;;) {
                        _cleanup_free_ char *escmountpoint = NULL, *mountpoint = NULL, *fstype = NULL;
                        int k;

                        k = fscanf(proc_self_mountinfo,
                                   "%*s "       /* (1) mount id */
                                   "%*s "       /* (2) parent id */
                                   "%*s "       /* (3) major:minor */
                                   "%*s "       /* (4) root */
                                   "%ms "       /* (5) mount point */
                                   "%*s"        /* (6) per-mount options */
                                   "%*[^-]"     /* (7) optional fields */
                                   "- "         /* (8) separator */
                                   "%ms "       /* (9) file system type */
                                   "%*s"        /* (10) mount source */
                                   "%*s"        /* (11) per-superblock options */
                                   "%*[^\n]",   /* some rubbish at the end */
                                   &escmountpoint,
                                   &fstype
                                   );
                        if (k != 2) {
                                if (k == EOF)
                                        break;
                                continue;
                        }

                        r = cunescape(escmountpoint, UNESCAPE_RELAX, &mountpoint);
                        if (r < 0)
                                return r;

                        if (path_startswith(mountpoint, "/sys/fs/cgroup") && streq(fstype, "cgroup2")) {
                                cg2_used = true;
                                cg2_mountpoint = strdup(mountpoint);
                        }
                }
        } else if (cg2_used) {
                switch (outer_cgver) {
                case CGROUP_UNIFIED_SYSTEMD233: cg2_mountpoint = strdup("/sys/fs/cgroup/unified"); break;
                case CGROUP_UNIFIED_SYSTEMD232: cg2_mountpoint = strdup("/sys/fs/cgroup/systemd"); break;
                case CGROUP_UNIFIED_ALL:        cg2_mountpoint = strdup("/sys/fs/cgroup");         break;
                case CGROUP_UNIFIED_NONE:
                        cg2_mountpoint = strdup("/tmp/container-cg2-XXXXXX");
                        if (!mkdtemp(cg2_mountpoint))
                                return log_error_errno(errno, "Failed to create temporary mount point for container cgroup hierarchy: %m");
                        r = mount_verbose(LOG_ERR, "cgroup", cg2_mountpoint, "cgroup2",
                                          MS_NOSUID|MS_NOEXEC|MS_NODEV, NULL);
                        if (r < 0) {
                                log_error("Failed to mount container cgroup hierarchy");
                                (void) rmdir(cg2_mountpoint);
                                return r;
                        }
                        break;
                default:
                case CGROUP_UNIFIED_UNKNOWN:
                        assert_not_reached("invalid outer_cgver");
                }
        }
        if (cg1sd_used) {
                switch (outer_cgver) {
                case CGROUP_UNIFIED_NONE:
                case CGROUP_UNIFIED_SYSTEMD233:
                        cg1sd_mountpoint = strdup("/sys/fs/cgroup/systemd");
                        break;
                case CGROUP_UNIFIED_SYSTEMD232:
                case CGROUP_UNIFIED_ALL:
                        cg1sd_mountpoint = strdup("/tmp/container-cg1sd-XXXXXX");
                        if (!mkdtemp(cg1sd_mountpoint))
                                return log_error_errno(errno, "Failed to create temporary mount point for container cgroup hierarchy: %m");
                        r = mount_verbose(LOG_ERR, "cgroup", cg1sd_mountpoint, "cgroup",
                                          MS_NOSUID|MS_NOEXEC|MS_NODEV, "none,name=systemd,xattr");
                        if (r < 0) {
                                log_error("Failed to mount container cgroup hierarchy");
                                (void) rmdir(cg1sd_mountpoint);
                                return r;
                        }
                        break;
                default:
                case CGROUP_UNIFIED_UNKNOWN:
                        assert_not_reached("can't use legacy/hybrid container on unknown host");
                        break;
                }
        }

        r = cgroup_setup_internal(pid, outer_cgver, inner_cgver, uid_shift, keep_unit,
                                  cg1sd_used ? cg1sd_mountpoint : NULL, cg2_used ? cg2_mountpoint : NULL);

        if (cg2_used && startswith(cg2_mountpoint, "/tmp")) {
                q = umount_verbose(cg2_mountpoint);
                if (r >= 0 && q < 0)
                        r = q;
                (void) rmdir(cg2_mountpoint);
        }
        if (cg1sd_used && startswith(cg1sd_mountpoint, "/tmp")) {
                q = umount_verbose(cg1sd_mountpoint);
                if (r >= 0 && q < 0)
                        r = q;
                (void) rmdir(cg1sd_mountpoint);
        }

        return r;
}

/* cgroup_decide_mounts *********************************************/

static int cgroup_decide_mounts_inherit(CGMounts *ret_mounts) {
        _cleanup_fclose_ FILE *proc_self_mountinfo = NULL;
        _cleanup_(cgroup_free_mounts) CGMounts mounts = {};
        int r;

        proc_self_mountinfo = fopen("/proc/self/mountinfo", "re");
        if (!proc_self_mountinfo)
                return -errno;

        for (;;) {
                _cleanup_free_ char *escmountpoint = NULL, *mountpoint = NULL, *fstype = NULL, *superopts = NULL, *fsopts = NULL;
                char *name;
                CGMountType type;
                int k;

                k = fscanf(proc_self_mountinfo,
                           "%*s "       /* (1) mount id */
                           "%*s "       /* (2) parent id */
                           "%*s "       /* (3) major:minor */
                           "%*s "       /* (4) root */
                           "%ms "       /* (5) mount point */
                           "%*s"        /* (6) per-mount options */
                           "%*[^-]"     /* (7) optional fields */
                           "- "         /* (8) separator */
                           "%ms "       /* (9) file system type */
                           "%*s"        /* (10) mount source */
                           "%ms"        /* (11) per-superblock options */
                           "%*[^\n]",   /* some rubbish at the end */
                           &escmountpoint,
                           &fstype,
                           &superopts
                );
                if (k != 3) {
                        if (k == EOF)
                                break;

                        continue;
                }

                r = cunescape(escmountpoint, UNESCAPE_RELAX, &mountpoint);
                if (r < 0)
                        return r;

                name = path_startswith(mountpoint, "/sys/fs/cgroup");
                if (!name)
                        continue;

                if (!filename_is_valid(name) && !isempty(name))
                        continue;

                if (streq(fstype, "tmpfs"))
                        type = CGMOUNT_TMPFS;
                else if(streq(fstype, "cgroup"))
                        type = CGMOUNT_CGROUP1;
                else if (streq(fstype, "cgroup2")) {
                        type = CGMOUNT_CGROUP2;
                } else
                        continue;

                r = mnt_split_optstr(superopts, NULL, NULL, &fsopts, 0, 0);
                if (r < 0)
                        return r;

                if (!cgmount_add(&mounts, type, fsopts, name)) {
                        return -ENOMEM;
                }

                if (type == CGMOUNT_TMPFS) {
                        _cleanup_closedir_ DIR *dir;
                        struct dirent *entry;

                        dir = opendir(mountpoint);
                        if (!dir)
                                return log_error_errno(errno, "Failed to open directory %s: %m", mountpoint);

                        FOREACH_DIRENT(entry, dir, break) {
                                _cleanup_free_ char *target = NULL;
                                r = dirent_ensure_type(dir, entry);
                                if (r < 0)
                                        return r;
                                if (entry->d_type != DT_LNK)
                                        continue;
                                r = readlinkat_malloc(dirfd(dir), entry->d_name, &target);
                                if (r < 0)
                                        return r;
                                if (!cgmount_add(&mounts, CGMOUNT_SYMLINK, target, entry->d_name))
                                        return -ENOMEM;
                        }
                }
        }

        *ret_mounts = mounts;
        mounts.mounts = NULL;
        mounts.n = 0;

        return 0;
}

/* Retrieve a list of cgroup v1 hierarchies. */
static int get_v1_hierarchies(Set **ret) {
        _cleanup_set_free_free_ Set *controllers = NULL;
        _cleanup_fclose_ FILE *f = NULL;
        int r;

        assert(ret);

        controllers = set_new(&string_hash_ops);
        if (!controllers)
                return -ENOMEM;

        f = fopen("/proc/self/cgroup", "re");
        if (!f)
                return errno == ENOENT ? -ESRCH : -errno;

        for (;;) {
                _cleanup_free_ char *line = NULL;
                char *e, *l;

                r = read_line(f, LONG_LINE_MAX, &line);
                if (r < 0)
                        return r;
                if (r == 0)
                        break;

                l = strchr(line, ':');
                if (!l)
                        continue;

                l++;
                e = strchr(l, ':');
                if (!e)
                        continue;

                *e = 0;

                if (streq(l, ""))
                        continue;

                r = set_put_strdup(controllers, l);
                if (r < 0)
                        return r;
        }

        *ret = TAKE_PTR(controllers);

        return 0;
}

/* Decide the legacy cgroup mounts when cgroup namespaces are used. */
static int cgroup_decide_mounts_sd_y_cgns(
                CGMounts *ret_mounts,
                CGroupUnified outer_cgver, CGroupUnified inner_cgver) {

        _cleanup_(cgroup_free_mounts) CGMounts mounts = {};
        _cleanup_set_free_free_ Set *hierarchies = NULL;
        const char *c;
        int r;

        /* Mount a tmpfs to /sys/fs/cgroup if it's not mounted there yet. */
        if (!cgmount_add(&mounts, CGMOUNT_TMPFS, "mode=755", ""))
                return log_oom();

        if (outer_cgver >= CGROUP_UNIFIED_ALL)
                goto skip_controllers;

        r = get_v1_hierarchies(&hierarchies);
        if (r < 0)
                return log_error_errno(r, "Failed to determine cgroup hierarchies: %m");

        for (;;) {
                _cleanup_free_ const char *hierarchy = NULL;

                hierarchy = set_steal_first(hierarchies);
                if (!hierarchy)
                        break;

                if (streq(hierarchy, "name=systemd"))
                        continue;

                if (!cgmount_add(&mounts, CGMOUNT_CGROUP1, hierarchy, hierarchy))
                        return log_oom();

                /* When multiple hierarchies are co-mounted, make their
                 * constituting individual hierarchies a symlink to the
                 * co-mount.
                 */
                c = hierarchy;
                for (;;) {
                        _cleanup_free_ char *target = NULL, *controller = NULL;

                        r = extract_first_word(&c, &controller, ",", 0);
                        if (r < 0)
                                return log_error_errno(r, "Failed to extract co-mounted cgroup controller: %m");
                        if (r == 0)
                                break;

                        if (!cgmount_add(&mounts, CGMOUNT_SYMLINK, hierarchy, controller))
                                return log_oom();
                }
        }

skip_controllers:
        switch (inner_cgver) {
        default:
        case CGROUP_UNIFIED_UNKNOWN:
                assert_not_reached("unknown inner_cgver");
        case CGROUP_UNIFIED_ALL:
                assert_not_reached("cgroup v2 requested in cgroup v1 function");
        case CGROUP_UNIFIED_SYSTEMD232:
                if (!cgmount_add(&mounts, CGMOUNT_CGROUP2, "", "systemd"))
                        return log_oom();
                break;
        case CGROUP_UNIFIED_SYSTEMD233:
                if (!cgmount_add(&mounts, CGMOUNT_CGROUP2, "", "unified"))
                        return log_oom();
                _fallthrough_;
        case CGROUP_UNIFIED_NONE:
                if (!cgmount_add(&mounts, CGMOUNT_CGROUP1, "none,name=systemd,xattr", "systemd"))
                        return log_oom();
                break;
        }

        *ret_mounts = mounts;
        mounts = (CGMounts){};

        return 0;
}

/* Decide the legacy cgroup mounts when cgroup namespaces are not used. */
static int cgroup_decide_mounts_sd_n_cgns(
                CGMounts *ret_mounts,
                CGroupUnified outer_cgver, CGroupUnified inner_cgver) {

        _cleanup_(cgroup_free_mounts) CGMounts mounts = {};
        _cleanup_set_free_free_ Set *controllers = NULL;
        int r;

        /* Mount a tmpfs to /sys/fs/cgroup if it's not mounted there yet. */
        if (!cgmount_add(&mounts, CGMOUNT_TMPFS, "mode=755", ""))
                return log_oom();

        if (outer_cgver >= CGROUP_UNIFIED_ALL)
                goto skip_controllers;

        r = cg_kernel_controllers(&controllers);
        if (r < 0)
                return log_error_errno(r, "Failed to determine cgroup controllers: %m");

        for (;;) {
                _cleanup_free_ char *controller = NULL, *origin = NULL, *combined = NULL;

                controller = set_steal_first(controllers);
                if (!controller)
                        break;

                origin = prefix_root("/sys/fs/cgroup/", controller);
                if (!origin)
                        return log_oom();

                r = readlink_malloc(origin, &combined);
                if (r == -EINVAL) {
                        /* Not a symbolic link, but directly a single cgroup hierarchy */

                        if (!cgmount_add(&mounts, CGMOUNT_CGROUP1, controller, controller))
                                return log_oom();

                } else if (r < 0)
                        return log_error_errno(r, "Failed to read link %s: %m", origin);
                else {
                        /* A symbolic link, a combination of controllers in one hierarchy */

                        if (!filename_is_valid(combined)) {
                                log_warning("Ignoring invalid combined hierarchy %s.", combined);
                                continue;
                        }

                        if (!cgmount_add(&mounts, CGMOUNT_CGROUP1, combined, combined))
                                return log_oom();

                        if (!cgmount_add(&mounts, CGMOUNT_SYMLINK, combined, controller))
                                return log_oom();
                }
        }

skip_controllers:
        switch (inner_cgver) {
        default:
        case CGROUP_UNIFIED_UNKNOWN:
                assert_not_reached("unknown inner_cgver");
        case CGROUP_UNIFIED_ALL:
                assert_not_reached("cgroup v2 requested in cgroup v1 function");
        case CGROUP_UNIFIED_SYSTEMD232:
                if (!cgmount_add(&mounts, CGMOUNT_CGROUP2, "", "systemd"))
                        return log_oom();
                break;
        case CGROUP_UNIFIED_SYSTEMD233:
                if (!cgmount_add(&mounts, CGMOUNT_CGROUP2, "", "unified"))
                        return log_oom();
                _fallthrough_;
        case CGROUP_UNIFIED_NONE:
                if (!cgmount_add(&mounts, CGMOUNT_CGROUP1, "none,name=systemd,xattr", "systemd"))
                        return log_oom();
                break;
        }

        *ret_mounts = mounts;
        mounts = (CGMounts){};

        return 0;
}

int cgroup_decide_mounts(
                CGMounts *ret_mounts,
                CGroupUnified outer_cgver, CGroupUnified inner_cgver,
                bool use_cgns) {

        switch (inner_cgver) {
        default:
        case CGROUP_UNIFIED_UNKNOWN:
                assert_not_reached("unknown inner_cgver");
        case CGROUP_UNIFIED_INHERIT:
                return cgroup_decide_mounts_inherit(ret_mounts);
        case CGROUP_UNIFIED_NONE:
        case CGROUP_UNIFIED_SYSTEMD232:
        case CGROUP_UNIFIED_SYSTEMD233:
                /* Historically, if use_cgns, then this ran inside the container; if !use_cgns, then it ran outside.
                 * The use_cgns one had to be added because running inside the container, it couldn't look at the host
                 * '/sys'.  Now that they both run outside the container again, I'm afraid to unify them because I'm
                 * worried that someone depends on the subtle differences in their behavior. */
                if (use_cgns)
                        return cgroup_decide_mounts_sd_y_cgns(ret_mounts, outer_cgver, inner_cgver);
                else
                        return cgroup_decide_mounts_sd_n_cgns(ret_mounts, outer_cgver, inner_cgver);
        case CGROUP_UNIFIED_ALL:
                if (!cgmount_add(ret_mounts, CGMOUNT_CGROUP2, "cgroup", ""))
                        return log_oom();
                return 0;
        }
}

/* cgroup_mount_mounts **********************************************/

static int cgroup_mount_cg(
                const char *mountpoint, const char *opts, CGMountType fstype,
                FILE *cgfile, bool use_userns) {

        const bool use_cgns = cgfile == NULL;
        /* If we are using userns and cgns, then we always let it be RW, because we can count on the shifted root user
         * to not have access to the things that would make us want to mount it RO.  Otherwise, we only give the
         * container RW access to its unified or name=systemd cgroup. */
        const bool rw = (use_userns && use_cgns) || fstype == CGMOUNT_CGROUP2 || streq(mountpoint, "/sys/fs/cgroup/systemd");

        int r;

        r = mount_verbose(LOG_ERR, "cgroup", mountpoint, fstype == CGMOUNT_CGROUP1 ? "cgroup" : "cgroup2",
                          MS_NOSUID|MS_NOEXEC|MS_NODEV| ((!rw||!use_cgns) ? MS_RDONLY : 0), opts);
        if (r < 0)
                return r;

        if (rw && !use_cgns) {
                /* emulate cgns by mounting everything but our subcgroup RO */
                const char *rwmountpoint = strjoina(mountpoint, ".");

                _cleanup_free_ char *cgroup = NULL;
                r = cgfile_get_cgroup(cgfile, fstype == CGMOUNT_CGROUP2 ? NULL : opts, &cgroup);
                if (r < 0) {
                        if (fstype == CGMOUNT_CGROUP2)
                                return log_error_errno(r, "Failed to get child's cgroup v2 path");
                        else
                                return log_error_errno(r, "Failed to associate mounted cgroup hierarchy %s with numbered cgroup hierarchy", mountpoint);
                }

                (void) mkdir(rwmountpoint, 0755);
                r = mount_verbose(LOG_ERR, "cgroup", rwmountpoint, fstype == CGMOUNT_CGROUP1 ? "cgroup" : "cgroup2",
                                  MS_NOSUID|MS_NOEXEC|MS_NODEV, opts);
                if (r < 0)
                        return r;
                r = mount_verbose(LOG_ERR, strjoina(rwmountpoint, cgroup), strjoina(mountpoint, cgroup), NULL, MS_BIND, NULL);
                if (r < 0)
                        return r;
                r = umount_verbose(rwmountpoint);
                if (r < 0)
                        return r;
                r = rmdir(rwmountpoint);
                if (r < 0)
                        return log_error_errno(r, "Failed to rmdir temporary mountpoint %s: %m", rwmountpoint);
        }

        return 0;
}

int cgroup_mount_mounts(
                CGMounts m,
                FILE *cgfile,
                uid_t uid_shift,
                const char *selinux_apifs_context) {

        const bool use_cgns = cgfile == NULL;
        const bool use_userns = uid_shift != UID_INVALID;
        const char *cgroup_root = "/sys/fs/cgroup";

        bool used_tmpfs = false;

        for (size_t i = 0; i < m.n; i++) {
                _cleanup_free_ char *options = NULL;
                const char *dst;
                int r;

                dst = prefix_roota(cgroup_root, m.mounts[i].dst);

                /* The checks here to see if things are already mounted are kind of primative.  Perhaps they should
                 * actually check the statfs() f_type to verify that the thing mounted is what we want to be mounted
                 * (similar to cgroup-util's detection logic)?  But I don't really understand the use-case for having
                 * any of these already mounted, so I'm not sure if such increased strictness would be unwelcome. */

                switch (m.mounts[i].type) {
                case CGMOUNT_SYMLINK:
                        (void) mkdir_parents(dst, 0755);
                        r = symlink_idempotent(m.mounts[i].src, dst);
                        if (r < 0)
                                return r;
                        break;
                case CGMOUNT_TMPFS:
                        used_tmpfs = true;
                        r = path_is_mount_point(dst, NULL, path_equal(cgroup_root, dst) ? AT_SYMLINK_FOLLOW : 0);
                        if (r < 0)
                                return log_error_errno(r, "Failed to determine if %s is mounted already: %m", dst);
                        if (r > 0)
                                continue;
                        r = tmpfs_patch_options(m.mounts[i].src, uid_shift, selinux_apifs_context, &options);
                        if (r < 0)
                                return log_oom();
                        r = mount_verbose(LOG_ERR, /*name*/"tmpfs", dst, /*fstype*/"tmpfs",
                                          MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_STRICTATIME, options);
                        if (r < 0)
                                return r;
                        break;
                case CGMOUNT_CGROUP1:
                case CGMOUNT_CGROUP2:
                        r = path_is_mount_point(dst, NULL, path_equal(cgroup_root, dst) ? AT_SYMLINK_FOLLOW : 0);
                        if (r < 0 && r != -ENOENT)
                                return log_error_errno(r, "Failed to determine if %s is mounted already: %m", dst);
                        if (r > 0) {
                                if (access(prefix_roota(dst, "cgroup.procs"), F_OK) >= 0)
                                        continue;
                                if (errno != ENOENT)
                                        return log_error_errno(errno, "Failed to determine if mount point %s is a cgroup hierarchy: %m", dst);
                                return log_error_errno(EINVAL, "%s is already mounted but not a cgroup hierarchy. Refusing.", dst);
                        }
                        (void) mkdir_p(dst, 0755);
                        r = cgroup_mount_cg(dst, m.mounts[i].src, m.mounts[i].type, cgfile, use_userns);
                        if (r < 0)
                                return r;
                        break;
                default:
                        assert_not_reached("Invalid CGMount type");
                        return -EINVAL;
                }
        }

        /* I'm going to be honest: I don't understand why we don't do this if we're using both userns and cgns. */
        if (used_tmpfs && (!use_userns || !use_cgns))
                return mount_verbose(LOG_ERR, NULL, "/sys/fs/cgroup", NULL,
                                     MS_REMOUNT|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_STRICTATIME|MS_RDONLY, "mode=755");

        return 0;
}
