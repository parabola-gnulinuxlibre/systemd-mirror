/***
  This file is part of systemd.

  Copyright 2010 Lennart Poettering

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

#ifdef HAVE_BLKID
#include <blkid/blkid.h>
#endif
#include <errno.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <sched.h>

#include <linux/loop.h>
#ifdef HAVE_SELINUX
#include <selinux/selinux.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mount.h>
#include <sys/personality.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>

#include <systemd/sd-daemon.h>
#include <systemd/sd-id128.h>

#include "core/loopback-setup.h"
#include "sd-id128/id128-util.h"
#include "systemd-basic/alloc-util.h"
#include "systemd-basic/barrier.h"
#include "systemd-basic/btrfs-util.h"
#include "systemd-basic/cap-list.h"
#include "systemd-basic/capability-util.h"
#include "systemd-basic/cgroup-util.h"
#include "systemd-basic/copy.h"
#include "systemd-basic/env-util.h"
#include "systemd-basic/fd-util.h"
#include "systemd-basic/fileio.h"
#include "systemd-basic/formats-util.h"
#include "systemd-basic/fs-util.h"
#include "systemd-basic/hostname-util.h"
#include "systemd-basic/log.h"
#include "systemd-basic/macro.h"
#include "systemd-basic/missing.h"
#include "systemd-basic/mkdir.h"
#include "systemd-basic/mount-util.h"
#include "systemd-basic/path-util.h"
#include "systemd-basic/process-util.h"
#include "systemd-basic/raw-clone.h"
#include "systemd-basic/rm-rf.h"
#include "systemd-basic/selinux-util.h"
#include "systemd-basic/signal-util.h"
#include "systemd-basic/socket-util.h"
#include "systemd-basic/stat-util.h"
#include "systemd-basic/stdio-util.h"
#include "systemd-basic/string-util.h"
#include "systemd-basic/strv.h"
#include "systemd-basic/terminal-util.h"
#include "systemd-basic/umask-util.h"
#include "systemd-basic/user-util.h"
#include "systemd-basic/util.h"
#include "systemd-blkid/blkid-util.h"
#include "systemd-shared/base-filesystem.h"
#include "systemd-shared/dev-setup.h"
#include "systemd-shared/fdset.h"
#include "systemd-shared/gpt.h"
#include "systemd-shared/ptyfwd.h"
#include "systemd-shared/udev-util.h"

#include "nspawn-args.h"
#include "nspawn-cgroup.h"
#include "nspawn-expose-ports.h"
#include "nspawn-mount.h"
#include "nspawn-network.h"
#include "nspawn-patch-uid.h"
#include "nspawn-register.h"
#include "nspawn-seccomp.h"
#include "nspawn-setuid.h"
#include "nspawn-stub-pid1.h"

/* nspawn is listening on the socket at the path in the constant nspawn_notify_socket_path
 * nspawn_notify_socket_path is relative to the container
 * the init process in the container pid can send messages to nspawn following the sd_notify(3) protocol */
#define NSPAWN_NOTIFY_SOCKET_PATH "/run/systemd/nspawn/notify"

typedef enum ContainerStatus {
        CONTAINER_TERMINATED,
        CONTAINER_REBOOTED
} ContainerStatus;

static const Args * args;

static int userns_lchown(const char *p, uid_t uid, gid_t gid) {
        assert(p);

        if (args->arg_userns_mode == USER_NAMESPACE_NO)
                return 0;

        if (uid == UID_INVALID && gid == GID_INVALID)
                return 0;

        if (uid != UID_INVALID) {
                uid += args->arg_uid_shift;

                if (uid < args->arg_uid_shift || uid >= args->arg_uid_shift + args->arg_uid_range)
                        return -EOVERFLOW;
        }

        if (gid != GID_INVALID) {
                gid += (gid_t) args->arg_uid_shift;

                if (gid < (gid_t) args->arg_uid_shift || gid >= (gid_t) (args->arg_uid_shift + args->arg_uid_range))
                        return -EOVERFLOW;
        }

        if (lchown(p, uid, gid) < 0)
                return -errno;

        return 0;
}

static int userns_mkdir(const char *root, const char *path, mode_t mode, uid_t uid, gid_t gid) {
        const char *q;

        q = prefix_roota(root, path);
        if (mkdir(q, mode) < 0) {
                if (errno == EEXIST)
                        return 0;
                return -errno;
        }

        return userns_lchown(q, uid, gid);
}

static int setup_timezone(const char *dest) {
        _cleanup_free_ char *p = NULL, *q = NULL;
        const char *where, *check, *what;
        char *z, *y;
        int r;

        assert(dest);

        /* Fix the timezone, if possible */
        r = readlink_malloc("/etc/localtime", &p);
        if (r < 0) {
                log_warning("host's /etc/localtime is not a symlink, not updating container timezone.");
                /* to handle warning, delete /etc/localtime and replace it
                 * with a symbolic link to a time zone data file.
                 *
                 * Example:
                 * ln -s /usr/share/zoneinfo/UTC /etc/localtime
                 */
                return 0;
        }

        z = path_startswith(p, "../usr/share/zoneinfo/");
        if (!z)
                z = path_startswith(p, "/usr/share/zoneinfo/");
        if (!z) {
                log_warning("/etc/localtime does not point into /usr/share/zoneinfo/, not updating container timezone.");
                return 0;
        }

        where = prefix_roota(dest, "/etc/localtime");
        r = readlink_malloc(where, &q);
        if (r >= 0) {
                y = path_startswith(q, "../usr/share/zoneinfo/");
                if (!y)
                        y = path_startswith(q, "/usr/share/zoneinfo/");

                /* Already pointing to the right place? Then do nothing .. */
                if (y && streq(y, z))
                        return 0;
        }

        check = strjoina("/usr/share/zoneinfo/", z);
        check = prefix_roota(dest, check);
        if (laccess(check, F_OK) < 0) {
                log_warning("Timezone %s does not exist in container, not updating container timezone.", z);
                return 0;
        }

        r = unlink(where);
        if (r < 0 && errno != ENOENT) {
                log_error_errno(errno, "Failed to remove existing timezone info %s in container: %m", where);
                return 0;
        }

        what = strjoina("../usr/share/zoneinfo/", z);
        if (symlink(what, where) < 0) {
                log_error_errno(errno, "Failed to correct timezone of container: %m");
                return 0;
        }

        r = userns_lchown(where, 0, 0);
        if (r < 0)
                return log_warning_errno(r, "Failed to chown /etc/localtime: %m");

        return 0;
}

static int setup_resolv_conf(const char *dest) {
        const char *where = NULL;
        int r;

        assert(dest);

        if (args->arg_private_network)
                return 0;

        /* Fix resolv.conf, if possible */
        where = prefix_roota(dest, "/etc/resolv.conf");

        r = copy_file("/etc/resolv.conf", where, O_TRUNC|O_NOFOLLOW, 0644, 0);
        if (r < 0) {
                /* If the file already exists as symlink, let's
                 * suppress the warning, under the assumption that
                 * resolved or something similar runs inside and the
                 * symlink points there.
                 *
                 * If the disk image is read-only, there's also no
                 * point in complaining.
                 */
                log_full_errno(IN_SET(r, -ELOOP, -EROFS) ? LOG_DEBUG : LOG_WARNING, r,
                               "Failed to copy /etc/resolv.conf to %s: %m", where);
                return 0;
        }

        r = userns_lchown(where, 0, 0);
        if (r < 0)
                log_warning_errno(r, "Failed to chown /etc/resolv.conf: %m");

        return 0;
}

static int setup_boot_id(const char *dest) {
        sd_id128_t rnd = SD_ID128_NULL;
        const char *from, *to;
        int r;

        /* Generate a new randomized boot ID, so that each boot-up of
         * the container gets a new one */

        from = prefix_roota(dest, "/run/proc-sys-kernel-random-boot-id");
        to = prefix_roota(dest, "/proc/sys/kernel/random/boot_id");

        r = sd_id128_randomize(&rnd);
        if (r < 0)
                return log_error_errno(r, "Failed to generate random boot id: %m");

        r = id128_write(from, ID128_UUID, rnd, false);
        if (r < 0)
                return log_error_errno(r, "Failed to write boot id: %m");

        r = mount_verbose(LOG_ERR, from, to, NULL, MS_BIND, NULL);
        if (r >= 0)
                r = mount_verbose(LOG_ERR, NULL, to, NULL,
                                  MS_BIND|MS_REMOUNT|MS_RDONLY|MS_NOSUID|MS_NODEV, NULL);

        (void) unlink(from);
        return r;
}

static int copy_devnodes(const char *dest) {

        static const char devnodes[] =
                "null\0"
                "zero\0"
                "full\0"
                "random\0"
                "urandom\0"
                "tty\0"
                "net/tun\0";

        const char *d;
        int r = 0;
        _cleanup_umask_ mode_t u;

        assert(dest);

        u = umask(0000);

        /* Create /dev/net, so that we can create /dev/net/tun in it */
        if (userns_mkdir(dest, "/dev/net", 0755, 0, 0) < 0)
                return log_error_errno(r, "Failed to create /dev/net directory: %m");

        NULSTR_FOREACH(d, devnodes) {
                _cleanup_free_ char *from = NULL, *to = NULL;
                struct stat st;

                from = strappend("/dev/", d);
                to = prefix_root(dest, from);

                if (stat(from, &st) < 0) {

                        if (errno != ENOENT)
                                return log_error_errno(errno, "Failed to stat %s: %m", from);

                } else if (!S_ISCHR(st.st_mode) && !S_ISBLK(st.st_mode)) {

                        log_error("%s is not a char or block device, cannot copy.", from);
                        return -EIO;

                } else {
                        if (mknod(to, st.st_mode, st.st_rdev) < 0) {
                                /*
                                 * This is some sort of protection too against
                                 * recursive userns chown on shared /dev/
                                 */
                                if (errno == EEXIST)
                                        log_notice("%s/dev/ should be an empty directory", dest);
                                if (errno != EPERM)
                                        return log_error_errno(errno, "mknod(%s) failed: %m", to);

                                /* Some systems abusively restrict mknod but
                                 * allow bind mounts. */
                                r = touch(to);
                                if (r < 0)
                                        return log_error_errno(r, "touch (%s) failed: %m", to);
                                r = mount_verbose(LOG_DEBUG, from, to, NULL, MS_BIND, NULL);
                                if (r < 0)
                                        return log_error_errno(r, "Both mknod and bind mount (%s) failed: %m", to);
                        }

                        r = userns_lchown(to, 0, 0);
                        if (r < 0)
                                return log_error_errno(r, "chown() of device node %s failed: %m", to);
                }
        }

        return r;
}

static int setup_pts(const char *dest) {
        _cleanup_free_ char *options = NULL;
        const char *p;
        int r;

#ifdef HAVE_SELINUX
        if (args->arg_selinux_apifs_context)
                (void) asprintf(&options,
                                "newinstance,ptmxmode=0666,mode=620,gid=" GID_FMT ",context=\"%s\"",
                                args->arg_uid_shift + TTY_GID,
                                args->arg_selinux_apifs_context);
        else
#endif
                (void) asprintf(&options,
                                "newinstance,ptmxmode=0666,mode=620,gid=" GID_FMT,
                                args->arg_uid_shift + TTY_GID);

        if (!options)
                return log_oom();

        /* Mount /dev/pts itself */
        p = prefix_roota(dest, "/dev/pts");
        if (mkdir(p, 0755) < 0)
                return log_error_errno(errno, "Failed to create /dev/pts: %m");
        r = mount_verbose(LOG_ERR, "devpts", p, "devpts", MS_NOSUID|MS_NOEXEC, options);
        if (r < 0)
                return r;
        r = userns_lchown(p, 0, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to chown /dev/pts: %m");

        /* Create /dev/ptmx symlink */
        p = prefix_roota(dest, "/dev/ptmx");
        if (symlink("pts/ptmx", p) < 0)
                return log_error_errno(errno, "Failed to create /dev/ptmx symlink: %m");
        r = userns_lchown(p, 0, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to chown /dev/ptmx: %m");

        /* And fix /dev/pts/ptmx ownership */
        p = prefix_roota(dest, "/dev/pts/ptmx");
        r = userns_lchown(p, 0, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to chown /dev/pts/ptmx: %m");

        return 0;
}

static int setup_dev_console(const char *dest, const char *console) {
        _cleanup_umask_ mode_t u;
        const char *to;
        int r;

        assert(dest);
        assert(console);

        u = umask(0000);

        r = chmod_and_chown(console, 0600, args->arg_uid_shift, args->arg_uid_shift);
        if (r < 0)
                return log_error_errno(r, "Failed to correct access mode for TTY: %m");

        /* We need to bind mount the right tty to /dev/console since
         * ptys can only exist on pts file systems. To have something
         * to bind mount things on we create a empty regular file. */

        to = prefix_roota(dest, "/dev/console");
        r = touch(to);
        if (r < 0)
                return log_error_errno(r, "touch() for /dev/console failed: %m");

        return mount_verbose(LOG_ERR, console, to, NULL, MS_BIND, NULL);
}

static int setup_kmsg(const char *dest, int kmsg_socket) {
        const char *from, *to;
        _cleanup_umask_ mode_t u;
        int fd, r;

        assert(kmsg_socket >= 0);

        u = umask(0000);

        /* We create the kmsg FIFO as /run/kmsg, but immediately
         * delete it after bind mounting it to /proc/kmsg. While FIFOs
         * on the reading side behave very similar to /proc/kmsg,
         * their writing side behaves differently from /dev/kmsg in
         * that writing blocks when nothing is reading. In order to
         * avoid any problems with containers deadlocking due to this
         * we simply make /dev/kmsg unavailable to the container. */
        from = prefix_roota(dest, "/run/kmsg");
        to = prefix_roota(dest, "/proc/kmsg");

        if (mkfifo(from, 0600) < 0)
                return log_error_errno(errno, "mkfifo() for /run/kmsg failed: %m");
        r = mount_verbose(LOG_ERR, from, to, NULL, MS_BIND, NULL);
        if (r < 0)
                return r;

        fd = open(from, O_RDWR|O_NDELAY|O_CLOEXEC);
        if (fd < 0)
                return log_error_errno(errno, "Failed to open fifo: %m");

        /* Store away the fd in the socket, so that it stays open as
         * long as we run the child */
        r = send_one_fd(kmsg_socket, fd, 0);
        safe_close(fd);

        if (r < 0)
                return log_error_errno(r, "Failed to send FIFO fd: %m");

        /* And now make the FIFO unavailable as /run/kmsg... */
        (void) unlink(from);

        return 0;
}

static int on_address_change(sd_netlink *rtnl, sd_netlink_message *m, void *userdata) {
        union in_addr_union *exposed = userdata;

        assert(rtnl);
        assert(m);
        assert(exposed);

        expose_port_execute(rtnl, args->arg_expose_ports, exposed);
        return 0;
}

static int setup_hostname(void) {

        if ((args->arg_clone_ns_flags & CLONE_NEWUTS) == 0)
                return 0;

        if (sethostname_idempotent(args->arg_machine) < 0)
                return -errno;

        return 0;
}

static int setup_journal(const char *directory) {
        sd_id128_t this_id;
        _cleanup_free_ char *d = NULL;
        const char *p, *q;
        bool try;
        char id[33];
        int r;

        /* Don't link journals in ephemeral mode */
        if (args->arg_ephemeral)
                return 0;

        if (args->arg_link_journal == LINK_NO)
                return 0;

        try = args->arg_link_journal_try || args->arg_link_journal == LINK_AUTO;

        r = sd_id128_get_machine(&this_id);
        if (r < 0)
                return log_error_errno(r, "Failed to retrieve machine ID: %m");

        if (sd_id128_equal(args->arg_uuid, this_id)) {
                log_full(try ? LOG_WARNING : LOG_ERR,
                         "Host and machine ids are equal (%s): refusing to link journals", sd_id128_to_string(args->arg_uuid, id));
                if (try)
                        return 0;
                return -EEXIST;
        }

        r = userns_mkdir(directory, "/var", 0755, 0, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to create /var: %m");

        r = userns_mkdir(directory, "/var/log", 0755, 0, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to create /var/log: %m");

        r = userns_mkdir(directory, "/var/log/journal", 0755, 0, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to create /var/log/journal: %m");

        (void) sd_id128_to_string(args->arg_uuid, id);

        p = strjoina("/var/log/journal/", id);
        q = prefix_roota(directory, p);

        if (path_is_mount_point(p, 0) > 0) {
                if (try)
                        return 0;

                log_error("%s: already a mount point, refusing to use for journal", p);
                return -EEXIST;
        }

        if (path_is_mount_point(q, 0) > 0) {
                if (try)
                        return 0;

                log_error("%s: already a mount point, refusing to use for journal", q);
                return -EEXIST;
        }

        r = readlink_and_make_absolute(p, &d);
        if (r >= 0) {
                if ((args->arg_link_journal == LINK_GUEST ||
                     args->arg_link_journal == LINK_AUTO) &&
                    path_equal(d, q)) {

                        r = userns_mkdir(directory, p, 0755, 0, 0);
                        if (r < 0)
                                log_warning_errno(r, "Failed to create directory %s: %m", q);
                        return 0;
                }

                if (unlink(p) < 0)
                        return log_error_errno(errno, "Failed to remove symlink %s: %m", p);
        } else if (r == -EINVAL) {

                if (args->arg_link_journal == LINK_GUEST &&
                    rmdir(p) < 0) {

                        if (errno == ENOTDIR) {
                                log_error("%s already exists and is neither a symlink nor a directory", p);
                                return r;
                        } else
                                return log_error_errno(errno, "Failed to remove %s: %m", p);
                }
        } else if (r != -ENOENT)
                return log_error_errno(r, "readlink(%s) failed: %m", p);

        if (args->arg_link_journal == LINK_GUEST) {

                if (symlink(q, p) < 0) {
                        if (try) {
                                log_debug_errno(errno, "Failed to symlink %s to %s, skipping journal setup: %m", q, p);
                                return 0;
                        } else
                                return log_error_errno(errno, "Failed to symlink %s to %s: %m", q, p);
                }

                r = userns_mkdir(directory, p, 0755, 0, 0);
                if (r < 0)
                        log_warning_errno(r, "Failed to create directory %s: %m", q);
                return 0;
        }

        if (args->arg_link_journal == LINK_HOST) {
                /* don't create parents here — if the host doesn't have
                 * permanent journal set up, don't force it here */

                if (mkdir(p, 0755) < 0 && errno != EEXIST) {
                        if (try) {
                                log_debug_errno(errno, "Failed to create %s, skipping journal setup: %m", p);
                                return 0;
                        } else
                                return log_error_errno(errno, "Failed to create %s: %m", p);
                }

        } else if (access(p, F_OK) < 0)
                return 0;

        if (dir_is_empty(q) == 0)
                log_warning("%s is not empty, proceeding anyway.", q);

        r = userns_mkdir(directory, p, 0755, 0, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to create %s: %m", q);

        r = mount_verbose(LOG_DEBUG, p, q, NULL, MS_BIND, NULL);
        if (r < 0)
                return log_error_errno(errno, "Failed to bind mount journal from host into guest: %m");

        return 0;
}

static int drop_capabilities(void) {
        return capability_bounding_set_drop(args->arg_caps_retain, false);
}

static int reset_audit_loginuid(void) {
        _cleanup_free_ char *p = NULL;
        int r;

        if ((args->arg_clone_ns_flags & CLONE_NEWPID) == 0)
                return 0;

        r = read_one_line_file("/proc/self/loginuid", &p);
        if (r == -ENOENT)
                return 0;
        if (r < 0)
                return log_error_errno(r, "Failed to read /proc/self/loginuid: %m");

        /* Already reset? */
        if (streq(p, "4294967295"))
                return 0;

        r = write_string_file("/proc/self/loginuid", "4294967295", 0);
        if (r < 0) {
                log_error_errno(r,
                                "Failed to reset audit login UID. This probably means that your kernel is too\n"
                                "old and you have audit enabled. Note that the auditing subsystem is known to\n"
                                "be incompatible with containers on old kernels. Please make sure to upgrade\n"
                                "your kernel or to off auditing with 'audit=0' on the kernel command line before\n"
                                "using systemd-nspawn. Sleeping for 5s... (%m)");

                sleep(5);
        }

        return 0;
}


static int setup_propagate(const char *root) {
        const char *p, *q;
        int r;

        (void) mkdir_p("/run/systemd/nspawn/", 0755);
        (void) mkdir_p("/run/systemd/nspawn/propagate", 0600);
        p = strjoina("/run/systemd/nspawn/propagate/", args->arg_machine);
        (void) mkdir_p(p, 0600);

        r = userns_mkdir(root, "/run/systemd", 0755, 0, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to create /run/systemd: %m");

        r = userns_mkdir(root, "/run/systemd/nspawn", 0755, 0, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to create /run/systemd/nspawn: %m");

        r = userns_mkdir(root, "/run/systemd/nspawn/incoming", 0600, 0, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to create /run/systemd/nspawn/incoming: %m");

        q = prefix_roota(root, "/run/systemd/nspawn/incoming");
        r = mount_verbose(LOG_ERR, p, q, NULL, MS_BIND, NULL);
        if (r < 0)
                return r;

        r = mount_verbose(LOG_ERR, NULL, q, NULL, MS_BIND|MS_REMOUNT|MS_RDONLY, NULL);
        if (r < 0)
                return r;

        /* machined will MS_MOVE into that directory, and that's only
         * supported for non-shared mounts. */
        return mount_verbose(LOG_ERR, NULL, q, NULL, MS_SLAVE, NULL);
}

static int setup_image(char **device_path, int *loop_nr) {
        struct loop_info64 info = {
                .lo_flags = LO_FLAGS_AUTOCLEAR|LO_FLAGS_PARTSCAN
        };
        _cleanup_close_ int fd = -1, control = -1, loop = -1;
        _cleanup_free_ char* loopdev = NULL;
        struct stat st;
        int r, nr;

        assert(device_path);
        assert(loop_nr);
        assert(args->arg_image);

        fd = open(args->arg_image, O_CLOEXEC|(args->arg_read_only ? O_RDONLY : O_RDWR)|O_NONBLOCK|O_NOCTTY);
        if (fd < 0)
                return log_error_errno(errno, "Failed to open %s: %m", args->arg_image);

        if (fstat(fd, &st) < 0)
                return log_error_errno(errno, "Failed to stat %s: %m", args->arg_image);

        if (S_ISBLK(st.st_mode)) {
                char *p;

                p = strdup(args->arg_image);
                if (!p)
                        return log_oom();

                *device_path = p;

                *loop_nr = -1;

                r = fd;
                fd = -1;

                return r;
        }

        if (!S_ISREG(st.st_mode)) {
                log_error("%s is not a regular file or block device.", args->arg_image);
                return -EINVAL;
        }

        control = open("/dev/loop-control", O_RDWR|O_CLOEXEC|O_NOCTTY|O_NONBLOCK);
        if (control < 0)
                return log_error_errno(errno, "Failed to open /dev/loop-control: %m");

        nr = ioctl(control, LOOP_CTL_GET_FREE);
        if (nr < 0)
                return log_error_errno(errno, "Failed to allocate loop device: %m");

        if (asprintf(&loopdev, "/dev/loop%i", nr) < 0)
                return log_oom();

        loop = open(loopdev, O_CLOEXEC|(args->arg_read_only ? O_RDONLY : O_RDWR)|O_NONBLOCK|O_NOCTTY);
        if (loop < 0)
                return log_error_errno(errno, "Failed to open loop device %s: %m", loopdev);

        if (ioctl(loop, LOOP_SET_FD, fd) < 0)
                return log_error_errno(errno, "Failed to set loopback file descriptor on %s: %m", loopdev);

        if (args->arg_read_only)
                info.lo_flags |= LO_FLAGS_READ_ONLY;

        if (ioctl(loop, LOOP_SET_STATUS64, &info) < 0)
                return log_error_errno(errno, "Failed to set loopback settings on %s: %m", loopdev);

        *device_path = loopdev;
        loopdev = NULL;

        *loop_nr = nr;

        r = loop;
        loop = -1;

        return r;
}

#define PARTITION_TABLE_BLURB \
        "Note that the disk image needs to either contain only a single MBR partition of\n" \
        "type 0x83 that is marked bootable, or a single GPT partition of type " \
        "0FC63DAF-8483-4772-8E79-3D69D8477DE4 or follow\n" \
        "    http://www.freedesktop.org/wiki/Specifications/DiscoverablePartitionsSpec/\n" \
        "to be bootable with systemd-nspawn."

static int dissect_image(
                int fd,
                char **root_device, bool *root_device_rw,
                char **home_device, bool *home_device_rw,
                char **srv_device, bool *srv_device_rw,
                char **esp_device,
                bool *secondary) {

#ifdef HAVE_BLKID
        int home_nr = -1, srv_nr = -1, esp_nr = -1;
#ifdef GPT_ROOT_NATIVE
        int root_nr = -1;
#endif
#ifdef GPT_ROOT_SECONDARY
        int secondary_root_nr = -1;
#endif
        _cleanup_free_ char *home = NULL, *root = NULL, *secondary_root = NULL, *srv = NULL, *esp = NULL, *generic = NULL;
        _cleanup_udev_enumerate_unref_ struct udev_enumerate *e = NULL;
        _cleanup_udev_device_unref_ struct udev_device *d = NULL;
        _cleanup_blkid_free_probe_ blkid_probe b = NULL;
        _cleanup_udev_unref_ struct udev *udev = NULL;
        struct udev_list_entry *first, *item;
        bool home_rw = true, root_rw = true, secondary_root_rw = true, srv_rw = true, generic_rw = true;
        bool is_gpt, is_mbr, multiple_generic = false;
        const char *pttype = NULL;
        blkid_partlist pl;
        struct stat st;
        unsigned i;
        int r;

        assert(fd >= 0);
        assert(root_device);
        assert(home_device);
        assert(srv_device);
        assert(esp_device);
        assert(secondary);
        assert(args->arg_image);

        b = blkid_new_probe();
        if (!b)
                return log_oom();

        errno = 0;
        r = blkid_probe_set_device(b, fd, 0, 0);
        if (r != 0) {
                if (errno == 0)
                        return log_oom();

                return log_error_errno(errno, "Failed to set device on blkid probe: %m");
        }

        blkid_probe_enable_partitions(b, 1);
        blkid_probe_set_partitions_flags(b, BLKID_PARTS_ENTRY_DETAILS);

        errno = 0;
        r = blkid_do_safeprobe(b);
        if (r == -2 || r == 1) {
                log_error("Failed to identify any partition table on\n"
                          "    %s\n"
                          PARTITION_TABLE_BLURB, args->arg_image);
                return -EINVAL;
        } else if (r != 0) {
                if (errno == 0)
                        errno = EIO;
                return log_error_errno(errno, "Failed to probe: %m");
        }

        (void) blkid_probe_lookup_value(b, "PTTYPE", &pttype, NULL);

        is_gpt = streq_ptr(pttype, "gpt");
        is_mbr = streq_ptr(pttype, "dos");

        if (!is_gpt && !is_mbr) {
                log_error("No GPT or MBR partition table discovered on\n"
                          "    %s\n"
                          PARTITION_TABLE_BLURB, args->arg_image);
                return -EINVAL;
        }

        errno = 0;
        pl = blkid_probe_get_partitions(b);
        if (!pl) {
                if (errno == 0)
                        return log_oom();

                log_error("Failed to list partitions of %s", args->arg_image);
                return -errno;
        }

        udev = udev_new();
        if (!udev)
                return log_oom();

        if (fstat(fd, &st) < 0)
                return log_error_errno(errno, "Failed to stat block device: %m");

        d = udev_device_new_from_devnum(udev, 'b', st.st_rdev);
        if (!d)
                return log_oom();

        for (i = 0;; i++) {
                int n, m;

                if (i >= 10) {
                        log_error("Kernel partitions never appeared.");
                        return -ENXIO;
                }

                e = udev_enumerate_new(udev);
                if (!e)
                        return log_oom();

                r = udev_enumerate_add_match_parent(e, d);
                if (r < 0)
                        return log_oom();

                r = udev_enumerate_scan_devices(e);
                if (r < 0)
                        return log_error_errno(r, "Failed to scan for partition devices of %s: %m", args->arg_image);

                /* Count the partitions enumerated by the kernel */
                n = 0;
                first = udev_enumerate_get_list_entry(e);
                udev_list_entry_foreach(item, first)
                        n++;

                /* Count the partitions enumerated by blkid */
                m = blkid_partlist_numof_partitions(pl);
                if (n == m + 1)
                        break;
                if (n > m + 1) {
                        log_error("blkid and kernel partition list do not match.");
                        return -EIO;
                }
                if (n < m + 1) {
                        unsigned j;

                        /* The kernel has probed fewer partitions than
                         * blkid? Maybe the kernel prober is still
                         * running or it got EBUSY because udev
                         * already opened the device. Let's reprobe
                         * the device, which is a synchronous call
                         * that waits until probing is complete. */

                        for (j = 0; j < 20; j++) {

                                r = ioctl(fd, BLKRRPART, 0);
                                if (r < 0)
                                        r = -errno;
                                if (r >= 0 || r != -EBUSY)
                                        break;

                                /* If something else has the device
                                 * open, such as an udev rule, the
                                 * ioctl will return EBUSY. Since
                                 * there's no way to wait until it
                                 * isn't busy anymore, let's just wait
                                 * a bit, and try again.
                                 *
                                 * This is really something they
                                 * should fix in the kernel! */

                                usleep(50 * USEC_PER_MSEC);
                        }

                        if (r < 0)
                                return log_error_errno(r, "Failed to reread partition table: %m");
                }

                e = udev_enumerate_unref(e);
        }

        first = udev_enumerate_get_list_entry(e);
        udev_list_entry_foreach(item, first) {
                _cleanup_udev_device_unref_ struct udev_device *q;
                const char *node;
                unsigned long long flags;
                blkid_partition pp;
                dev_t qn;
                int nr;

                errno = 0;
                q = udev_device_new_from_syspath(udev, udev_list_entry_get_name(item));
                if (!q) {
                        if (!errno)
                                errno = ENOMEM;

                        return log_error_errno(errno, "Failed to get partition device of %s: %m", args->arg_image);
                }

                qn = udev_device_get_devnum(q);
                if (major(qn) == 0)
                        continue;

                if (st.st_rdev == qn)
                        continue;

                node = udev_device_get_devnode(q);
                if (!node)
                        continue;

                pp = blkid_partlist_devno_to_partition(pl, qn);
                if (!pp)
                        continue;

                flags = blkid_partition_get_flags(pp);

                nr = blkid_partition_get_partno(pp);
                if (nr < 0)
                        continue;

                if (is_gpt) {
                        sd_id128_t type_id;
                        const char *stype;

                        if (flags & GPT_FLAG_NO_AUTO)
                                continue;

                        stype = blkid_partition_get_type_string(pp);
                        if (!stype)
                                continue;

                        if (sd_id128_from_string(stype, &type_id) < 0)
                                continue;

                        if (sd_id128_equal(type_id, GPT_HOME)) {

                                if (home && nr >= home_nr)
                                        continue;

                                home_nr = nr;
                                home_rw = !(flags & GPT_FLAG_READ_ONLY);

                                r = free_and_strdup(&home, node);
                                if (r < 0)
                                        return log_oom();

                        } else if (sd_id128_equal(type_id, GPT_SRV)) {

                                if (srv && nr >= srv_nr)
                                        continue;

                                srv_nr = nr;
                                srv_rw = !(flags & GPT_FLAG_READ_ONLY);

                                r = free_and_strdup(&srv, node);
                                if (r < 0)
                                        return log_oom();
                        } else if (sd_id128_equal(type_id, GPT_ESP)) {

                                if (esp && nr >= esp_nr)
                                        continue;

                                esp_nr = nr;

                                r = free_and_strdup(&esp, node);
                                if (r < 0)
                                        return log_oom();
                        }
#ifdef GPT_ROOT_NATIVE
                        else if (sd_id128_equal(type_id, GPT_ROOT_NATIVE)) {

                                if (root && nr >= root_nr)
                                        continue;

                                root_nr = nr;
                                root_rw = !(flags & GPT_FLAG_READ_ONLY);

                                r = free_and_strdup(&root, node);
                                if (r < 0)
                                        return log_oom();
                        }
#endif
#ifdef GPT_ROOT_SECONDARY
                        else if (sd_id128_equal(type_id, GPT_ROOT_SECONDARY)) {

                                if (secondary_root && nr >= secondary_root_nr)
                                        continue;

                                secondary_root_nr = nr;
                                secondary_root_rw = !(flags & GPT_FLAG_READ_ONLY);

                                r = free_and_strdup(&secondary_root, node);
                                if (r < 0)
                                        return log_oom();
                        }
#endif
                        else if (sd_id128_equal(type_id, GPT_LINUX_GENERIC)) {

                                if (generic)
                                        multiple_generic = true;
                                else {
                                        generic_rw = !(flags & GPT_FLAG_READ_ONLY);

                                        r = free_and_strdup(&generic, node);
                                        if (r < 0)
                                                return log_oom();
                                }
                        }

                } else if (is_mbr) {
                        int type;

                        if (flags != 0x80) /* Bootable flag */
                                continue;

                        type = blkid_partition_get_type(pp);
                        if (type != 0x83) /* Linux partition */
                                continue;

                        if (generic)
                                multiple_generic = true;
                        else {
                                generic_rw = true;

                                r = free_and_strdup(&root, node);
                                if (r < 0)
                                        return log_oom();
                        }
                }
        }

        if (root) {
                *root_device = root;
                root = NULL;

                *root_device_rw = root_rw;
                *secondary = false;
        } else if (secondary_root) {
                *root_device = secondary_root;
                secondary_root = NULL;

                *root_device_rw = secondary_root_rw;
                *secondary = true;
        } else if (generic) {

                /* There were no partitions with precise meanings
                 * around, but we found generic partitions. In this
                 * case, if there's only one, we can go ahead and boot
                 * it, otherwise we bail out, because we really cannot
                 * make any sense of it. */

                if (multiple_generic) {
                        log_error("Identified multiple bootable Linux partitions on\n"
                                  "    %s\n"
                                  PARTITION_TABLE_BLURB, args->arg_image);
                        return -EINVAL;
                }

                *root_device = generic;
                generic = NULL;

                *root_device_rw = generic_rw;
                *secondary = false;
        } else {
                log_error("Failed to identify root partition in disk image\n"
                          "    %s\n"
                          PARTITION_TABLE_BLURB, args->arg_image);
                return -EINVAL;
        }

        if (home) {
                *home_device = home;
                home = NULL;

                *home_device_rw = home_rw;
        }

        if (srv) {
                *srv_device = srv;
                srv = NULL;

                *srv_device_rw = srv_rw;
        }

        if (esp) {
                *esp_device = esp;
                esp = NULL;
        }

        return 0;
#else
        log_error("--image= is not supported, compiled without blkid support.");
        return -EOPNOTSUPP;
#endif
}

static int mount_device(const char *what, const char *where, const char *directory, bool rw) {
#ifdef HAVE_BLKID
        _cleanup_blkid_free_probe_ blkid_probe b = NULL;
        const char *fstype, *p, *options;
        int r;

        assert(what);
        assert(where);

        if (args->arg_read_only)
                rw = false;

        if (directory)
                p = strjoina(where, directory);
        else
                p = where;

        errno = 0;
        b = blkid_new_probe_from_filename(what);
        if (!b) {
                if (errno == 0)
                        return log_oom();
                return log_error_errno(errno, "Failed to allocate prober for %s: %m", what);
        }

        blkid_probe_enable_superblocks(b, 1);
        blkid_probe_set_superblocks_flags(b, BLKID_SUBLKS_TYPE);

        errno = 0;
        r = blkid_do_safeprobe(b);
        if (r == -1 || r == 1) {
                log_error("Cannot determine file system type of %s", what);
                return -EINVAL;
        } else if (r != 0) {
                if (errno == 0)
                        errno = EIO;
                return log_error_errno(errno, "Failed to probe %s: %m", what);
        }

        errno = 0;
        if (blkid_probe_lookup_value(b, "TYPE", &fstype, NULL) < 0) {
                if (errno == 0)
                        errno = EINVAL;
                log_error("Failed to determine file system type of %s", what);
                return -errno;
        }

        if (streq(fstype, "crypto_LUKS")) {
                log_error("nspawn currently does not support LUKS disk images.");
                return -EOPNOTSUPP;
        }

        /* If this is a loopback device then let's mount the image with discard, so that the underlying file remains
         * sparse when possible. */
        if (STR_IN_SET(fstype, "btrfs", "ext4", "vfat", "xfs")) {
                const char *l;

                l = path_startswith(what, "/dev");
                if (l && startswith(l, "loop"))
                        options = "discard";
        }

        return mount_verbose(LOG_ERR, what, p, fstype, MS_NODEV|(rw ? 0 : MS_RDONLY), options);
#else
        log_error("--image= is not supported, compiled without blkid support.");
        return -EOPNOTSUPP;
#endif
}

static int recursive_chown(const char *directory, uid_t shift, uid_t range) {
        int r;

        assert(directory);

        if (args->arg_userns_mode == USER_NAMESPACE_NO || !args->arg_userns_chown)
                return 0;

        r = path_patch_uid(directory, args->arg_uid_shift, args->arg_uid_range);
        if (r == -EOPNOTSUPP)
                return log_error_errno(r, "Automatic UID/GID adjusting is only supported for UID/GID ranges starting at multiples of 2^16 with a range of 2^16.");
        if (r == -EBADE)
                return log_error_errno(r, "Upper 16 bits of root directory UID and GID do not match.");
        if (r < 0)
                return log_error_errno(r, "Failed to adjust UID/GID shift of OS tree: %m");
        if (r == 0)
                log_debug("Root directory of image is already owned by the right UID/GID range, skipping recursive chown operation.");
        else
                log_debug("Patched directory tree to match UID/GID range.");

        return r;
}

static int mount_devices(
                const char *where,
                const char *root_device, bool root_device_rw,
                const char *home_device, bool home_device_rw,
                const char *srv_device, bool srv_device_rw,
                const char *esp_device) {
        int r;

        assert(where);

        if (root_device) {
                r = mount_device(root_device, args->arg_directory, NULL, root_device_rw);
                if (r < 0)
                        return log_error_errno(r, "Failed to mount root directory: %m");
        }

        if (home_device) {
                r = mount_device(home_device, args->arg_directory, "/home", home_device_rw);
                if (r < 0)
                        return log_error_errno(r, "Failed to mount home directory: %m");
        }

        if (srv_device) {
                r = mount_device(srv_device, args->arg_directory, "/srv", srv_device_rw);
                if (r < 0)
                        return log_error_errno(r, "Failed to mount server data directory: %m");
        }

        if (esp_device) {
                const char *mp, *x;

                /* Mount the ESP to /efi if it exists and is empty. If it doesn't exist, use /boot instead. */

                mp = "/efi";
                x = strjoina(args->arg_directory, mp);
                r = dir_is_empty(x);
                if (r == -ENOENT) {
                        mp = "/boot";
                        x = strjoina(args->arg_directory, mp);
                        r = dir_is_empty(x);
                }

                if (r > 0) {
                        r = mount_device(esp_device, args->arg_directory, mp, true);
                        if (r < 0)
                                return log_error_errno(r, "Failed to  mount ESP: %m");
                }
        }

        return 0;
}

static void loop_remove(int nr, int *image_fd) {
        _cleanup_close_ int control = -1;
        int r;

        if (nr < 0)
                return;

        if (image_fd && *image_fd >= 0) {
                r = ioctl(*image_fd, LOOP_CLR_FD);
                if (r < 0)
                        log_debug_errno(errno, "Failed to close loop image: %m");
                *image_fd = safe_close(*image_fd);
        }

        control = open("/dev/loop-control", O_RDWR|O_CLOEXEC|O_NOCTTY|O_NONBLOCK);
        if (control < 0) {
                log_warning_errno(errno, "Failed to open /dev/loop-control: %m");
                return;
        }

        r = ioctl(control, LOOP_CTL_REMOVE, nr);
        if (r < 0)
                log_debug_errno(errno, "Failed to remove loop %d: %m", nr);
}

/*
 * Return values:
 * < 0 : wait_for_terminate() failed to get the state of the
 *       container, the container was terminated by a signal, or
 *       failed for an unknown reason.  No change is made to the
 *       container argument.
 * > 0 : The program executed in the container terminated with an
 *       error.  The exit code of the program executed in the
 *       container is returned.  The container argument has been set
 *       to CONTAINER_TERMINATED.
 *   0 : The container is being rebooted, has been shut down or exited
 *       successfully.  The container argument has been set to either
 *       CONTAINER_TERMINATED or CONTAINER_REBOOTED.
 *
 * That is, success is indicated by a return value of zero, and an
 * error is indicated by a non-zero value.
 */
static int wait_for_container(pid_t pid, ContainerStatus *container) {
        siginfo_t status;
        int r;

        r = wait_for_terminate(pid, &status);
        if (r < 0)
                return log_warning_errno(r, "Failed to wait for container: %m");

        switch (status.si_code) {

        case CLD_EXITED:
                if (status.si_status == 0)
                        log_full(args->arg_quiet ? LOG_DEBUG : LOG_INFO, "Container %s exited successfully.", args->arg_machine);
                else
                        log_full(args->arg_quiet ? LOG_DEBUG : LOG_INFO, "Container %s failed with error code %i.", args->arg_machine, status.si_status);

                *container = CONTAINER_TERMINATED;
                return status.si_status;

        case CLD_KILLED:
                if (status.si_status == SIGINT) {
                        log_full(args->arg_quiet ? LOG_DEBUG : LOG_INFO, "Container %s has been shut down.", args->arg_machine);
                        *container = CONTAINER_TERMINATED;
                        return 0;

                } else if (status.si_status == SIGHUP) {
                        log_full(args->arg_quiet ? LOG_DEBUG : LOG_INFO, "Container %s is being rebooted.", args->arg_machine);
                        *container = CONTAINER_REBOOTED;
                        return 0;
                }

                /* CLD_KILLED fallthrough */

        case CLD_DUMPED:
                log_error("Container %s terminated by signal %s.", args->arg_machine, signal_to_string(status.si_status));
                return -EIO;

        default:
                log_error("Container %s failed due to unknown reason.", args->arg_machine);
                return -EIO;
        }
}

static int on_orderly_shutdown(sd_event_source *s, const struct signalfd_siginfo *si, void *userdata) {
        pid_t pid;

        pid = PTR_TO_PID(userdata);
        if (pid > 0) {
                if (kill(pid, args->arg_kill_signal) >= 0) {
                        log_info("Trying to halt container. Send SIGTERM again to trigger immediate termination.");
                        sd_event_source_set_userdata(s, NULL);
                        return 0;
                }
        }

        sd_event_exit(sd_event_source_get_event(s), 0);
        return 0;
}

static int inner_child(
                Barrier *barrier,
                const char *directory,
                bool secondary,
                int kmsg_socket,
                int rtnl_socket,
                FDSet *fds) {

        _cleanup_free_ char *home = NULL;
        char as_uuid[37];
        unsigned n_env = 1;
        const char *envp[] = {
                "PATH=" DEFAULT_PATH_SPLIT_USR,
                NULL, /* container */
                NULL, /* TERM */
                NULL, /* HOME */
                NULL, /* USER */
                NULL, /* LOGNAME */
                NULL, /* container_uuid */
                NULL, /* LISTEN_FDS */
                NULL, /* LISTEN_PID */
                NULL, /* NOTIFY_SOCKET */
                NULL
        };

        _cleanup_strv_free_ char **env_use = NULL;
        int r;

        assert(barrier);
        assert(directory);
        assert(kmsg_socket >= 0);

        if (args->arg_userns_mode != USER_NAMESPACE_NO) {
                /* Tell the parent, that it now can write the UID map. */
                (void) barrier_place(barrier); /* #1 */

                /* Wait until the parent wrote the UID map */
                if (!barrier_place_and_sync(barrier)) { /* #2 */
                        log_error("Parent died too early");
                        return -ESRCH;
                }
        }

        r = reset_uid_gid();
        if (r < 0)
                return log_error_errno(r, "Couldn't become new root: %m");

        r = mount_all(NULL,
                      args->arg_userns_mode != USER_NAMESPACE_NO,
                      true,
                      args->arg_private_network,
                      args->arg_uid_shift,
                      args->arg_uid_range,
                      args->arg_selinux_apifs_context);

        if (r < 0)
                return r;

        r = mount_sysfs(NULL);
        if (r < 0)
                return r;

        /* Wait until we are cgroup-ified, so that we
         * can mount the right cgroup path writable */
        if (!barrier_place_and_sync(barrier)) { /* #3 */
                log_error("Parent died too early");
                return -ESRCH;
        }

        if (args->arg_use_cgns) {
                r = unshare(CLONE_NEWCGROUP);
                if (r < 0)
                        return log_error_errno(errno, "Failed to unshare cgroup namespace");
                r = mount_cgroups(
                                "",
                                args->arg_unified_cgroup_hierarchy,
                                args->arg_userns_mode != USER_NAMESPACE_NO,
                                args->arg_uid_shift,
                                args->arg_uid_range,
                                args->arg_selinux_apifs_context,
                                true);
                if (r < 0)
                        return r;
        } else {
                r = mount_systemd_cgroup_writable("", args->arg_unified_cgroup_hierarchy);
                if (r < 0)
                        return r;
        }

        r = setup_boot_id(NULL);
        if (r < 0)
                return r;

        r = setup_kmsg(NULL, kmsg_socket);
        if (r < 0)
                return r;
        kmsg_socket = safe_close(kmsg_socket);

        umask(0022);

        if (setsid() < 0)
                return log_error_errno(errno, "setsid() failed: %m");

        if (args->arg_private_network)
                loopback_setup();

        if (args->arg_expose_ports) {
                r = expose_port_send_rtnl(rtnl_socket);
                if (r < 0)
                        return r;
                rtnl_socket = safe_close(rtnl_socket);
        }

        r = drop_capabilities();
        if (r < 0)
                return log_error_errno(r, "drop_capabilities() failed: %m");

        setup_hostname();

        if (args->arg_personality != PERSONALITY_INVALID) {
                if (personality(args->arg_personality) < 0)
                        return log_error_errno(errno, "personality() failed: %m");
        } else if (secondary) {
                if (personality(PER_LINUX32) < 0)
                        return log_error_errno(errno, "personality() failed: %m");
        }

#ifdef HAVE_SELINUX
        if (args->arg_selinux_context)
                if (setexeccon(args->arg_selinux_context) < 0)
                        return log_error_errno(errno, "setexeccon(\"%s\") failed: %m", args->arg_selinux_context);
#endif

        r = change_uid_gid(args->arg_user, &home);
        if (r < 0)
                return r;

        /* LXC sets container=lxc, so follow the scheme here */
        envp[n_env++] = strjoina("container=", args->arg_container_service_name);

        envp[n_env] = strv_find_prefix(environ, "TERM=");
        if (envp[n_env])
                n_env++;

        if ((asprintf((char**)(envp + n_env++), "HOME=%s", home ? home: "/root") < 0) ||
            (asprintf((char**)(envp + n_env++), "USER=%s", args->arg_user ? args->arg_user : "root") < 0) ||
            (asprintf((char**)(envp + n_env++), "LOGNAME=%s", args->arg_user ? args->arg_user : "root") < 0))
                return log_oom();

        assert(!sd_id128_is_null(args->arg_uuid));

        if (asprintf((char**)(envp + n_env++), "container_uuid=%s", id128_to_uuid_string(args->arg_uuid, as_uuid)) < 0)
                return log_oom();

        if (fdset_size(fds) > 0) {
                r = fdset_cloexec(fds, false);
                if (r < 0)
                        return log_error_errno(r, "Failed to unset O_CLOEXEC for file descriptors.");

                if ((asprintf((char **)(envp + n_env++), "LISTEN_FDS=%u", fdset_size(fds)) < 0) ||
                    (asprintf((char **)(envp + n_env++), "LISTEN_PID=1") < 0))
                        return log_oom();
        }
        if (asprintf((char **)(envp + n_env++), "NOTIFY_SOCKET=%s", NSPAWN_NOTIFY_SOCKET_PATH) < 0)
                return log_oom();

        env_use = strv_env_merge(2, envp, args->arg_setenv);
        if (!env_use)
                return log_oom();

        /* Let the parent know that we are ready and
         * wait until the parent is ready with the
         * setup, too... */
        if (!barrier_place_and_sync(barrier)) { /* #4 */
                log_error("Parent died too early");
                return -ESRCH;
        }

        if (args->arg_chdir)
                if (chdir(args->arg_chdir) < 0)
                        return log_error_errno(errno, "Failed to change to specified working directory %s: %m", args->arg_chdir);

        switch (args->arg_start_mode) {
        default:
                assert_not_reached("Unrecognized args->arg_start_mode");
                return -EINVAL;
        case START_PID2:
                r = stub_pid1();
                if (r < 0)
                        return r;
                /* fall through */
        case START_PID1:
                /* Now, explicitly close the log, so that we
                 * then can close all remaining fds. Closing
                 * the log explicitly first has the benefit
                 * that the logging subsystem knows about it,
                 * and is thus ready to be reopened should we
                 * need it again. Note that the other fds
                 * closed here are at least the locking and
                 * barrier fds. */
                log_close();
                (void) fdset_close_others(fds);

                if (!strv_isempty(args->arg_parameters))
                        execvpe(args->arg_parameters[0], args->arg_parameters, env_use);
                else {
                        if (!args->arg_chdir)
                                /* If we cannot change the directory, we'll end up in /, that is expected. */
                                (void) chdir(home ?: "/root");

                        execle("/bin/bash", "-bash", NULL, env_use);
                        execle("/bin/sh", "-sh", NULL, env_use);
                }
                break;
        case START_BOOT:
                log_close(); /* see the above comment */
                (void) fdset_close_others(fds);

                if (true) { /* to create a stack to put `a` and `m` on */
                        char **a;
                        size_t m;

                        /* Automatically search for the init system */

                        m = strv_length(args->arg_parameters);
                        a = newa(char*, m + 2);
                        memcpy_safe(a + 1, args->arg_parameters, m * sizeof(char*));
                        a[1 + m] = NULL;

                        a[0] = (char*) "/usr/lib/systemd/systemd";
                        execve(a[0], a, env_use);

                        a[0] = (char*) "/lib/systemd/systemd";
                        execve(a[0], a, env_use);

                        a[0] = (char*) "/sbin/init";
                        execve(a[0], a, env_use);
                }
                break;
        }

        r = -errno;
        (void) log_open();
        return log_error_errno(r, "execv() failed: %m");
}

static int setup_sd_notify_child(void) {
        static const int one = 1;
        int fd = -1;
        union sockaddr_union sa = {
                .sa.sa_family = AF_UNIX,
        };
        int r;

        fd = socket(AF_UNIX, SOCK_DGRAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
        if (fd < 0)
                return log_error_errno(errno, "Failed to allocate notification socket: %m");

        (void) mkdir_parents(NSPAWN_NOTIFY_SOCKET_PATH, 0755);
        (void) unlink(NSPAWN_NOTIFY_SOCKET_PATH);

        strncpy(sa.un.sun_path, NSPAWN_NOTIFY_SOCKET_PATH, sizeof(sa.un.sun_path)-1);
        r = bind(fd, &sa.sa, SOCKADDR_UN_LEN(sa.un));
        if (r < 0) {
                safe_close(fd);
                return log_error_errno(errno, "bind(%s) failed: %m", sa.un.sun_path);
        }

        r = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, sizeof(one));
        if (r < 0) {
                safe_close(fd);
                return log_error_errno(errno, "SO_PASSCRED failed: %m");
        }

        return fd;
}

static int outer_child(
                Barrier *barrier,
                const char *console,
                const char *root_device, bool root_device_rw,
                const char *home_device, bool home_device_rw,
                const char *srv_device, bool srv_device_rw,
                const char *esp_device,
                bool interactive,
                bool secondary,
                int pid_socket,
                int uuid_socket,
                int notify_socket,
                int kmsg_socket,
                int rtnl_socket,
                int uid_shift_socket,
                FDSet *fds) {

        pid_t pid;
        ssize_t l;
        int r;
        _cleanup_close_ int fd = -1;

        assert(barrier);
        assert(args->arg_directory);
        assert(console);
        assert(pid_socket >= 0);
        assert(uuid_socket >= 0);
        assert(notify_socket >= 0);
        assert(kmsg_socket >= 0);

        if (prctl(PR_SET_PDEATHSIG, SIGKILL) < 0)
                return log_error_errno(errno, "PR_SET_PDEATHSIG failed: %m");

        if (interactive) {
                close_nointr(STDIN_FILENO);
                close_nointr(STDOUT_FILENO);
                close_nointr(STDERR_FILENO);

                r = open_terminal(console, O_RDWR);
                if (r != STDIN_FILENO) {
                        if (r >= 0) {
                                safe_close(r);
                                r = -EINVAL;
                        }

                        return log_error_errno(r, "Failed to open console: %m");
                }

                if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO ||
                    dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
                        return log_error_errno(errno, "Failed to duplicate console: %m");
        }

        r = reset_audit_loginuid();
        if (r < 0)
                return r;

        /* Mark everything as slave, so that we still
         * receive mounts from the real root, but don't
         * propagate mounts to the real root. */
        r = mount_verbose(LOG_ERR, NULL, "/", NULL, MS_SLAVE|MS_REC, NULL);
        if (r < 0)
                return r;

        r = mount_devices(args->arg_directory,
                          root_device, root_device_rw,
                          home_device, home_device_rw,
                          srv_device, srv_device_rw,
                          esp_device);
        if (r < 0)
                return r;

        r = determine_uid_shift(args->arg_directory);
        if (r < 0)
                return r;

        r = negotiate_uid_outer_child(uid_shift_socket);
        if (r < 0)
                return r;

        /* Turn directory into bind mount */
        r = mount_verbose(LOG_ERR, args->arg_directory, args->arg_directory, NULL, MS_BIND|MS_REC, NULL);
        if (r < 0)
                return r;

        /* Mark everything as shared so our mounts get propagated down. This is
         * required to make new bind mounts available in systemd services
         * inside the containter that create a new mount namespace.
         * See https://github.com/systemd/systemd/issues/3860
         * Further submounts (such as /dev) done after this will inherit the
         * shared propagation mode.*/
        r = mount_verbose(LOG_ERR, NULL, args->arg_directory, NULL, MS_SHARED|MS_REC, NULL);
        if (r < 0)
                return r;

        r = recursive_chown(args->arg_directory, args->arg_uid_shift, args->arg_uid_range);
        if (r < 0)
                return r;

        r = setup_volatile(
                        args->arg_directory,
                        args->arg_volatile_mode,
                        args->arg_userns_mode != USER_NAMESPACE_NO,
                        args->arg_uid_shift,
                        args->arg_uid_range,
                        args->arg_selinux_context);
        if (r < 0)
                return r;

        r = base_filesystem_create(args->arg_directory, args->arg_uid_shift, (gid_t) args->arg_uid_shift);
        if (r < 0)
                return r;

        if (args->arg_read_only) {
                r = bind_remount_recursive(args->arg_directory, true, NULL);
                if (r < 0)
                        return log_error_errno(r, "Failed to make tree read-only: %m");
        }

        r = mount_all(args->arg_directory,
                      args->arg_userns_mode != USER_NAMESPACE_NO,
                      false,
                      args->arg_private_network,
                      args->arg_uid_shift,
                      args->arg_uid_range,
                      args->arg_selinux_apifs_context);
        if (r < 0)
                return r;

        r = copy_devnodes(args->arg_directory);
        if (r < 0)
                return r;

        dev_setup(args->arg_directory, args->arg_uid_shift, args->arg_uid_shift);

        r = setup_pts(args->arg_directory);
        if (r < 0)
                return r;

        r = setup_propagate(args->arg_directory);
        if (r < 0)
                return r;

        r = setup_dev_console(args->arg_directory, console);
        if (r < 0)
                return r;

        r = setup_seccomp(args->arg_caps_retain);
        if (r < 0)
                return r;

        r = setup_timezone(args->arg_directory);
        if (r < 0)
                return r;

        r = setup_resolv_conf(args->arg_directory);
        if (r < 0)
                return r;

        r = setup_machine_id(args->arg_directory);
        if (r < 0)
                return r;

        r = setup_journal(args->arg_directory);
        if (r < 0)
                return r;

        r = mount_custom(
                        args->arg_directory,
                        args->arg_custom_mounts,
                        args->arg_n_custom_mounts,
                        args->arg_userns_mode != USER_NAMESPACE_NO,
                        args->arg_uid_shift,
                        args->arg_uid_range,
                        args->arg_selinux_apifs_context);
        if (r < 0)
                return r;

        if (!args->arg_use_cgns) {
                r = mount_cgroups(
                                args->arg_directory,
                                args->arg_unified_cgroup_hierarchy,
                                args->arg_userns_mode != USER_NAMESPACE_NO,
                                args->arg_uid_shift,
                                args->arg_uid_range,
                                args->arg_selinux_apifs_context,
                                false);
                if (r < 0)
                        return r;
        }

        r = mount_move_root(args->arg_directory);
        if (r < 0)
                return log_error_errno(r, "Failed to move root directory: %m");

        fd = setup_sd_notify_child();
        if (fd < 0)
                return fd;

        pid = raw_clone(SIGCHLD|CLONE_NEWNS|
                        args->arg_clone_ns_flags |
                        (args->arg_private_network ? CLONE_NEWNET : 0) |
                        (args->arg_userns_mode != USER_NAMESPACE_NO ? CLONE_NEWUSER : 0));
        if (pid < 0)
                return log_error_errno(errno, "Failed to fork inner child: %m");
        if (pid == 0) {
                pid_socket = safe_close(pid_socket);
                uuid_socket = safe_close(uuid_socket);
                notify_socket = safe_close(notify_socket);
                uid_shift_socket = safe_close(uid_shift_socket);

                /* The inner child has all namespaces that are
                 * requested, so that we all are owned by the user if
                 * user namespaces are turned on. */

                r = inner_child(barrier, args->arg_directory, secondary, kmsg_socket, rtnl_socket, fds);
                if (r < 0)
                        _exit(EXIT_FAILURE);

                _exit(EXIT_SUCCESS);
        }

        l = send(pid_socket, &pid, sizeof(pid), MSG_NOSIGNAL);
        if (l < 0)
                return log_error_errno(errno, "Failed to send PID: %m");
        if (l != sizeof(pid)) {
                log_error("Short write while sending PID.");
                return -EIO;
        }

        r = send_uuid_outer_child(uuid_socket);
        if (r < 0)
                return r;

        l = send_one_fd(notify_socket, fd, 0);
        if (l < 0)
                return log_error_errno(errno, "Failed to send notify fd: %m");

        pid_socket = safe_close(pid_socket);
        uuid_socket = safe_close(uuid_socket);
        notify_socket = safe_close(notify_socket);
        kmsg_socket = safe_close(kmsg_socket);
        rtnl_socket = safe_close(rtnl_socket);

        return 0;
}

static int setup_uid_map(pid_t pid) {
        char uid_map[strlen("/proc//uid_map") + DECIMAL_STR_MAX(uid_t) + 1], line[DECIMAL_STR_MAX(uid_t)*3+3+1];
        int r;

        assert(pid > 1);

        xsprintf(uid_map, "/proc/" PID_FMT "/uid_map", pid);
        xsprintf(line, UID_FMT " " UID_FMT " " UID_FMT "\n", 0, args->arg_uid_shift, args->arg_uid_range);
        r = write_string_file(uid_map, line, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to write UID map: %m");

        /* We always assign the same UID and GID ranges */
        xsprintf(uid_map, "/proc/" PID_FMT "/gid_map", pid);
        r = write_string_file(uid_map, line, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to write GID map: %m");

        return 0;
}

static int nspawn_dispatch_notify_fd(sd_event_source *source, int fd, uint32_t revents, void *userdata) {
        char buf[NOTIFY_BUFFER_MAX+1];
        char *p = NULL;
        struct iovec iovec = {
                .iov_base = buf,
                .iov_len = sizeof(buf)-1,
        };
        union {
                struct cmsghdr cmsghdr;
                uint8_t buf[CMSG_SPACE(sizeof(struct ucred)) +
                            CMSG_SPACE(sizeof(int) * NOTIFY_FD_MAX)];
        } control = {};
        struct msghdr msghdr = {
                .msg_iov = &iovec,
                .msg_iovlen = 1,
                .msg_control = &control,
                .msg_controllen = sizeof(control),
        };
        struct cmsghdr *cmsg;
        struct ucred *ucred = NULL;
        ssize_t n;
        pid_t inner_child_pid;
        _cleanup_strv_free_ char **tags = NULL;

        assert(userdata);

        inner_child_pid = PTR_TO_PID(userdata);

        if (revents != EPOLLIN) {
                log_warning("Got unexpected poll event for notify fd.");
                return 0;
        }

        n = recvmsg(fd, &msghdr, MSG_DONTWAIT|MSG_CMSG_CLOEXEC);
        if (n < 0) {
                if (errno == EAGAIN || errno == EINTR)
                        return 0;

                return log_warning_errno(errno, "Couldn't read notification socket: %m");
        }
        cmsg_close_all(&msghdr);

        CMSG_FOREACH(cmsg, &msghdr) {
                if (cmsg->cmsg_level == SOL_SOCKET &&
                           cmsg->cmsg_type == SCM_CREDENTIALS &&
                           cmsg->cmsg_len == CMSG_LEN(sizeof(struct ucred))) {

                        ucred = (struct ucred*) CMSG_DATA(cmsg);
                }
        }

        if (!ucred || ucred->pid != inner_child_pid) {
                log_warning("Received notify message without valid credentials. Ignoring.");
                return 0;
        }

        if ((size_t) n >= sizeof(buf)) {
                log_warning("Received notify message exceeded maximum size. Ignoring.");
                return 0;
        }

        buf[n] = 0;
        tags = strv_split(buf, "\n\r");
        if (!tags)
                return log_oom();

        if (strv_find(tags, "READY=1"))
                sd_notifyf(false, "READY=1\n");

        p = strv_find_startswith(tags, "STATUS=");
        if (p)
                sd_notifyf(false, "STATUS=Container running: %s", p);

        return 0;
}

static int setup_sd_notify_parent(sd_event *event, int fd, pid_t *inner_child_pid) {
        int r;
        sd_event_source *notify_event_source;

        r = sd_event_add_io(event, &notify_event_source, fd, EPOLLIN, nspawn_dispatch_notify_fd, inner_child_pid);
        if (r < 0)
                return log_error_errno(r, "Failed to allocate notify event source: %m");

        (void) sd_event_source_set_description(notify_event_source, "nspawn-notify");

        return 0;
}

static int run(int master,
               const char* console,
               const char *root_device, bool root_device_rw,
               const char *home_device, bool home_device_rw,
               const char *srv_device, bool srv_device_rw,
               const char *esp_device,
               bool interactive,
               bool secondary,
               FDSet *fds,
               char veth_name[IFNAMSIZ], bool *veth_created,
               union in_addr_union *exposed,
               pid_t *pid, int *ret) {

        static const struct sigaction sa = {
                .sa_handler = nop_signal_handler,
                .sa_flags = SA_NOCLDSTOP,
        };

        _cleanup_release_lock_file_ LockFile uid_shift_lock = LOCK_FILE_INIT;
        _cleanup_close_ int etc_passwd_lock = -1;
        _cleanup_close_pair_ int
                kmsg_socket_pair[2] = { -1, -1 },
                rtnl_socket_pair[2] = { -1, -1 },
                pid_socket_pair[2] = { -1, -1 },
                uuid_socket_pair[2] = { -1, -1 },
                notify_socket_pair[2] = { -1, -1 },
                uid_shift_socket_pair[2] = { -1, -1 };
        _cleanup_close_ int notify_socket= -1;
        _cleanup_(barrier_destroy) Barrier barrier = BARRIER_NULL;
        _cleanup_(sd_event_unrefp) sd_event *event = NULL;
        _cleanup_(pty_forward_freep) PTYForward *forward = NULL;
        _cleanup_(sd_netlink_unrefp) sd_netlink *rtnl = NULL;
        ContainerStatus container_status = 0;
        char last_char = 0;
        int ifi = 0, r;
        ssize_t l;
        sigset_t mask_chld;

        assert_se(sigemptyset(&mask_chld) == 0);
        assert_se(sigaddset(&mask_chld, SIGCHLD) == 0);

        if (args->arg_userns_mode == USER_NAMESPACE_PICK) {
                /* When we shall pick the UID/GID range, let's first lock /etc/passwd, so that we can safely
                 * check with getpwuid() if the specific user already exists. Note that /etc might be
                 * read-only, in which case this will fail with EROFS. But that's really OK, as in that case we
                 * can be reasonably sure that no users are going to be added. Note that getpwuid() checks are
                 * really just an extra safety net. We kinda assume that the UID range we allocate from is
                 * really ours. */

                etc_passwd_lock = take_etc_passwd_lock(NULL);
                if (etc_passwd_lock < 0 && etc_passwd_lock != -EROFS)
                        return log_error_errno(etc_passwd_lock, "Failed to take /etc/passwd lock: %m");
        }

        r = barrier_create(&barrier);
        if (r < 0)
                return log_error_errno(r, "Cannot initialize IPC barrier: %m");

        if (socketpair(AF_UNIX, SOCK_SEQPACKET|SOCK_CLOEXEC, 0, kmsg_socket_pair) < 0)
                return log_error_errno(errno, "Failed to create kmsg socket pair: %m");

        if (socketpair(AF_UNIX, SOCK_SEQPACKET|SOCK_CLOEXEC, 0, rtnl_socket_pair) < 0)
                return log_error_errno(errno, "Failed to create rtnl socket pair: %m");

        if (socketpair(AF_UNIX, SOCK_SEQPACKET|SOCK_CLOEXEC, 0, pid_socket_pair) < 0)
                return log_error_errno(errno, "Failed to create pid socket pair: %m");

        if (socketpair(AF_UNIX, SOCK_SEQPACKET|SOCK_CLOEXEC, 0, uuid_socket_pair) < 0)
                return log_error_errno(errno, "Failed to create id socket pair: %m");

        if (socketpair(AF_UNIX, SOCK_SEQPACKET|SOCK_CLOEXEC, 0, notify_socket_pair) < 0)
                return log_error_errno(errno, "Failed to create notify socket pair: %m");

        if (args->arg_userns_mode != USER_NAMESPACE_NO)
                if (socketpair(AF_UNIX, SOCK_SEQPACKET|SOCK_CLOEXEC, 0, uid_shift_socket_pair) < 0)
                        return log_error_errno(errno, "Failed to create uid shift socket pair: %m");

        /* Child can be killed before execv(), so handle SIGCHLD in order to interrupt
         * parent's blocking calls and give it a chance to call wait() and terminate. */
        r = sigprocmask(SIG_UNBLOCK, &mask_chld, NULL);
        if (r < 0)
                return log_error_errno(errno, "Failed to change the signal mask: %m");

        r = sigaction(SIGCHLD, &sa, NULL);
        if (r < 0)
                return log_error_errno(errno, "Failed to install SIGCHLD handler: %m");

        *pid = raw_clone(SIGCHLD|CLONE_NEWNS);
        if (*pid < 0)
                return log_error_errno(errno, "clone() failed%s: %m",
                                       errno == EINVAL ?
                                       ", do you have namespace support enabled in your kernel? (You need UTS, IPC, PID and NET namespacing built in)" : "");

        if (*pid == 0) {
                /* The outer child only has a file system namespace. */
                barrier_set_role(&barrier, BARRIER_CHILD);

                master = safe_close(master);

                kmsg_socket_pair[0] = safe_close(kmsg_socket_pair[0]);
                rtnl_socket_pair[0] = safe_close(rtnl_socket_pair[0]);
                pid_socket_pair[0] = safe_close(pid_socket_pair[0]);
                uuid_socket_pair[0] = safe_close(uuid_socket_pair[0]);
                notify_socket_pair[0] = safe_close(notify_socket_pair[0]);
                uid_shift_socket_pair[0] = safe_close(uid_shift_socket_pair[0]);

                (void) reset_all_signal_handlers();
                (void) reset_signal_mask();

                r = outer_child(&barrier,
                                console,
                                root_device, root_device_rw,
                                home_device, home_device_rw,
                                srv_device, srv_device_rw,
                                esp_device,
                                interactive,
                                secondary,
                                pid_socket_pair[1],
                                uuid_socket_pair[1],
                                notify_socket_pair[1],
                                kmsg_socket_pair[1],
                                rtnl_socket_pair[1],
                                uid_shift_socket_pair[1],
                                fds);
                if (r < 0)
                        _exit(EXIT_FAILURE);

                _exit(EXIT_SUCCESS);
        }

        barrier_set_role(&barrier, BARRIER_PARENT);

        fds = fdset_free(fds);

        kmsg_socket_pair[1] = safe_close(kmsg_socket_pair[1]);
        rtnl_socket_pair[1] = safe_close(rtnl_socket_pair[1]);
        pid_socket_pair[1] = safe_close(pid_socket_pair[1]);
        uuid_socket_pair[1] = safe_close(uuid_socket_pair[1]);
        notify_socket_pair[1] = safe_close(notify_socket_pair[1]);
        uid_shift_socket_pair[1] = safe_close(uid_shift_socket_pair[1]);

        r = negotiate_uid_parent(uid_shift_socket_pair[0], &uid_shift_lock);
        if (r < 0)
                return r;

        /* Wait for the outer child. */
        r = wait_for_terminate_and_warn("namespace helper", *pid, NULL);
        if (r != 0)
                return r < 0 ? r : -EIO;

        /* And now retrieve the PID of the inner child. */
        l = recv(pid_socket_pair[0], pid, sizeof *pid, 0);
        if (l < 0)
                return log_error_errno(errno, "Failed to read inner child PID: %m");
        if (l != sizeof *pid) {
                log_error("Short read while reading inner child PID.");
                return -EIO;
        }

        /* We also retrieve container UUID in case it was generated by outer child */
        r = recv_uuid_parent(uuid_socket_pair[0]);
        if (r < 0)
                return r;

        /* We also retrieve the socket used for notifications generated by outer child */
        notify_socket = receive_one_fd(notify_socket_pair[0], 0);
        if (notify_socket < 0)
                return log_error_errno(notify_socket,
                                       "Failed to receive notification socket from the outer child: %m");

        log_debug("Init process invoked as PID "PID_FMT, *pid);

        if (args->arg_userns_mode != USER_NAMESPACE_NO) {
                if (!barrier_place_and_sync(&barrier)) { /* #1 */
                        log_error("Child died too early.");
                        return -ESRCH;
                }

                r = setup_uid_map(*pid);
                if (r < 0)
                        return r;

                (void) barrier_place(&barrier); /* #2 */
        }

        if (args->arg_private_network) {

                r = move_network_interfaces(*pid, args->arg_network_interfaces);
                if (r < 0)
                        return r;

                if (args->arg_network_veth) {
                        r = setup_veth(args->arg_machine, *pid, veth_name,
                                       args->arg_network_bridge || args->arg_network_zone);
                        if (r < 0)
                                return r;
                        else if (r > 0)
                                ifi = r;

                        if (args->arg_network_bridge) {
                                /* Add the interface to a bridge */
                                r = setup_bridge(veth_name, args->arg_network_bridge, false);
                                if (r < 0)
                                        return r;
                                if (r > 0)
                                        ifi = r;
                        } else if (args->arg_network_zone) {
                                /* Add the interface to a bridge, possibly creating it */
                                r = setup_bridge(veth_name, args->arg_network_zone, true);
                                if (r < 0)
                                        return r;
                                if (r > 0)
                                        ifi = r;
                        }
                }

                r = setup_veth_extra(args->arg_machine, *pid, args->arg_network_veth_extra);
                if (r < 0)
                        return r;

                /* We created the primary and extra veth links now; let's remember this, so that we know to
                   remove them later on. Note that we don't bother with removing veth links that were created
                   here when their setup failed half-way, because in that case the kernel should be able to
                   remove them on its own, since they cannot be referenced by anything yet. */
                *veth_created = true;

                r = setup_macvlan(args->arg_machine, *pid, args->arg_network_macvlan);
                if (r < 0)
                        return r;

                r = setup_ipvlan(args->arg_machine, *pid, args->arg_network_ipvlan);
                if (r < 0)
                        return r;
        }

        if (args->arg_register) {
                r = register_machine(
                                args->arg_machine,
                                *pid,
                                args->arg_directory,
                                args->arg_uuid,
                                ifi,
                                args->arg_slice,
                                args->arg_custom_mounts, args->arg_n_custom_mounts,
                                args->arg_kill_signal,
                                args->arg_property,
                                args->arg_keep_unit,
                                args->arg_container_service_name);
                if (r < 0)
                        return r;
        }

        r = setup_cgroup(*pid,
                         args->arg_uid_shift,
                         args->arg_cgroup_ver,
                         args->arg_keep_unit);
        if (r < 0)
                return r;

        /* Notify the child that the parent is ready with all
         * its setup (including cgroup-ification), and that
         * the child can now hand over control to the code to
         * run inside the container. */
        (void) barrier_place(&barrier); /* #3 */

        /* Block SIGCHLD here, before notifying child.
         * process_pty() will handle it with the other signals. */
        assert_se(sigprocmask(SIG_BLOCK, &mask_chld, NULL) >= 0);

        /* Reset signal to default */
        r = default_signals(SIGCHLD, -1);
        if (r < 0)
                return log_error_errno(r, "Failed to reset SIGCHLD: %m");

        r = sd_event_new(&event);
        if (r < 0)
                return log_error_errno(r, "Failed to get default event source: %m");

        r = setup_sd_notify_parent(event, notify_socket, PID_TO_PTR(*pid));
        if (r < 0)
                return r;

        /* Let the child know that we are ready and wait that the child is completely ready now. */
        if (!barrier_place_and_sync(&barrier)) { /* #4 */
                log_error("Child died too early.");
                return -ESRCH;
        }

        /* At this point we have made use of the UID we picked, and thus nss-mymachines
         * will make them appear in getpwuid(), thus we can release the /etc/passwd lock. */
        etc_passwd_lock = safe_close(etc_passwd_lock);

        sd_notifyf(false,
                   "STATUS=Container running.\n"
                   "X_NSPAWN_LEADER_PID=" PID_FMT, *pid);
        if (!args->arg_notify_ready)
                sd_notify(false, "READY=1\n");

        if (args->arg_kill_signal > 0) {
                /* Try to kill the init system on SIGINT or SIGTERM */
                sd_event_add_signal(event, NULL, SIGINT, on_orderly_shutdown, PID_TO_PTR(*pid));
                sd_event_add_signal(event, NULL, SIGTERM, on_orderly_shutdown, PID_TO_PTR(*pid));
        } else {
                /* Immediately exit */
                sd_event_add_signal(event, NULL, SIGINT, NULL, NULL);
                sd_event_add_signal(event, NULL, SIGTERM, NULL, NULL);
        }

        /* simply exit on sigchld */
        sd_event_add_signal(event, NULL, SIGCHLD, NULL, NULL);

        if (args->arg_expose_ports) {
                r = expose_port_watch_rtnl(event, rtnl_socket_pair[0], on_address_change, exposed, &rtnl);
                if (r < 0)
                        return r;

                (void) expose_port_execute(rtnl, args->arg_expose_ports, exposed);
        }

        rtnl_socket_pair[0] = safe_close(rtnl_socket_pair[0]);

        r = pty_forward_new(event, master,
                            PTY_FORWARD_IGNORE_VHANGUP | (interactive ? 0 : PTY_FORWARD_READ_ONLY),
                            &forward);
        if (r < 0)
                return log_error_errno(r, "Failed to create PTY forwarder: %m");

        r = sd_event_loop(event);
        if (r < 0)
                return log_error_errno(r, "Failed to run event loop: %m");

        pty_forward_get_last_char(forward, &last_char);

        forward = pty_forward_free(forward);

        if (!args->arg_quiet && last_char != '\n')
                putc('\n', stdout);

        /* Kill if it is not dead yet anyway */
        if (args->arg_register && !args->arg_keep_unit)
                terminate_machine(*pid);

        /* Normally redundant, but better safe than sorry */
        kill(*pid, SIGKILL);

        r = wait_for_container(*pid, &container_status);
        *pid = 0;

        if (r < 0)
                /* We failed to wait for the container, or the container exited abnormally. */
                return r;
        if (r > 0 || container_status == CONTAINER_TERMINATED) {
                /* r > 0 → The container exited with a non-zero status.
                 *         As a special case, we need to replace 133 with a different value,
                 *         because 133 is special-cased in the service file to reboot the container.
                 * otherwise → The container exited with zero status and a reboot was not requested.
                 */
                if (r == 133)
                        r = EXIT_FAILURE; /* replace 133 with the general failure code */
                *ret = r;
                return 0; /* finito */
        }

        /* CONTAINER_REBOOTED, loop again */

        if (args->arg_keep_unit) {
                /* Special handling if we are running as a service: instead of simply
                 * restarting the machine we want to restart the entire service, so let's
                 * inform systemd about this with the special exit code 133. The service
                 * file uses RestartForceExitStatus=133 so that this results in a full
                 * nspawn restart. This is necessary since we might have cgroup parameters
                 * set we want to have flushed out. */
                *ret = 0;
                return 133;
        }

        expose_port_flush(args->arg_expose_ports, exposed);

        (void) remove_veth_links(veth_name, args->arg_network_veth_extra);
        *veth_created = false;
        return 1; /* loop again */
}

int main(int argc, char *argv[]) {

        _cleanup_free_ char *device_path = NULL, *root_device = NULL, *home_device = NULL, *srv_device = NULL, *esp_device = NULL, *console = NULL;
        bool root_device_rw = true, home_device_rw = true, srv_device_rw = true;
        _cleanup_close_ int master = -1, image_fd = -1;
        _cleanup_fdset_free_ FDSet *fds = NULL;
        int r, n_fd_passed, loop_nr = -1, ret = EXIT_SUCCESS;
        char veth_name[IFNAMSIZ] = "";
        bool secondary = false, remove_subvol = false;
        pid_t pid = 0;
        union in_addr_union exposed = {};
        _cleanup_release_lock_file_ LockFile tree_global_lock = LOCK_FILE_INIT, tree_local_lock = LOCK_FILE_INIT;
        bool interactive, veth_created = false;

        args = args_get();

        log_parse_environment();
        log_open();
        cg_unified_flush();

        /* Make sure rename_process() in the stub init process can work */
        saved_argv = argv;
        saved_argc = argc;

        r = parse_argv(argc, argv);
        if (r <= 0)
                goto finish;

        if (geteuid() != 0) {
                log_error("Need to be root.");
                r = -EPERM;
                goto finish;
        }
        r = determine_names();
        if (r < 0)
                goto finish;

        r = load_settings();
        if (r < 0)
                goto finish;

        r = verify_arguments();
        if (r < 0)
                goto finish;

        n_fd_passed = sd_listen_fds(false);
        if (n_fd_passed > 0) {
                r = fdset_new_listen_fds(&fds, false);
                if (r < 0) {
                        log_error_errno(r, "Failed to collect file descriptors: %m");
                        goto finish;
                }
        }

        if (args->arg_directory) {
                assert(!args->arg_image);

                if (path_equal(args->arg_directory, "/") && !args->arg_ephemeral) {
                        log_error("Spawning container on root directory is not supported. Consider using --ephemeral.");
                        r = -EINVAL;
                        goto finish;
                }

                if (args->arg_ephemeral) {
                        assert(!args->arg_template);

                        r = lock_tree_ephemeral(&tree_global_lock, &tree_local_lock);
                        if (r < 0)
                                goto finish;

                        remove_subvol = true;

                } else {
                        r = lock_tree_plain(&tree_global_lock, &tree_local_lock);
                        if (r < 0)
                                goto finish;

                        if (args->arg_template) {
                                r = btrfs_subvol_snapshot(args->arg_template, args->arg_directory, (args->arg_read_only ? BTRFS_SNAPSHOT_READ_ONLY : 0) | BTRFS_SNAPSHOT_FALLBACK_COPY | BTRFS_SNAPSHOT_RECURSIVE | BTRFS_SNAPSHOT_QUOTA);
                                if (r == -EEXIST) {
                                        if (!args->arg_quiet)
                                                log_info("Directory %s already exists, not populating from template %s.", args->arg_directory, args->arg_template);
                                } else if (r < 0) {
                                        log_error_errno(r, "Couldn't create snapshot %s from %s: %m", args->arg_directory, args->arg_template);
                                        goto finish;
                                } else {
                                        if (!args->arg_quiet)
                                                log_info("Populated %s from template %s.", args->arg_directory, args->arg_template);
                                }
                        }
                }

                if (args->arg_start_mode == START_BOOT) {
                        if (path_is_os_tree(args->arg_directory) <= 0) {
                                log_error("Directory %s doesn't look like an OS root directory (os-release file is missing). Refusing.", args->arg_directory);
                                r = -EINVAL;
                                goto finish;
                        }
                } else {
                        const char *p;

                        p = strjoina(args->arg_directory, "/usr/");
                        if (laccess(p, F_OK) < 0) {
                                log_error("Directory %s doesn't look like it has an OS tree. Refusing.", args->arg_directory);
                                r = -EINVAL;
                                goto finish;
                        }
                }

        } else {
                assert(args->arg_image);
                assert(!args->arg_template);
                assert(!args->arg_ephemeral);

                r = lock_tree_image(&tree_global_lock, &tree_local_lock);
                if (r < 0)
                        goto finish;

                image_fd = setup_image(&device_path, &loop_nr);
                if (image_fd < 0) {
                        r = image_fd;
                        goto finish;
                }

                r = dissect_image(image_fd,
                                  &root_device, &root_device_rw,
                                  &home_device, &home_device_rw,
                                  &srv_device, &srv_device_rw,
                                  &esp_device,
                                  &secondary);
                if (r < 0)
                        goto finish;
        }

        r = custom_mounts_prepare();
        if (r < 0)
                goto finish;

        if (args->arg_cgroup_ver == CGROUP_VER_INVALID) {
                r = pick_cgroup_ver(args->arg_directory, &args->arg_cgroup_ver);
                if (r < 0)
                        goto finish;
        }

        interactive =
                isatty(STDIN_FILENO) > 0 &&
                isatty(STDOUT_FILENO) > 0;

        master = posix_openpt(O_RDWR|O_NOCTTY|O_CLOEXEC|O_NDELAY);
        if (master < 0) {
                r = log_error_errno(errno, "Failed to acquire pseudo tty: %m");
                goto finish;
        }

        r = ptsname_malloc(master, &console);
        if (r < 0) {
                r = log_error_errno(r, "Failed to determine tty name: %m");
                goto finish;
        }

        if (args->arg_selinux_apifs_context) {
                r = mac_selinux_apply(console, args->arg_selinux_apifs_context);
                if (r < 0)
                        goto finish;
        }

        if (unlockpt(master) < 0) {
                r = log_error_errno(errno, "Failed to unlock tty: %m");
                goto finish;
        }

        if (!args->arg_quiet)
                log_info("Spawning container %s on %s.\nPress ^] three times within 1s to kill container.",
                         args->arg_machine, args->arg_image ?: args->arg_directory);

        assert_se(sigprocmask_many(SIG_BLOCK, NULL, SIGCHLD, SIGWINCH, SIGTERM, SIGINT, -1) >= 0);

        if (prctl(PR_SET_CHILD_SUBREAPER, 1) < 0) {
                r = log_error_errno(errno, "Failed to become subreaper: %m");
                goto finish;
        }

        for (;;) {
                r = run(master,
                        console,
                        root_device, root_device_rw,
                        home_device, home_device_rw,
                        srv_device, srv_device_rw,
                        esp_device,
                        interactive, secondary,
                        fds,
                        veth_name, &veth_created,
                        &exposed,
                        &pid, &ret);
                if (r <= 0)
                        break;
        }

finish:
        sd_notify(false,
                  "STOPPING=1\n"
                  "STATUS=Terminating...");

        if (pid > 0)
                kill(pid, SIGKILL);

        /* Try to flush whatever is still queued in the pty */
        if (master >= 0)
                (void) copy_bytes(master, STDOUT_FILENO, (uint64_t) -1, false);

        loop_remove(loop_nr, &image_fd);

        if (remove_subvol && args->arg_directory) {
                int k;

                k = btrfs_subvol_remove(args->arg_directory, BTRFS_REMOVE_RECURSIVE|BTRFS_REMOVE_QUOTA);
                if (k < 0)
                        log_warning_errno(k, "Cannot remove subvolume '%s', ignoring: %m", args->arg_directory);
        }

        if (args->arg_machine) {
                const char *p;

                p = strjoina("/run/systemd/nspawn/propagate/", args->arg_machine);
                (void) rm_rf(p, REMOVE_ROOT);
        }

        expose_port_flush(args->arg_expose_ports, &exposed);

        if (veth_created)
                (void) remove_veth_links(veth_name, args->arg_network_veth_extra);
        (void) remove_bridge(args->arg_network_zone);

        args_free();

        return r < 0 ? EXIT_FAILURE : ret;
}
