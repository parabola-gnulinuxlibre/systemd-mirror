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

/* Retrieve existing subsystems. This function is called in a new cgroup
 * namespace.
 */
static int get_controllers(Set *subsystems) {
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

                if (STR_IN_SET(l, "", "name=systemd"))
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

static int mount_legacy_cgroup_hierarchy(const char *dest, const char *controller, const char *hierarchy,
                                         CGroupUnified unified_requested, bool read_only) {
        const char *to, *fstype, *opts;
        int r;

        to = strjoina(strempty(dest), "/sys/fs/cgroup/", hierarchy);

        r = path_is_mount_point(to, 0);
        if (r < 0 && r != -ENOENT)
                return log_error_errno(r, "Failed to determine if %s is mounted already: %m", to);
        if (r > 0)
                return 0;

        mkdir_p(to, 0755);

        /* The superblock mount options of the mount point need to be
         * identical to the hosts', and hence writable... */
        if (streq(controller, SYSTEMD_CGROUP_CONTROLLER)) {
                if (unified_requested >= CGROUP_UNIFIED_SYSTEMD) {
                        fstype = "cgroup2";
                        opts = NULL;
                } else {
                        fstype = "cgroup";
                        opts = "none,name=systemd,xattr";
                }
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
                CGroupUnified unified_requested, bool userns, uid_t uid_shift,
                uid_t uid_range, const char *selinux_apifs_context) {
        _cleanup_set_free_free_ Set *controllers = NULL;
        const char *cgroup_root = "/sys/fs/cgroup", *c;
        int r;

        (void) mkdir_p(cgroup_root, 0755);

        /* Mount a tmpfs to /sys/fs/cgroup if it's not mounted there yet. */
        r = path_is_mount_point(cgroup_root, AT_SYMLINK_FOLLOW);
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
                r = tmpfs_patch_options("mode=755", userns, 0, uid_range, true, selinux_apifs_context, &options);
                if (r < 0)
                        return log_oom();

                r = mount_verbose(LOG_ERR, "tmpfs", cgroup_root, "tmpfs",
                                  MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_STRICTATIME, options);
                if (r < 0)
                        return r;
        }

        if (cg_all_unified() > 0)
                goto skip_controllers;

        controllers = set_new(&string_hash_ops);
        if (!controllers)
                return log_oom();

        r = get_controllers(controllers);
        if (r < 0)
                return log_error_errno(r, "Failed to determine cgroup controllers: %m");

        for (;;) {
                _cleanup_free_ const char *controller = NULL;

                controller = set_steal_first(controllers);
                if (!controller)
                        break;

                r = mount_legacy_cgroup_hierarchy("", controller, controller, unified_requested, !userns);
                if (r < 0)
                        return r;

                /* When multiple hierarchies are co-mounted, make their
                 * constituting individual hierarchies a symlink to the
                 * co-mount.
                 */
                c = controller;
                for (;;) {
                        _cleanup_free_ char *target = NULL, *tok = NULL;

                        r = extract_first_word(&c, &tok, ",", 0);
                        if (r < 0)
                                return log_error_errno(r, "Failed to extract co-mounted cgroup controller: %m");
                        if (r == 0)
                                break;

                        target = prefix_root("/sys/fs/cgroup", tok);
                        if (!target)
                                return log_oom();

                        if (streq(controller, tok))
                                break;

                        r = symlink_idempotent(controller, target);
                        if (r == -EINVAL)
                                return log_error_errno(r, "Invalid existing symlink for combined hierarchy: %m");
                        if (r < 0)
                                return log_error_errno(r, "Failed to create symlink for combined hierarchy: %m");
                }
        }

skip_controllers:
        r = mount_legacy_cgroup_hierarchy("", SYSTEMD_CGROUP_CONTROLLER, "systemd", unified_requested, false);
        if (r < 0)
                return r;

        if (!userns)
                return mount_verbose(LOG_ERR, NULL, cgroup_root, NULL,
                                     MS_REMOUNT|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_STRICTATIME|MS_RDONLY, "mode=755");

        return 0;
}

/* Mount legacy cgroup hierarchy when cgroup namespaces are unsupported. */
static int mount_legacy_cgns_unsupported(
                const char *dest,
                CGroupUnified unified_requested, bool userns, uid_t uid_shift, uid_t uid_range,
                const char *selinux_apifs_context) {
        _cleanup_set_free_free_ Set *controllers = NULL;
        const char *cgroup_root;
        int r;

        cgroup_root = prefix_roota(dest, "/sys/fs/cgroup");

        (void) mkdir_p(cgroup_root, 0755);

        /* Mount a tmpfs to /sys/fs/cgroup if it's not mounted there yet. */
        r = path_is_mount_point(cgroup_root, AT_SYMLINK_FOLLOW);
        if (r < 0)
                return log_error_errno(r, "Failed to determine if /sys/fs/cgroup is already mounted: %m");
        if (r == 0) {
                _cleanup_free_ char *options = NULL;

                r = tmpfs_patch_options("mode=755", userns, uid_shift, uid_range, false, selinux_apifs_context, &options);
                if (r < 0)
                        return log_oom();

                r = mount_verbose(LOG_ERR, "tmpfs", cgroup_root, "tmpfs",
                                  MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_STRICTATIME, options);
                if (r < 0)
                        return r;
        }

        if (cg_all_unified() > 0)
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

                        r = mount_legacy_cgroup_hierarchy(dest, controller, controller, unified_requested, true);
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

                        r = mount_legacy_cgroup_hierarchy(dest, combined, combined, unified_requested, true);
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
        r = mount_legacy_cgroup_hierarchy(dest, SYSTEMD_CGROUP_CONTROLLER, "systemd", unified_requested, false);
        if (r < 0)
                return r;

        return mount_verbose(LOG_ERR, NULL, cgroup_root, NULL,
                             MS_REMOUNT|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_STRICTATIME|MS_RDONLY, "mode=755");
}

static int mount_unified_cgroups(const char *dest) {
        const char *p;
        int r;

        assert(dest);

        p = prefix_roota(dest, "/sys/fs/cgroup");

        (void) mkdir_p(p, 0755);

        r = path_is_mount_point(p, AT_SYMLINK_FOLLOW);
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
                CGroupUnified unified_requested,
                bool userns, uid_t uid_shift, uid_t uid_range,
                const char *selinux_apifs_context,
                bool use_cgns) {

        if (unified_requested >= CGROUP_UNIFIED_ALL)
                return mount_unified_cgroups(dest);
        else if (use_cgns)
                return mount_legacy_cgns_supported(unified_requested, userns, uid_shift, uid_range, selinux_apifs_context);

        return mount_legacy_cgns_unsupported(dest, unified_requested, userns, uid_shift, uid_range, selinux_apifs_context);
}

int mount_systemd_cgroup_writable(
                const char *dest,
                CGroupUnified unified_requested) {

        _cleanup_free_ char *own_cgroup_path = NULL;
        const char *systemd_root, *systemd_own;
        int r;

        assert(dest);

        r = cg_pid_get_path(NULL, 0, &own_cgroup_path);
        if (r < 0)
                return log_error_errno(r, "Failed to determine our own cgroup path: %m");

        /* If we are living in the top-level, then there's nothing to do... */
        if (path_equal(own_cgroup_path, "/"))
                return 0;

        if (unified_requested >= CGROUP_UNIFIED_ALL) {
                systemd_own = strjoina(dest, "/sys/fs/cgroup", own_cgroup_path);
                systemd_root = prefix_roota(dest, "/sys/fs/cgroup");
        } else {
                systemd_own = strjoina(dest, "/sys/fs/cgroup/systemd", own_cgroup_path);
                systemd_root = prefix_roota(dest, "/sys/fs/cgroup/systemd");
        }

        /* Make our own cgroup a (writable) bind mount */
        r = mount_verbose(LOG_ERR, systemd_own, systemd_own,  NULL, MS_BIND, NULL);
        if (r < 0)
                return r;

        /* And then remount the systemd cgroup root read-only */
        return mount_verbose(LOG_ERR, NULL, systemd_root, NULL,
                             MS_BIND|MS_REMOUNT|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_RDONLY, NULL);
}
