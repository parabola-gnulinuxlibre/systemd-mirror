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

#include "systemd-basic/alloc-util.h"
#include "systemd-basic/fd-util.h"
#include "systemd-basic/fileio.h"
#include "systemd-basic/mkdir.h"
#include "systemd-basic/mount-util.h"
#include "systemd-basic/rm-rf.h"
#include "systemd-basic/string-util.h"
#include "systemd-basic/strv.h"
#include "systemd-basic/user-util.h"
#include "systemd-basic/util.h"

#include "nspawn-cgroup.h"

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
                                       "Failed to chown() cgroup file %s, ignoring: %m", fn);

        return 0;
}

int chown_cgroup(pid_t pid, uid_t uid_shift) {
        _cleanup_sdcgroupfree_ SdCGroup cgroup;
        _cleanup_free_ char *fs = NULL;

        int r;

        /* If uid_shift == UID_INVALID, then chown_cgroup_path() is a no-op, and there isn't really a point to actually
         * doing any of this work.  Of course, it would still be "safe" to continue, just pointless; and another
         * opportunity for cgroup-util to screw up on non-systemd systems. */
        if (uid_shift == UID_INVALID)
                return 0;

        r = cg2_sd_pid_get_cgroup(pid, &cgroup);
        if (r < 0)
                return log_error_errno(r, "Failed to get host cgroup of the container: %m");

        fs = cg2_sd_cgroup_get_filepath(cgroup);
        if (!fs)
                return log_error_errno(-ENOMEM, "Failed to get host file system path for container cgroup: %m");

        r = chown_cgroup_path(fs, uid_shift);
        if (r < 0)
                return log_error_errno(r, "Failed to chown() cgroup %s: %m", fs);

        return 0;
}

int sync_cgroup(pid_t pid, SdCGroupVersion inner_cgver, uid_t uid_shift) {
        _cleanup_sdcgroupfree_ SdCGroup outer_cgroup;
        _cleanup_free_ char *cgpath = NULL;
        char mountpoint[] = "/tmp/containerXXXXXX", pid_string[DECIMAL_STR_MAX(pid) + 1];
        bool undo_mount = false;
        const char *fn, *inner_hier;
        int r;
        SdCGroupVersion outer_cgver;

#define LOG_PFIX "PID " PID_FMT ": sync host cgroup -> container cgroup"

        r = cg2_sd_get_version(&outer_cgver);
        if (r < 0)
                return log_error_errno(r, LOG_PFIX ": failed to determine host cgroup version: %m", pid);

        if (cg2_sd_ver_get_hier_ver(outer_cgver) == cg2_sd_ver_get_hier_ver(inner_cgver))
                return 0;

        /* When the host uses the legacy cgroup setup, but the
         * container shall use the unified hierarchy, let's make sure
         * we copy the path from the name=systemd hierarchy into the
         * unified hierarchy. Similar for the reverse situation. */

        r = cg2_sd_pid_get_cgroup(pid, &outer_cgroup);
        if (r < 0)
                return log_error_errno(r, LOG_PFIX ": failed to determine host cgroup: %m", pid);
        cgpath = cg2_sd_cgroup_get_cgpath(outer_cgroup);
        if (!cgpath)
                return log_error_errno(-ENOMEM, LOG_PFIX ": %m", pid);

        /* In order to access the container's hierarchy we need to mount it */
        if (!mkdtemp(mountpoint))
                return log_error_errno(errno, LOG_PFIX ": failed to create temporary mount point for container cgroup hierarchy: %m", pid);

        if (cg2_sd_ver_get_hier_ver(outer_cgver) == 2) {
                /* host: v2 ; container: v1 */
                inner_hier = "?:name=systemd";
                r = mount_verbose(LOG_ERR, "cgroup", mountpoint, "cgroup",
                                  MS_NOSUID|MS_NOEXEC|MS_NODEV, "none,name=systemd,xattr");
        } else {
                /* host: v1 ; container: v2 */
                inner_hier = "0:";
                r = mount_verbose(LOG_ERR, "cgroup", mountpoint, "cgroup2",
                                  MS_NOSUID|MS_NOEXEC|MS_NODEV, NULL);
        }
        if (r < 0) {
                log_error(LOG_PFIX ": failed to mount container cgroup hierarchy", pid);
                goto finish;
        }

        undo_mount = true;

        /* If nspawn dies abruptly the cgroup hierarchy created below
         * its unit isn't cleaned up. So, let's remove it
         * https://github.com/systemd/systemd/pull/4223#issuecomment-252519810 */
        fn = strjoina(mountpoint, cgroup);
        (void) rm_rf(fn, REMOVE_ROOT|REMOVE_ONLY_DIRECTORIES);

        fn = strjoina(mountpoint, cgroup, "/cgroup.procs");
        (void) mkdir_parents(fn, 0755);

        sprintf(pid_string, PID_FMT, pid);
        r = write_string_file(fn, pid_string, 0);
        if (r < 0) {
                log_error_errno(r, LOG_PFIX ": failed to move process to `%s:%s': %m", pid, inner_hier, cgroup);
                goto finish;
        }

        fn = strjoina(mountpoint, cgroup);
        r = chown_cgroup_path(fn, uid_shift);
        if (r < 0)
                log_error_errno(r, LOG_PFIX ": chown(path=\"%s\", uid=" UID_FMT "): %m", pid, fn, uid_shift);
finish:
        if (undo_mount)
                /* FIXME: if this errors, the log message is too low level */
                (void) umount_verbose(mountpoint);

        (void) rmdir(mountpoint);
        return r;
}

int create_subcgroup(pid_t pid, SdCGroupVersion inner_cgver) {
        _cleanup_free_ char *cgroup = NULL;
        const char *child;
        int r;
        CGroupMask supported;
        SdCGroupVersion outer_cgver;

        /* In the unified hierarchy inner nodes may only contain
         * subgroups, but not processes. Hence, if we running in the
         * unified hierarchy and the container does the same, and we
         * did not create a scope unit for the container move us and
         * the container into two separate subcgroups. */

        if (inner_cgver == CGROUP_VER_1)
                return 0;

        r = cg2_sd_get_version(&outer_cgver);
        if (r < 0)
                return log_error_errno(r, "Failed to create host subcgroup: Failed to determine cgroup version: %m");

        if (outer_cgver == CGROUP_VER_1)
                return 0;

        r = cg_mask_supported(&supported);
        if (r < 0)
                return log_error_errno(r, "Failed to create host subcgroup: Failed to determine supported controllers: %m");

        r = cg_pid_get_path(SYSTEMD_CGROUP_CONTROLLER, 0, &cgroup);
        if (r < 0)
                return log_error_errno(r, "Failed to create host subcgroup: Failed to get our control group: %m");

        child = strjoina(cgroup, "/payload");
        r = cg_create_and_attach(SYSTEMD_CGROUP_CONTROLLER, child, pid);
        if (r < 0)
                return log_error_errno(r, "Failed to create host subcgroup: Failed to create %s subcgroup: %m", child);

        child = strjoina(cgroup, "/supervisor");
        r = cg_create_and_attach(SYSTEMD_CGROUP_CONTROLLER, child, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to create host subcgroup: Failed to create %s subcgroup: %m", child);

        /* Try to enable as many controllers as possible for the new payload. */
        (void) cg_enable_everywhere(supported, supported, cgroup);
        return 0;
}
