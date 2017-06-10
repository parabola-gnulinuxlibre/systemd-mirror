/***
  This file is part of systemd.

  Copyright 2015 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include <sys/mount.h>

#include <linux/magic.h>

#include "systemd-basic/alloc-util.h"
#include "systemd-basic/escape.h"
#include "systemd-basic/fd-util.h"
#include "systemd-basic/fileio.h"
#include "systemd-basic/fs-util.h"
#include "systemd-basic/label.h"
#include "systemd-basic/mkdir.h"
#include "systemd-basic/mount-util.h"
#include "systemd-basic/parse-util.h"
#include "systemd-basic/path-util.h"
#include "systemd-basic/set.h"
#include "systemd-basic/stat-util.h"
#include "systemd-basic/string-util.h"
#include "systemd-basic/strv.h"
#include "systemd-basic/user-util.h"
#include "systemd-basic/util.h"

#include "nspawn-mount.h"

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
        const char *hsrc = NULL, *hdst = NULL;
        CGMount *c, *ret;

        assert(mounts);
        assert(type >= 0 && type < _CGMOUNT_MAX);
        assert(src);
        assert(dst);

        hsrc = strdup(str);
        hdst = strdup(dst);
        if (!hsrc || !hdst) {
                free(hsrc);
                free(hdst);
                return NULL;
        }

        c = realloc_multiply(mounts->mounts, sizeof(CGMount), mounts->n + 1);
        if (!c)
                return NULL;

        mounts->mounts = c;
        ret = (mounts->mounts)[mounts->n];
        (mounts->n)++;

        *ret = (CGMount) {
                .type = type,
                .src = hsrc,
                .dst = hdst,
        };
        return ret;
}

static void cgmount_free_all(CGMounts mounts) {
        for (size_t i = 0; i < mounts.n; i++) {
                free(mounts.mounts[i].src);
                free(mounts.mounts[i].dst);
        }
        free(mounts.mounts);
}

DEFINE_TRIVIAL_CLEANUP_FUNC(CGMounts, cgmount_free_all);

/* Code that recieves CGMounts **************************************/

static int cgpath_count_procs(const char *cgpath) {
        char line[LINE_MAX];
        int n = 0;
        _cleanup_fclose_ FILE *procs = NULL;

        procs = fopen(root_prefixa(cgpath, "cgroup.procs"), "re");
        if (!procs)
                return -errno;

        FOREACH_LINE(line, procs, return -errno)
                n++;

        return n;
}

static int cgroup_setup_mount_cg(const char *mountpoint, const char *opts, const char *fstype, bool use_cgns, bool use_userns) {
        const char *cgpath;
        int r;

        /* First the base mount; this is always RW. */
        r = mount_verbose(LOG_ERR, "cgroup", mountpoint, fstype, MS_NOSUID|MS_NOEXEC|MS_NODEV, opts);
        if (r < 0)
                return r;

        /* A couple of things below need to know the abspath to our cgroup within the mountpoint; go ahead and figure
         * that out now. */
        if (use_cgns)
                cgpath = mountpoint;
        else {
                const char *scontroller, *state, *cgroup = NULL;
                size_t controller_len;
                FOREACH_WORD_SEPARATOR(scontroller, controller_len, opts, state) {
                        _cleanup_free_ const char *controller = strndup(scontroller, controller_len);
                        if (cg_pid_get_path2(controller, 0, &cgroup) == 0)
                                break;
                }
                if (!cgroup)
                        return log_error_errno(EBADMSG, "Failed to associate mounted cgroup hierarchy %s with numbered cgroup hierarchy");
        }

        /* If we are using a userns, then we always let it be RW, because we can count on the shifted root user to not
         * have access to the things that would make us want to mount it RO.  Otherwise, we give the container RW
         * access to its cgroups if it isn't sharing them with other processes (presumably, this is the systemd
         * controller and not really any others). */
        if ( use_userns || (cgpath_count_procs(cgpath) == 1)) {
                /* RW */
                if (!use_cgns) {
                        /* emulate cgns by mounting everything but our subcgroup RO */
                        r = mount_verbose(LOG_ERR, cgpath, cgpath, NULL, MS_BIND, NULL);
                        if (r < 0)
                                return r;
                        r = mount_verbose(LOG_ERR, NULL, mountpoint, NULL,
                                          MS_BIND|MS_REMOUNT|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_RDONLY, NULL);
                        if (r < 0)
                                return r;
                }
        } else {
                /* RO */
                r = mount_verbose(LOG_ERR, NULL, mountpoint, NULL,
                                  MS_BIND|MS_REMOUNT|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_RDONLY, NULL);
                if (r < 0)
                        return r;
        }

        return 0;
}

static int cgroup_setup_mount_one(CGMount m, bool use_cgns, bool use_userns, const char *selinux_apifs_context) {
        _cleanup_free_ char *options = NULL;
        char *dst;
        int r;

        dst = root_prefixa("/sys/fs/cgroup", dst);

        /* The checks here to see if things are already mounted are kind of primative.  Perhaps they should actually
         * check the statfs() f_type to verify that the thing mounted is what we want to be mounted (similar to
         * cgroup-util's detection logic)?  But I don't really understand the use-case for having any of these already
         * mounted, so I'm not sure if such increased strictness would be unwelcome. */

        switch (m.type) {
        case CGMOUNT_SYMLINK:
                (void) mkdir_parents(dst, 0755);
                return symlink_idempotent(m.src, dst);
        case CGMOUNT_TMPFS:
                r = path_is_mount_point(dst, AT_SYMLINK_FOLLOW);
                if (r < 0)
                        return log_error_errno(r, "Failed to determine if %s is mounted already: %m", dst);
                if (r > 0)
                        return 0;
                r = tmpfs_patch_options(m.src, 0, selinux_apifs_context, &options);
                if (r < 0)
                        return log_oom();
                return mount_verbose(LOG_ERR, /*name*/"tmpfs", dst, /*fstype*/"tmpfs",
                                     MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_STRICTATIME, options);
        case CGMOUNT_CGROUP1:
                r = path_is_mount_point(dst, 0);
                if (r < 0 && r != -ENOENT)
                        return log_error_errno(r, "Failed to determine if %s is mounted already: %m", dst);
                if (r > 0) {
                        if (access(prefix_roota(dst, "cgroup.procs")) >= 0)
                                return 0;
                        if (errno != ENOENT)
                                return log_error_errno(errno, "Failed to determine if mount point %s is a cgroup hierarchy: %m", dst);
                        return log_error_errno(EINVAL, "%s is already mounted but not a cgroup hierarchy. Refusing.", dst);
                }
                (void) mkdir_p(dst, 0755);
                return cgroup_setup_mount_cg(dst, m.src, "cgroup", use_cgns, use_userns);
        case CGMOUNT_CGROUP2:
                r = path_is_mount_point(dst, 0);
                if (r < 0 && r != -ENOENT)
                        return log_error_errno(r, "Failed to determine if %s is mounted already: %m", dst);
                if (r > 0) {
                        if (access(prefix_roota(dst, "cgroup.procs")) >= 0)
                                return 0;
                        if (errno != ENOENT)
                                return log_error_errno(errno, "Failed to determine if mount point %s is a cgroup hierarchy: %m", dst);
                        return log_error_errno(EINVAL, "%s is already mounted but not a cgroup hierarchy. Refusing.", dst);
                }
                (void) mkdir_p(dst, 0755);
                return cgroup_setup_mount_cg(dst, m.src, "cgroup2", use_cgns, use_userns);
        default:
                assert_not_reached("Invalid CGMount type");
                return -EINVAL;
        }
}

int cgroup_setup_mount(CGMounts mounts, bool use_cgns, bool use_userns, const char *selinux_apifs_context) {
        int r;

        for (size_t i = 0; i < mounts.n; i++) {
                r = cgroup_setup_mount_one(mounts.mounts[i], use_cgns, use_userns, selinux_apifs_context);
                if (r < 0)
                        return r;
        }

        return 0;
}

/* Code that creates CGMounts ***************************************/

static int cgroup_setup_enumerate_inherit(CGMounts *ret_mounts) {
        _cleanup_fclose_ FILE *proc_self_mountinfo = NULL;
        _cleanup_(cgmount_free_allp) CGMounts mounts = {};
        int r;

        proc_self_mountinfo = fopen("/proc/self/mountinfo", "re");
        if (!proc_self_mountinfo)
                return -errno;

        for (;;) {
                _cleanup_free_ char *escmountpoint = NULL, *mountpoint = NULL, *fstype = NULL, *superopts = NULL;
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

                if (!filename_is_valid(name))
                        continue;

                if (streq(fstype, "tmpfs"))
                        type = CGMOUNT_TMPFS;
                else if(streq(fstype, "cgroup"))
                        type = CGMOUNT_CGROUP1;
                else if (streq(fstype, "cgroup2"))
                        type = CGMOUNT_CGROUP2;
                else
                        continue;

                if (!cgmount_add(&mounts, type, superopts, name)) {
                        return -ENOMEM;
                }

                if (type == CGMOUNT_TMPFS) {
                        /* TODO */
                }
        }

        *ret_mounts = mounts;
        *ret_n = n;

        mounts.mounts = NULL;
        mounts.n = 0;

        return 0;
}

/* Retrieve a list of cgroup v1 hierarchies. */
static int get_v1_hierarchies(Set *subsystems) {
        _cleanup_fclose_ FILE *f = NULL;
        char line[LINE_MAX];

        assert(subsystems);

        f = fopen("/proc/self/cgroup", "re");
        if (!f)
                return errno == ENOENT ? -ESRCH : -errno;

        FOREACH_LINE(line, f, return -errno) {
                int r;
                char *e, *l, *p;

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

                p = strdup(l);
                if (!p)
                        return -ENOMEM;

                r = set_consume(subsystems, p);
                if (r < 0)
                        return r;
        }

        return 0;
}

static const char *controller_to_dirname(const char *controller) {
        const char *e;

        assert(controller);

        /* Converts a controller name to the directory name below
         * /sys/fs/cgroup/ we want to mount it to. Effectively, this
         * just cuts off the name= prefixed used for named
         * hierarchies, if it is specified. */

        e = startswith(controller, "name=");
        if (e)
                return e;

        return controller;
}

static int cgroup_setup_enumerate_sd(CGMounts *ret_mounts, CGroupMode outer_cgver, CGroupMode inner_cgver, bool use_cgns)  {
        CGMount *mounts = NULL;
        size_t n = 0;

        if (!cgmount_add(&mounts, &n, CGMOUNT_TMPFS, "mode=755", ""))
                return log_oom();

        /* There's no "good" reason to have the behavior be different based on use_cgns; it's there for historical
         * compatibility now.  Originally there was the !use_cgns logic, that ran in outer_child.  But then, when
         * use_cgns support was added, it needed to run in inner_child, and the old code wouldn't work in inner_child.
         * So, a second implementation worked in inner_child was added, for th use_cgns case.  Now they both run in
         * outer_child, and there's no good reason to still have two separate bits of logic.  But, I'm worried that
         * someone depends on the subtle differences of whether they SYSTEMD_NSPAWN_USE_CGNS or not. */

        if (use_cgns) {
                _cleanup_set_free_free_ Set *hierarchies = NULL;
                int r;

                /* If the outer_cgver == CGROUP_VER_2, then `hierarchies` will be empty, and this will be a no-op.
                 * That's OK. */
                hierarchies = set_new(&string_hash_ops);
                if (!)
                        return log_oom();
                r = get_v1_hierarchies(hierarchies);
                if (r < 0)
                        return log_error_errno(r, "Failed to determine cgroup hierarchies: %m");

                for (;;) {
                        _cleanup_free_ const char *hierarchy = NULL;
                        const char c;

                        hierarchy = set_steal_first(hierarchies);
                        if (!hierarchy)
                                break;

                        /* depending on inner_cgver or outer_cgver, we may need to override the name=systemd
                         * controller, so we handle that below. */
                        if (streq(hierarchy, "name=systemd"))
                                continue;

                        if (!cgmount_add(&mounts, &n, CGMOUNT_CGROUP1, hierarchy, controller_to_dirname(hierarchy)))
                                return log_oom();

                        /* When multiple hierarchies are co-mounted, make their
                         * constituting individual hierarchies a symlink to the
                         * co-mount.
                         */
                        c = hierarchy;
                        for (;;) {
                                _cleanup_free_ const char *controller = NULL;

                                r = extract_first_word(&c, &controller, ",", 0);
                                if (r < 0)
                                        return log_error_errno(r, "Failed to extract co-mounted cgroup controller: %m");
                                if (r == 0)
                                        break;

                                if (streq(controller, hierarchy))
                                        break;

                                if (!cgmount_add(&mounts, &n, CGMOUNT_SYMLINK, controller_to_dirname(hierarchy), controller_to_dirname(controller)))
                                        return log_oom();

                        }
                }
        } else if (outer_cgver != CGROUP_VER_2) { /* !use_cgns */
                _cleanup_set_free_free_ Set *controllers = NULL;
                int r;

                controllers = set_new(&string_hash_ops);
                if (!controllers)
                        return log_oom();

                r = cg_kernel_controllers(controllers);
                if (r < 0)
                        return log_error_errno(r, "Failed to determine cgroup controllers: %m");

                for (;;) {
                        _cleanup_free_ char *controller = NULL, *origin = NULL, *combined = NULL;

                        controller = set_steal_first(controllers);
                        if (!controller)
                                break;

                        r = readlink_malloc(origin, &combined);
                        if (r == -EINVAL) {
                                /* Not a symbolic link, but directly a single cgroup hierarchy */

                                if (!cgmount_add(&mounts, &n, CGMOUNT_CGROUP1, controller, controller))
                                        return log_oom();

                        } else if (r < 0)
                                return log_error_errno(r, "Failed to read link %s: %m", origin);
                        else {
                                _cleanup_free_ char *target = NULL;

                                /* A symbolic link, a combination of controllers in one hierarchy */

                                if (!filename_is_valid(combined)) {
                                        log_warning("Ignoring invalid combined hierarchy %s.", combined);
                                        continue;
                                }

                                if (!cgmount_add(&mounts, &n, CGMOUNT_CGROUP1, combined, combined))
                                        return log_oom();

                                if (!cgmount_add(&mounts, &n, CGMOUNT_SYMLINK, combined, controller))
                                        return log_oom();
                        }
                }
        }

        switch (inner_cgver) {
        case CGROUP_VER_1_SD:
                if (!cgmount_add(&mounts, &n, CGMOUNT_CGROUP1, "name=systemd", "systemd"))
                        log_oom();
        case CGROUP_VER_MIXED_SD232:
                if (!cgmount_add(&mounts, &n, CGMOUNT_CGROUP2, "", "systemd"))
                        log_oom();
        default:
                assert_not_reached("non-legacy cgroup version desired in legacy setup function");
                return -EINVAL;
        }

        *ret_mounts = mounts;
        *ret_n = n;

        return 0;
}

int cgroup_setup_enumerate(
                CGMounts *ret_mounts,
                CGroupMode outer_cgver, CGroupMode inner_cgver,
                bool use_cgns
) {
        switch (inner_cgver) {
        case CGROUP_VER_NONE:
                return 0;
        case CGROUP_VER_INHERIT:
                return cgroup_setup_enumerate_inherit(ret_mounts);
        case CGROUP_VER_2:
                if (!cgmount_add(ret_mounts, CGMOUNT_CGROUP2, "cgroup", ""))
                        return log_oom();
                return 0;
        case CGROUP_VER_1_SD:
        case CGROUP_VER_MIXED_SD232:
                return cgroup_setup_enumerate_sd(ret_mounts, outer_cgver, inner_cgver, use_cgns);
        default:
                assert_not_reached("Invalid cgroup ver requested");
                return -EINVAL;
        }
}
