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

#include "fs-util.h"
#include "mount-util.h"
#include "path-util.h"
#include "strv.h"
#include "user-util.h"

#include "nspawn-mount.h"

static int tmpfs_patch_options(
                const char *options,
                bool userns,
                uid_t uid_shift, uid_t uid_range,
                bool patch_ids,
                const char *selinux_apifs_context,
                char **ret) {

        char *buf = NULL;

        if ((userns && uid_shift != 0) || patch_ids) {
                assert(uid_shift != UID_INVALID);

                if (asprintf(&buf, "%s%suid=" UID_FMT ",gid=" UID_FMT,
                             options ?: "", options ? "," : "",
                             uid_shift, uid_shift) < 0)
                        return -ENOMEM;

                options = buf;
        }

#ifdef HAVE_SELINUX
        if (selinux_apifs_context) {
                char *t;

                t = strjoin(options ?: "", options ? "," : "",
                            "context=\"", selinux_apifs_context, "\"");
                free(buf);
                if (!t)
                        return -ENOMEM;

                buf = t;
        }
#endif

        if (!buf && options) {
                buf = strdup(options);
                if (!buf)
                        return -ENOMEM;
        }
        *ret = buf;

        return !!buf;
}

static int mkdir_userns(const char *path, mode_t mode, MountSettingsMask mask, uid_t uid_shift) {
        int r;

        assert(path);

        r = mkdir(path, mode);
        if (r < 0 && errno != EEXIST)
                return -errno;

        if ((mask & MOUNT_USE_USERNS) == 0)
                return 0;

        if (mask & MOUNT_IN_USERNS)
                return 0;

        r = lchown(path, uid_shift, uid_shift);
        if (r < 0)
                return -errno;

        return 0;
}

static int mkdir_userns_p(const char *prefix, const char *path, mode_t mode, MountSettingsMask mask, uid_t uid_shift) {
        const char *p, *e;
        int r;

        assert(path);

        if (prefix && !path_startswith(path, prefix))
                return -ENOTDIR;

        /* create every parent directory in the path, except the last component */
        p = path + strspn(path, "/");
        for (;;) {
                char t[strlen(path) + 1];

                e = p + strcspn(p, "/");
                p = e + strspn(e, "/");

                /* Is this the last component? If so, then we're done */
                if (*p == 0)
                        break;

                memcpy(t, path, e - path);
                t[e-path] = 0;

                if (prefix && path_startswith(prefix, t))
                        continue;

                r = mkdir_userns(t, mode, mask, uid_shift);
                if (r < 0)
                        return r;
        }

        return mkdir_userns(path, mode, mask, uid_shift);
}

int mount_all(const char *dest,
              MountSettingsMask mount_settings,
              uid_t uid_shift, uid_t uid_range,
              const char *selinux_apifs_context) {

        typedef struct MountPoint {
                const char *what;
                const char *where;
                const char *type;
                const char *options;
                unsigned long flags;
                MountSettingsMask mount_settings;
        } MountPoint;

        static const MountPoint mount_table[] = {
                /* inner child mounts */
                { "proc",                "/proc",               "proc",  NULL,        MS_NOSUID|MS_NOEXEC|MS_NODEV,                              MOUNT_FATAL|MOUNT_IN_USERNS },
                { "/proc/sys",           "/proc/sys",           NULL,    NULL,        MS_BIND,                                                   MOUNT_FATAL|MOUNT_IN_USERNS|MOUNT_APPLY_APIVFS_RO },                          /* Bind mount first ... */
                { "/proc/sys/net",       "/proc/sys/net",       NULL,    NULL,        MS_BIND,                                                   MOUNT_FATAL|MOUNT_IN_USERNS|MOUNT_APPLY_APIVFS_RO|MOUNT_APPLY_APIVFS_NETNS }, /* (except for this) */
                { NULL,                  "/proc/sys",           NULL,    NULL,        MS_BIND|MS_RDONLY|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_REMOUNT, MOUNT_FATAL|MOUNT_IN_USERNS|MOUNT_APPLY_APIVFS_RO },                          /* ... then, make it r/o */
                { "/proc/sysrq-trigger", "/proc/sysrq-trigger", NULL,    NULL,        MS_BIND,                                                               MOUNT_IN_USERNS|MOUNT_APPLY_APIVFS_RO },                          /* Bind mount first ... */
                { NULL,                  "/proc/sysrq-trigger", NULL,    NULL,        MS_BIND|MS_RDONLY|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_REMOUNT,             MOUNT_IN_USERNS|MOUNT_APPLY_APIVFS_RO },                          /* ... then, make it r/o */

                /* outer child mounts */
                { "tmpfs",               "/tmp",                "tmpfs", "mode=1777", MS_NOSUID|MS_NODEV|MS_STRICTATIME,                                            MOUNT_FATAL },
                { "tmpfs",               "/sys",                "tmpfs", "mode=755",  MS_NOSUID|MS_NOEXEC|MS_NODEV,                              MOUNT_FATAL|MOUNT_APPLY_APIVFS_NETNS },

                { "tmpfs",               "/dev",                "tmpfs", "mode=755",  MS_NOSUID|MS_STRICTATIME,                                  MOUNT_FATAL },
                { "tmpfs",               "/dev/shm",            "tmpfs", "mode=1777", MS_NOSUID|MS_NODEV|MS_STRICTATIME,                         MOUNT_FATAL },
                { "tmpfs",               "/run",                "tmpfs", "mode=755",  MS_NOSUID|MS_NODEV|MS_STRICTATIME,                         MOUNT_FATAL },
#ifdef HAVE_SELINUX
                { "/sys/fs/selinux",     "/sys/fs/selinux",     NULL,     NULL,       MS_BIND,                                                   0 },  /* Bind mount first */
                { NULL,                  "/sys/fs/selinux",     NULL,     NULL,       MS_BIND|MS_RDONLY|MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_REMOUNT, 0 },  /* Then, make it r/o */
#endif
        };

        unsigned k;
        int r;
        bool use_userns = (mount_settings & MOUNT_USE_USERNS);
        bool netns = (mount_settings & MOUNT_APPLY_APIVFS_NETNS);
        bool ro = (mount_settings & MOUNT_APPLY_APIVFS_RO);
        bool in_userns = (mount_settings & MOUNT_IN_USERNS);

        for (k = 0; k < ELEMENTSOF(mount_table); k++) {
                _cleanup_free_ char *where = NULL, *options = NULL;
                const char *o;
                bool fatal = (mount_table[k].mount_settings & MOUNT_FATAL);

                if (in_userns != (bool)(mount_table[k].mount_settings & MOUNT_IN_USERNS))
                        continue;

                if (!netns && (bool)(mount_table[k].mount_settings & MOUNT_APPLY_APIVFS_NETNS))
                        continue;

                if (!ro && (bool)(mount_table[k].mount_settings & MOUNT_APPLY_APIVFS_RO))
                        continue;

                r = chase_symlinks(mount_table[k].where, dest, CHASE_NONEXISTENT|CHASE_PREFIX_ROOT, &where);
                if (r < 0)
                        return log_error_errno(r, "Failed to resolve %s/%s: %m", dest, mount_table[k].where);

                r = path_is_mount_point(where, NULL, 0);
                if (r < 0 && r != -ENOENT)
                        return log_error_errno(r, "Failed to detect whether %s is a mount point: %m", where);

                /* Skip this entry if it is not a remount. */
                if (mount_table[k].what && r > 0)
                        continue;

                r = mkdir_userns_p(dest, where, 0755, mount_settings, uid_shift);
                if (r < 0 && r != -EEXIST) {
                        if (fatal)
                                return log_error_errno(r, "Failed to create directory %s: %m", where);

                        log_debug_errno(r, "Failed to create directory %s: %m", where);
                        continue;
                }

                o = mount_table[k].options;
                if (streq_ptr(mount_table[k].type, "tmpfs")) {
                        if (in_userns)
                                r = tmpfs_patch_options(o, use_userns, 0, uid_range, true, selinux_apifs_context, &options);
                        else
                                r = tmpfs_patch_options(o, use_userns, uid_shift, uid_range, false, selinux_apifs_context, &options);
                        if (r < 0)
                                return log_oom();
                        if (r > 0)
                                o = options;
                }

                r = mount_verbose(fatal ? LOG_ERR : LOG_DEBUG,
                                  mount_table[k].what,
                                  where,
                                  mount_table[k].type,
                                  mount_table[k].flags,
                                  o);
                if (r < 0 && fatal)
                        return r;
        }

        return 0;
}

int setup_pivot_root(const char *directory, const char *pivot_root_new, const char *pivot_root_old) {
        _cleanup_free_ char *directory_pivot_root_new = NULL;
        _cleanup_free_ char *pivot_tmp_pivot_root_old = NULL;
        char pivot_tmp[] = "/tmp/nspawn-pivot-XXXXXX";
        bool remove_pivot_tmp = false;
        int r;

        assert(directory);

        if (!pivot_root_new)
                return 0;

        /* Pivot pivot_root_new to / and the existing / to pivot_root_old.
         * If pivot_root_old is NULL, the existing / disappears.
         * This requires a temporary directory, pivot_tmp, which is
         * not a child of either.
         *
         * This is typically used for OSTree-style containers, where
         * the root partition contains several sysroots which could be
         * run. Normally, one would be chosen by the bootloader and
         * pivoted to / by initramfs.
         *
         * For example, for an OSTree deployment, pivot_root_new
         * would be: /ostree/deploy/$os/deploy/$checksum. Note that this
         * code doesn’t do the /var mount which OSTree expects: use
         * --bind +/sysroot/ostree/deploy/$os/var:/var for that.
         *
         * So in the OSTree case, we’ll end up with something like:
         *  - directory = /tmp/nspawn-root-123456
         *  - pivot_root_new = /ostree/deploy/os/deploy/123abc
         *  - pivot_root_old = /sysroot
         *  - directory_pivot_root_new =
         *       /tmp/nspawn-root-123456/ostree/deploy/os/deploy/123abc
         *  - pivot_tmp = /tmp/nspawn-pivot-123456
         *  - pivot_tmp_pivot_root_old = /tmp/nspawn-pivot-123456/sysroot
         *
         * Requires all file systems at directory and below to be mounted
         * MS_PRIVATE or MS_SLAVE so they can be moved.
         */
        directory_pivot_root_new = prefix_root(directory, pivot_root_new);

        /* Remount directory_pivot_root_new to make it movable. */
        r = mount_verbose(LOG_ERR, directory_pivot_root_new, directory_pivot_root_new, NULL, MS_BIND, NULL);
        if (r < 0)
                goto done;

        if (pivot_root_old) {
                if (!mkdtemp(pivot_tmp)) {
                        r = log_error_errno(errno, "Failed to create temporary directory: %m");
                        goto done;
                }

                remove_pivot_tmp = true;
                pivot_tmp_pivot_root_old = prefix_root(pivot_tmp, pivot_root_old);

                r = mount_verbose(LOG_ERR, directory_pivot_root_new, pivot_tmp, NULL, MS_MOVE, NULL);
                if (r < 0)
                        goto done;

                r = mount_verbose(LOG_ERR, directory, pivot_tmp_pivot_root_old, NULL, MS_MOVE, NULL);
                if (r < 0)
                        goto done;

                r = mount_verbose(LOG_ERR, pivot_tmp, directory, NULL, MS_MOVE, NULL);
                if (r < 0)
                        goto done;
        } else {
                r = mount_verbose(LOG_ERR, directory_pivot_root_new, directory, NULL, MS_MOVE, NULL);
                if (r < 0)
                        goto done;
        }

done:
        if (remove_pivot_tmp)
                (void) rmdir(pivot_tmp);

        return r;
}
