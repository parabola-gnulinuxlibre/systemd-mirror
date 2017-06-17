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

#include "alloc-util.h"
#include "fd-util.h"
#include "fileio.h"
#include "fs-util.h"
#include "mkdir.h"
#include "mount-util.h"
#include "path-util.h"
#include "rm-rf.h"
#include "string-util.h"
#include "strv.h"
#include "user-util.h"
#include "util.h"

#include "nspawn-cgroup.h"
#include "nspawn-mount.h"

static int chown_cgroup_path(const char *path, uid_t uid_shift) {
        _cleanup_close_ int fd = -1;
        const char *fn;

        fd = open(path, O_RDONLY|O_CLOEXEC|O_DIRECTORY);
        if (fd < 0)
                return -errno;

        FOREACH_STRING(fn,
                       ".",
                       "tasks",
                       "notify_on_release",
                       "cgroup.procs",
                       "cgroup.events",
                       "cgroup.clone_children",
                       "cgroup.controllers",
                       "cgroup.subtree_control")
                if (fchownat(fd, fn, uid_shift, uid_shift, 0) < 0)
                        log_full_errno(errno == ENOENT ? LOG_DEBUG :  LOG_WARNING, errno,
                                       "Failed to chown \"%s/%s\", ignoring: %m", path, fn);

        return 0;
}

static int chown_cgroup(pid_t pid, uid_t uid_shift) {
        _cleanup_free_ char *path = NULL, *fs = NULL;
        int r;

        r = cg_pid_get_path(NULL, pid, &path);
        if (r < 0)
                return log_error_errno(r, "Failed to get container cgroup path: %m");

        r = cg_get_path(SYSTEMD_CGROUP_CONTROLLER, path, NULL, &fs);
        if (r < 0)
                return log_error_errno(r, "Failed to get file system path for container cgroup: %m");

        r = chown_cgroup_path(fs, uid_shift);
        if (r < 0)
                return log_error_errno(r, "Failed to chown() cgroup %s: %m", fs);

        return 0;
}

static int sync_cgroup(pid_t pid, CGroupUnified outer_cgver, CGroupUnified inner_cgver, uid_t uid_shift) {
        _cleanup_free_ char *cgroup = NULL;
        char tree[] = "/tmp/unifiedXXXXXX", pid_string[DECIMAL_STR_MAX(pid) + 1];
        bool undo_mount = false;
        const char *fn;
        int r;

        if ((outer_cgver >= CGROUP_UNIFIED_SYSTEMD232) == (inner_cgver >= CGROUP_UNIFIED_SYSTEMD232))
                return 0;

        /* When the host uses the legacy cgroup setup, but the
         * container shall use the unified hierarchy, let's make sure
         * we copy the path from the name=systemd hierarchy into the
         * unified hierarchy. Similar for the reverse situation. */

        r = cg_pid_get_path(SYSTEMD_CGROUP_CONTROLLER, pid, &cgroup);
        if (r < 0)
                return log_error_errno(r, "Failed to get control group of " PID_FMT ": %m", pid);

        /* In order to access the unified hierarchy we need to mount it */
        if (!mkdtemp(tree))
                return log_error_errno(errno, "Failed to generate temporary mount point for unified hierarchy: %m");

        if (outer_cgver >= CGROUP_UNIFIED_SYSTEMD232)
                r = mount_verbose(LOG_ERR, "cgroup", tree, "cgroup",
                                  MS_NOSUID|MS_NOEXEC|MS_NODEV, "none,name=systemd,xattr");
        else
                r = mount_verbose(LOG_ERR, "cgroup", tree, "cgroup2",
                                  MS_NOSUID|MS_NOEXEC|MS_NODEV, NULL);
        if (r < 0)
                goto finish;

        undo_mount = true;

        /* If nspawn dies abruptly the cgroup hierarchy created below
         * its unit isn't cleaned up. So, let's remove it
         * https://github.com/systemd/systemd/pull/4223#issuecomment-252519810 */
        fn = strjoina(tree, cgroup);
        (void) rm_rf(fn, REMOVE_ROOT|REMOVE_ONLY_DIRECTORIES);

        fn = strjoina(tree, cgroup, "/cgroup.procs");
        (void) mkdir_parents(fn, 0755);

        sprintf(pid_string, PID_FMT, pid);
        r = write_string_file(fn, pid_string, 0);
        if (r < 0) {
                log_error_errno(r, "Failed to move process: %m");
                goto finish;
        }

        fn = strjoina(tree, cgroup);
        r = chown_cgroup_path(fn, uid_shift);
        if (r < 0)
                log_error_errno(r, "Failed to chown() cgroup %s: %m", fn);
finish:
        if (undo_mount)
                (void) umount_verbose(tree);

        (void) rmdir(tree);
        return r;
}

static int create_subcgroup(pid_t pid, CGroupUnified outer_cgver, CGroupUnified inner_cgver) {
        _cleanup_free_ char *cgroup = NULL;
        const char *child;
        int r;
        CGroupMask supported;

        /* In the unified hierarchy inner nodes may only contain
         * subgroups, but not processes. Hence, if we running in the
         * unified hierarchy and the container does the same, and we
         * did not create a scope unit for the container move us and
         * the container into two separate subcgroups. */

        if (inner_cgver == CGROUP_UNIFIED_NONE)
                return 0;

        if (outer_cgver < CGROUP_UNIFIED_SYSTEMD232)
                return 0;

        r = cg_mask_supported(&supported);
        if (r < 0)
                return log_error_errno(r, "Failed to determine supported controllers: %m");

        r = cg_pid_get_path(SYSTEMD_CGROUP_CONTROLLER, 0, &cgroup);
        if (r < 0)
                return log_error_errno(r, "Failed to get our control group: %m");

        child = strjoina(cgroup, "/payload");
        r = cg_create_and_attach(SYSTEMD_CGROUP_CONTROLLER, child, pid);
        if (r < 0)
                return log_error_errno(r, "Failed to create %s subcgroup: %m", child);

        child = strjoina(cgroup, "/supervisor");
        r = cg_create_and_attach(SYSTEMD_CGROUP_CONTROLLER, child, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to create %s subcgroup: %m", child);

        /* Try to enable as many controllers as possible for the new payload. */
        (void) cg_enable_everywhere(supported, supported, cgroup);
        return 0;
}

int cgroup_setup(pid_t pid, CGroupUnified outer_cgver, CGroupUnified inner_cgver, uid_t uid_shift, bool keep_unit) {

        int r;

        r = sync_cgroup(pid, outer_cgver, inner_cgver, uid_shift);
        if (r < 0)
                return r;

        if (keep_unit) {
                r = create_subcgroup(pid, outer_cgver, inner_cgver);
                if (r < 0)
                        return r;
        }

        r = chown_cgroup(pid, uid_shift);
        if (r < 0)
                return r;

        return 0;
}

/********************************************************************/

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

static int mount_legacy_cgroup_hierarchy(
                const char *dest,
                const char *controller,
                const char *hierarchy,
                bool read_only) {

        const char *to, *fstype, *opts;
        int r;

        to = strjoina(strempty(dest), "/sys/fs/cgroup/", hierarchy);

        r = path_is_mount_point(to, dest, 0);
        if (r < 0 && r != -ENOENT)
                return log_error_errno(r, "Failed to determine if %s is mounted already: %m", to);
        if (r > 0)
                return 0;

        mkdir_p(to, 0755);

        /* The superblock mount options of the mount point need to be
         * identical to the hosts', and hence writable... */
        if (streq(controller, SYSTEMD_CGROUP_CONTROLLER_HYBRID)) {
                fstype = "cgroup2";
                opts = NULL;
        } else if (streq(controller, SYSTEMD_CGROUP_CONTROLLER_LEGACY)) {
                fstype = "cgroup";
                opts = "none,name=systemd,xattr";
        } else {
                fstype = "cgroup";
                opts = controller;
        }

        r = mount_verbose(LOG_ERR, "cgroup", to, fstype, MS_NOSUID|MS_NOEXEC|MS_NODEV, opts);
        if (r < 0)
                return r;

        /* ... hence let's only make the bind mount read-only, not the superblock. */
        if (read_only) {
                r = mount_verbose(LOG_ERR, NULL, to, NULL,
                                  MS_BIND|MS_REMOUNT|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_RDONLY, NULL);
                if (r < 0)
                        return r;
        }

        return 1;
}

/* Mount a legacy cgroup hierarchy when cgroup namespaces are supported. */
static int mount_legacy_cgns_supported(
                const char *dest,
                CGroupUnified outer_cgver,
                CGroupUnified inner_cgver,
                bool userns,
                uid_t uid_shift,
                uid_t uid_range,
                const char *selinux_apifs_context) {

        _cleanup_set_free_free_ Set *hierarchies = NULL;
        const char *cgroup_root = "/sys/fs/cgroup", *c;
        int r;

        (void) mkdir_p(cgroup_root, 0755);

        /* Mount a tmpfs to /sys/fs/cgroup if it's not mounted there yet. */
        r = path_is_mount_point(cgroup_root, dest, AT_SYMLINK_FOLLOW);
        if (r < 0)
                return log_error_errno(r, "Failed to determine if /sys/fs/cgroup is already mounted: %m");
        if (r == 0) {
                _cleanup_free_ char *options = NULL;

                /* When cgroup namespaces are enabled and user namespaces are
                 * used then the mount of the cgroupfs is done *inside* the new
                 * user namespace. We're root in the new user namespace and the
                 * kernel will happily translate our uid/gid to the correct
                 * uid/gid as seen from e.g. /proc/1/mountinfo. So we simply
                 * pass uid 0 and not uid_shift to tmpfs_patch_options().
                 */
                r = tmpfs_patch_options("mode=755", 0, selinux_apifs_context, &options);
                if (r < 0)
                        return log_oom();

                r = mount_verbose(LOG_ERR, "tmpfs", cgroup_root, "tmpfs",
                                  MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_STRICTATIME, options);
                if (r < 0)
                        return r;
        }

        if (outer_cgver >= CGROUP_UNIFIED_ALL)
                goto skip_controllers;

        hierarchies = set_new(&string_hash_ops);
        if (!hierarchies)
                return log_oom();

        r = get_v1_hierarchies(hierarchies);
        if (r < 0)
                return log_error_errno(r, "Failed to determine cgroup hierarchies: %m");

        for (;;) {
                _cleanup_free_ const char *hierarchy = NULL;

                hierarchy = set_steal_first(hierarchies);
                if (!hierarchy)
                        break;

                if (streq(hierarchy, "name=systemd"))
                        continue;

                r = mount_legacy_cgroup_hierarchy("", hierarchy, hierarchy, !userns);
                if (r < 0)
                        return r;

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

                        target = prefix_root("/sys/fs/cgroup", controller);
                        if (!target)
                                return log_oom();

                        if (streq(hierarchy, controller))
                                break;

                        r = symlink_idempotent(hierarchy, target);
                        if (r == -EINVAL)
                                return log_error_errno(r, "Invalid existing symlink for combined hierarchy: %m");
                        if (r < 0)
                                return log_error_errno(r, "Failed to create symlink for combined hierarchy: %m");
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
                r = mount_legacy_cgroup_hierarchy("", SYSTEMD_CGROUP_CONTROLLER_HYBRID, "systemd", false);
                if (r < 0)
                        return r;
                break;
        case CGROUP_UNIFIED_SYSTEMD233:
                r = mount_legacy_cgroup_hierarchy("", SYSTEMD_CGROUP_CONTROLLER_HYBRID, "unified", false);
                if (r < 0)
                        return r;
                /* fall through */
        case CGROUP_UNIFIED_NONE:
                r = mount_legacy_cgroup_hierarchy("", SYSTEMD_CGROUP_CONTROLLER_LEGACY, "systemd", false);
                if (r < 0)
                        return r;
                break;
        }

        if (!userns)
                return mount_verbose(LOG_ERR, NULL, cgroup_root, NULL,
                                     MS_REMOUNT|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_STRICTATIME|MS_RDONLY, "mode=755");

        return 0;
}

/* Mount legacy cgroup hierarchy when cgroup namespaces are unsupported. */
static int mount_legacy_cgns_unsupported(
                const char *dest,
                CGroupUnified outer_cgver,
                CGroupUnified inner_cgver,
                bool userns,
                uid_t uid_shift,
                uid_t uid_range,
                const char *selinux_apifs_context) {

        _cleanup_set_free_free_ Set *controllers = NULL;
        const char *cgroup_root;
        int r;

        cgroup_root = prefix_roota(dest, "/sys/fs/cgroup");

        (void) mkdir_p(cgroup_root, 0755);

        /* Mount a tmpfs to /sys/fs/cgroup if it's not mounted there yet. */
        r = path_is_mount_point(cgroup_root, dest, AT_SYMLINK_FOLLOW);
        if (r < 0)
                return log_error_errno(r, "Failed to determine if /sys/fs/cgroup is already mounted: %m");
        if (r == 0) {
                _cleanup_free_ char *options = NULL;

                r = tmpfs_patch_options("mode=755", uid_shift == 0 ? UID_INVALID : uid_shift, selinux_apifs_context, &options);
                if (r < 0)
                        return log_oom();

                r = mount_verbose(LOG_ERR, "tmpfs", cgroup_root, "tmpfs",
                                  MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_STRICTATIME, options);
                if (r < 0)
                        return r;
        }

        if (outer_cgver >= CGROUP_UNIFIED_ALL)
                goto skip_controllers;

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

                origin = prefix_root("/sys/fs/cgroup/", controller);
                if (!origin)
                        return log_oom();

                r = readlink_malloc(origin, &combined);
                if (r == -EINVAL) {
                        /* Not a symbolic link, but directly a single cgroup hierarchy */

                        r = mount_legacy_cgroup_hierarchy(dest, controller, controller, true);
                        if (r < 0)
                                return r;

                } else if (r < 0)
                        return log_error_errno(r, "Failed to read link %s: %m", origin);
                else {
                        _cleanup_free_ char *target = NULL;

                        target = prefix_root(dest, origin);
                        if (!target)
                                return log_oom();

                        /* A symbolic link, a combination of controllers in one hierarchy */

                        if (!filename_is_valid(combined)) {
                                log_warning("Ignoring invalid combined hierarchy %s.", combined);
                                continue;
                        }

                        r = mount_legacy_cgroup_hierarchy(dest, combined, combined, true);
                        if (r < 0)
                                return r;

                        r = symlink_idempotent(combined, target);
                        if (r == -EINVAL)
                                return log_error_errno(r, "Invalid existing symlink for combined hierarchy: %m");
                        if (r < 0)
                                return log_error_errno(r, "Failed to create symlink for combined hierarchy: %m");
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
                r = mount_legacy_cgroup_hierarchy(dest, SYSTEMD_CGROUP_CONTROLLER_HYBRID, "systemd", false);
                if (r < 0)
                        return r;
                break;
        case CGROUP_UNIFIED_SYSTEMD233:
                r = mount_legacy_cgroup_hierarchy(dest, SYSTEMD_CGROUP_CONTROLLER_HYBRID, "unified", false);
                if (r < 0)
                        return r;
                /* fall through */
        case CGROUP_UNIFIED_NONE:
                r = mount_legacy_cgroup_hierarchy(dest, SYSTEMD_CGROUP_CONTROLLER_LEGACY, "systemd", false);
                if (r < 0)
                        return r;
                break;
        }

        return mount_verbose(LOG_ERR, NULL, cgroup_root, NULL,
                             MS_REMOUNT|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_STRICTATIME|MS_RDONLY, "mode=755");
}

static int mount_unified_cgroups(const char *dest) {
        const char *p;
        int r;

        assert(dest);

        p = prefix_roota(dest, "/sys/fs/cgroup");

        (void) mkdir_p(p, 0755);

        r = path_is_mount_point(p, dest, AT_SYMLINK_FOLLOW);
        if (r < 0)
                return log_error_errno(r, "Failed to determine if %s is mounted already: %m", p);
        if (r > 0) {
                p = prefix_roota(dest, "/sys/fs/cgroup/cgroup.procs");
                if (access(p, F_OK) >= 0)
                        return 0;
                if (errno != ENOENT)
                        return log_error_errno(errno, "Failed to determine if mount point %s contains the unified cgroup hierarchy: %m", p);

                log_error("%s is already mounted but not a unified cgroup hierarchy. Refusing.", p);
                return -EINVAL;
        }

        return mount_verbose(LOG_ERR, "cgroup", p, "cgroup2", MS_NOSUID|MS_NOEXEC|MS_NODEV, NULL);
}

int mount_cgroups(
                const char *dest,
                CGroupUnified outer_cgver,
                CGroupUnified inner_cgver,
                bool userns,
                uid_t uid_shift,
                uid_t uid_range,
                const char *selinux_apifs_context,
                bool use_cgns) {

        switch (inner_cgver) {
        default:
        case CGROUP_UNIFIED_UNKNOWN:
                assert_not_reached("unknown inner_cgver");
        case CGROUP_UNIFIED_NONE:
        case CGROUP_UNIFIED_SYSTEMD232:
        case CGROUP_UNIFIED_SYSTEMD233:
                if (use_cgns)
                        return mount_legacy_cgns_supported(dest, outer_cgver, inner_cgver, userns, uid_shift, uid_range, selinux_apifs_context);
                else
                        return mount_legacy_cgns_unsupported(dest, outer_cgver, inner_cgver, userns, uid_shift, uid_range, selinux_apifs_context);
        case CGROUP_UNIFIED_ALL:
                return mount_unified_cgroups(dest);
        }
}

static int mount_systemd_cgroup_writable_one(const char *systemd_own, const char *systemd_root)
{
        int r;

        /* Make our own cgroup a (writable) bind mount */
        r = mount_verbose(LOG_ERR, systemd_own, systemd_own,  NULL, MS_BIND, NULL);
        if (r < 0)
                return r;

        /* And then remount the systemd cgroup root read-only */
        return mount_verbose(LOG_ERR, NULL, systemd_root, NULL,
                             MS_BIND|MS_REMOUNT|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_RDONLY, NULL);
}

int mount_systemd_cgroup_writable(
                const char *dest,
                CGroupUnified inner_cgver) {

        _cleanup_free_ char *own_cgroup_path = NULL;
        int r;

        assert(dest);

        r = cg_pid_get_path(NULL, 0, &own_cgroup_path);
        if (r < 0)
                return log_error_errno(r, "Failed to determine our own cgroup path: %m");

        /* If we are living in the top-level, then there's nothing to do... */
        if (path_equal(own_cgroup_path, "/"))
                return 0;

        switch (inner_cgver) {
        default:
        case CGROUP_UNIFIED_UNKNOWN:
                assert_not_reached("unknown inner_cgver");
        case CGROUP_UNIFIED_ALL:
                return mount_systemd_cgroup_writable_one(strjoina(dest, "/sys/fs/cgroup", own_cgroup_path),
                                                         prefix_roota(dest, "/sys/fs/cgroup"));
        case CGROUP_UNIFIED_SYSTEMD233:
        case CGROUP_UNIFIED_SYSTEMD232:
                r = mount_systemd_cgroup_writable_one(strjoina(dest, "/sys/fs/cgroup/unified", own_cgroup_path),
                                                      prefix_roota(dest, "/sys/fs/cgroup/unified"));
                if (r < 0)
                        return r;
                /* fall through */
        case CGROUP_UNIFIED_NONE:
                return mount_systemd_cgroup_writable_one(strjoina(dest, "/sys/fs/cgroup/systemd", own_cgroup_path),
                                                         prefix_roota(dest, "/sys/fs/cgroup/systemd"));
        }
}
