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
#include <blkid.h>
#endif
#include <errno.h>
#include <getopt.h>
#include <grp.h>
#include <linux/loop.h>
#include <pwd.h>
#include <sched.h>
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
#include <sys/wait.h>
#include <unistd.h>

#include "sd-bus.h"
#include "sd-daemon.h"
#include "sd-id128.h"

#include "alloc-util.h"
#include "barrier.h"
#include "base-filesystem.h"
#include "blkid-util.h"
#include "btrfs-util.h"
#include "bus-util.h"
#include "cap-list.h"
#include "capability-util.h"
#include "cgroup-util.h"
#include "copy.h"
#include "dev-setup.h"
#include "dissect-image.h"
#include "env-util.h"
#include "fd-util.h"
#include "fdset.h"
#include "fileio.h"
#include "format-util.h"
#include "fs-util.h"
#include "gpt.h"
#include "hexdecoct.h"
#include "hostname-util.h"
#include "id128-util.h"
#include "log.h"
#include "loop-util.h"
#include "loopback-setup.h"
#include "machine-image.h"
#include "macro.h"
#include "missing.h"
#include "mkdir.h"
#include "mount-util.h"
#include "netlink-util.h"
#include "nspawn-cgroup.h"
#include "nspawn-expose-ports.h"
#include "nspawn-mount.h"
#include "nspawn-patch-uid.h"
#include "nspawn-register.h"
#include "nspawn-seccomp.h"
#include "nspawn-settings.h"
#include "nspawn-setuid.h"
#include "nspawn-stub-pid1.h"
#include "parse-util.h"
#include "path-util.h"
#include "process-util.h"
#include "ptyfwd.h"
#include "random-util.h"
#include "raw-clone.h"
#include "rm-rf.h"
#include "selinux-util.h"
#include "signal-util.h"
#include "socket-util.h"
#include "stat-util.h"
#include "stdio-util.h"
#include "string-util.h"
#include "strv.h"
#include "terminal-util.h"
#include "udev-util.h"
#include "umask-util.h"
#include "user-util.h"
#include "util.h"

/* Note that devpts's gid= parameter parses GIDs as signed values, hence we stay away from the upper half of the 32bit
 * UID range here. We leave a bit of room at the lower end and a lot of room at the upper end, so that other subsystems
 * may have their own allocation ranges too. */
#define UID_SHIFT_PICK_MIN ((uid_t) UINT32_C(0x00080000))
#define UID_SHIFT_PICK_MAX ((uid_t) UINT32_C(0x6FFF0000))

/* nspawn is listening on the socket at the path in the constant nspawn_notify_socket_path
 * nspawn_notify_socket_path is relative to the container
 * the init process in the container pid can send messages to nspawn following the sd_notify(3) protocol */
#define NSPAWN_NOTIFY_SOCKET_PATH "/run/systemd/nspawn/notify"

#define EXIT_FORCE_RESTART 133

typedef enum ContainerStatus {
        CONTAINER_TERMINATED,
        CONTAINER_REBOOTED
} ContainerStatus;

typedef enum LinkJournal {
        LINK_NO,
        LINK_AUTO,
        LINK_HOST,
        LINK_GUEST
} LinkJournal;

static char *arg_directory = NULL;
static char *arg_chdir = NULL;
static sd_id128_t arg_uuid = {};
static char *arg_machine = NULL;
static const char *arg_selinux_context = NULL;
static const char *arg_selinux_apifs_context = NULL;
static const char *arg_slice = NULL;
static bool arg_read_only = false;
static StartMode arg_start_mode = START_PID1;
static LinkJournal arg_link_journal = LINK_AUTO;
static bool arg_link_journal_try = false;
static uint64_t arg_caps_retain =
        (1ULL << CAP_AUDIT_CONTROL) |
        (1ULL << CAP_AUDIT_WRITE) |
        (1ULL << CAP_CHOWN) |
        (1ULL << CAP_DAC_OVERRIDE) |
        (1ULL << CAP_DAC_READ_SEARCH) |
        (1ULL << CAP_FOWNER) |
        (1ULL << CAP_FSETID) |
        (1ULL << CAP_IPC_OWNER) |
        (1ULL << CAP_KILL) |
        (1ULL << CAP_LEASE) |
        (1ULL << CAP_LINUX_IMMUTABLE) |
        (1ULL << CAP_MKNOD) |
        (1ULL << CAP_NET_BIND_SERVICE) |
        (1ULL << CAP_NET_BROADCAST) |
        (1ULL << CAP_NET_RAW) |
        (1ULL << CAP_SETFCAP) |
        (1ULL << CAP_SETGID) |
        (1ULL << CAP_SETPCAP) |
        (1ULL << CAP_SETUID) |
        (1ULL << CAP_SYS_ADMIN) |
        (1ULL << CAP_SYS_BOOT) |
        (1ULL << CAP_SYS_CHROOT) |
        (1ULL << CAP_SYS_NICE) |
        (1ULL << CAP_SYS_PTRACE) |
        (1ULL << CAP_SYS_RESOURCE) |
        (1ULL << CAP_SYS_TTY_CONFIG);
static CustomMount *arg_custom_mounts = NULL;
static unsigned arg_n_custom_mounts = 0;
static char **arg_setenv = NULL;
static bool arg_quiet = false;
static char **arg_network_interfaces = NULL;
static char **arg_network_macvlan = NULL;
static char **arg_network_ipvlan = NULL;
static char *arg_network_bridge = NULL;
static char *arg_network_zone = NULL;
static unsigned long arg_personality = PERSONALITY_INVALID;
static VolatileMode arg_volatile_mode = VOLATILE_NO;
static char **arg_property = NULL;
static uid_t arg_uid_shift = UID_INVALID, arg_uid_range = 0x10000U;
static int arg_kill_signal = 0;
static CGroupUnified arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_UNKNOWN;
static SettingsMask arg_settings_mask = 0;
static char **arg_parameters = NULL;
static const char *arg_container_service_name = "systemd-nspawn";
static bool arg_notify_ready = false;
static bool arg_use_cgns = true;
static unsigned long arg_clone_ns_flags = CLONE_NEWIPC|CLONE_NEWPID|CLONE_NEWUTS;
static MountSettingsMask arg_mount_settings = MOUNT_APPLY_APIVFS_RO;
static void *arg_root_hash = NULL;

static int detect_unified_cgroup_hierarchy(const char *directory) {
        const char *e;
        int r;

        /* Allow the user to control whether the unified hierarchy is used */
        e = getenv("UNIFIED_CGROUP_HIERARCHY");
        if (e) {
                r = parse_boolean(e);
                if (r < 0)
                        return log_error_errno(r, "Failed to parse $UNIFIED_CGROUP_HIERARCHY.");
                if (r > 0)
                        arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_ALL;
                else
                        arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_NONE;

                return 0;
        }

        /* Otherwise inherit the default from the host system */
        r = cg_all_unified();
        if (r < 0)
                return log_error_errno(r, "Failed to determine whether we are in all unified mode.");
        if (r > 0) {
                /* Unified cgroup hierarchy support was added in 230. Unfortunately the detection
                 * routine only detects 231, so we'll have a false negative here for 230. */
                r = systemd_installation_has_version(directory, 230);
                if (r < 0)
                        return log_error_errno(r, "Failed to determine systemd version in container: %m");
                if (r > 0)
                        arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_ALL;
                else
                        arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_NONE;
        } else if (cg_unified_controller(SYSTEMD_CGROUP_CONTROLLER) > 0) {
                /* Mixed cgroup hierarchy support was added in 233 */
                r = systemd_installation_has_version(directory, 233);
                if (r < 0)
                        return log_error_errno(r, "Failed to determine systemd version in container: %m");
                if (r > 0)
                        arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_SYSTEMD;
                else
                        arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_NONE;
        } else
                arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_NONE;

        return 0;
}

static void parse_share_ns_env(const char *name, unsigned long ns_flag) {
        int r;

        r = getenv_bool(name);
        if (r == -ENXIO)
                return;
        if (r < 0)
                log_warning_errno(r, "Failed to parse %s from environment, defaulting to false.", name);
        arg_clone_ns_flags = (arg_clone_ns_flags & ~ns_flag) | (r > 0 ? 0 : ns_flag);
}

static void parse_mount_settings_env(void) {
        int r;
        const char *e;

        e = getenv("SYSTEMD_NSPAWN_API_VFS_WRITABLE");
        if (!e)
                return;

        if (streq(e, "network")) {
                arg_mount_settings |= MOUNT_APPLY_APIVFS_RO|MOUNT_APPLY_APIVFS_NETNS;
                return;
        }

        r = parse_boolean(e);
        if (r < 0) {
                log_warning_errno(r, "Failed to parse SYSTEMD_NSPAWN_API_VFS_WRITABLE from environment, ignoring.");
                return;
        }

        SET_FLAG(arg_mount_settings, MOUNT_APPLY_APIVFS_RO, r == 0);
        SET_FLAG(arg_mount_settings, MOUNT_APPLY_APIVFS_NETNS, false);
}

static int parse_argv(int argc, char *argv[]) {

        static const struct option options[] = {
                { "directory",             required_argument, NULL, 'D'                     },
                {}
        };

        int c, r;
        const char *e;
        uint64_t plus = 0, minus = 0;
        bool mask_all_settings = false, mask_no_settings = false;

        assert(argc >= 0);
        assert(argv);

        while ((c = getopt_long(argc, argv, "+hD:u:abL:M:jS:Z:qi:xp:nUE:", options, NULL)) >= 0)

                switch (c) {

                case 'D':
                        r = parse_path_argument_and_warn(optarg, false, &arg_directory);
                        if (r < 0)
                                return r;
                        break;

                case '?':
                        return -EINVAL;

                default:
                        assert_not_reached("Unhandled option");
                }

        parse_share_ns_env("SYSTEMD_NSPAWN_SHARE_NS_IPC", CLONE_NEWIPC);
        parse_share_ns_env("SYSTEMD_NSPAWN_SHARE_NS_PID", CLONE_NEWPID);
        parse_share_ns_env("SYSTEMD_NSPAWN_SHARE_NS_UTS", CLONE_NEWUTS);
        parse_share_ns_env("SYSTEMD_NSPAWN_SHARE_SYSTEM", CLONE_NEWIPC|CLONE_NEWPID|CLONE_NEWUTS);

        parse_mount_settings_env();

        if (!(arg_clone_ns_flags & CLONE_NEWPID) ||
            !(arg_clone_ns_flags & CLONE_NEWUTS)) {
                if (arg_start_mode != START_PID1) {
                        log_error("--boot cannot be used without namespacing.");
                        return -EINVAL;
                }
        }

        if (arg_network_bridge && arg_network_zone) {
                log_error("--network-bridge= and --network-zone= may not be combined.");
                return -EINVAL;
        }

        if (argc > optind) {
                arg_parameters = strv_copy(argv + optind);
                if (!arg_parameters)
                        return log_oom();

                arg_settings_mask |= SETTING_START_MODE;
        }

        /* Load all settings from .nspawn files */
        if (mask_no_settings)
                arg_settings_mask = 0;

        /* Don't load any settings from .nspawn files */
        if (mask_all_settings)
                arg_settings_mask = _SETTINGS_MASK_ALL;

        arg_caps_retain = (arg_caps_retain | plus) & ~minus;

        r = cg_unified_flush();
        if (r < 0)
                return log_error_errno(r, "Failed to determine whether the unified cgroups hierarchy is used: %m");

        e = getenv("SYSTEMD_NSPAWN_CONTAINER_SERVICE");
        if (e)
                arg_container_service_name = e;

        r = getenv_bool("SYSTEMD_NSPAWN_USE_CGNS");
        if (r < 0)
                arg_use_cgns = cg_ns_supported();
        else
                arg_use_cgns = r;

        return 1;
}

static int verify_arguments(void) {
        if (arg_volatile_mode != VOLATILE_NO && arg_read_only) {
                log_error("Cannot combine --read-only with --volatile. Note that --volatile already implies a read-only base hierarchy.");
                return -EINVAL;
        }

        if (arg_start_mode == START_BOOT && arg_kill_signal <= 0)
                arg_kill_signal = SIGRTMIN+3;

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

        return 0;
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

        if (unlink(where) < 0 && errno != ENOENT) {
                log_full_errno(IN_SET(errno, EROFS, EACCES, EPERM) ? LOG_DEBUG : LOG_WARNING, /* Don't complain on read-only images */
                               errno,
                               "Failed to remove existing timezone info %s in container, ignoring: %m", where);
                return 0;
        }

        what = strjoina("../usr/share/zoneinfo/", z);
        if (symlink(what, where) < 0) {
                log_full_errno(IN_SET(errno, EROFS, EACCES, EPERM) ? LOG_DEBUG : LOG_WARNING,
                               errno,
                               "Failed to correct timezone of container, ignoring: %m");
                return 0;
        }

        return 0;
}

static int resolved_listening(void) {
        _cleanup_(sd_bus_flush_close_unrefp) sd_bus *bus = NULL;
        _cleanup_free_ char *dns_stub_listener_mode = NULL;
        int r;

        /* Check if resolved is listening */

        r = sd_bus_open_system(&bus);
        if (r < 0)
                return r;

        r = bus_name_has_owner(bus, "org.freedesktop.resolve1", NULL);
        if (r <= 0)
                return r;

        r = sd_bus_get_property_string(bus,
                                       "org.freedesktop.resolve1",
                                       "/org/freedesktop/resolve1",
                                       "org.freedesktop.resolve1.Manager",
                                       "DNSStubListener",
                                       NULL,
                                       &dns_stub_listener_mode);
        if (r < 0)
                return r;

        return STR_IN_SET(dns_stub_listener_mode, "udp", "yes");
}

static int setup_resolv_conf(const char *dest) {
        _cleanup_free_ char *resolved = NULL, *etc = NULL;
        const char *where;
        int r, found;

        assert(dest);

        r = chase_symlinks("/etc", dest, CHASE_PREFIX_ROOT, &etc);
        if (r < 0) {
                log_warning_errno(r, "Failed to resolve /etc path in container, ignoring: %m");
                return 0;
        }

        where = strjoina(etc, "/resolv.conf");
        found = chase_symlinks(where, dest, CHASE_NONEXISTENT, &resolved);
        if (found < 0) {
                log_warning_errno(found, "Failed to resolve /etc/resolv.conf path in container, ignoring: %m");
                return 0;
        }

        if (access("/usr/lib/systemd/resolv.conf", F_OK) >= 0 &&
            resolved_listening() > 0) {

                /* resolved is enabled on the host. In this, case bind mount its static resolv.conf file into the
                 * container, so that the container can use the host's resolver. Given that network namespacing is
                 * disabled it's only natural of the container also uses the host's resolver. It also has the big
                 * advantage that the container will be able to follow the host's DNS server configuration changes
                 * transparently. */

                if (found == 0) /* missing? */
                        (void) touch(resolved);

                r = mount_verbose(LOG_DEBUG, "/usr/lib/systemd/resolv.conf", resolved, NULL, MS_BIND, NULL);
                if (r >= 0)
                        return mount_verbose(LOG_ERR, NULL, resolved, NULL, MS_BIND|MS_REMOUNT|MS_RDONLY|MS_NOSUID|MS_NODEV, NULL);
        }

        /* If that didn't work, let's copy the file */
        r = copy_file("/etc/resolv.conf", where, O_TRUNC|O_NOFOLLOW, 0644, 0, COPY_REFLINK);
        if (r < 0) {
                /* If the file already exists as symlink, let's suppress the warning, under the assumption that
                 * resolved or something similar runs inside and the symlink points there.
                 *
                 * If the disk image is read-only, there's also no point in complaining.
                 */
                log_full_errno(IN_SET(r, -ELOOP, -EROFS, -EACCES, -EPERM) ? LOG_DEBUG : LOG_WARNING, r,
                               "Failed to copy /etc/resolv.conf to %s, ignoring: %m", where);
                return 0;
        }

        return 0;
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
                                /* Explicitly warn the user when /dev is already populated. */
                                if (errno == EEXIST)
                                        log_notice("%s/dev is pre-mounted and pre-populated. If a pre-mounted /dev is provided it needs to be an unpopulated file system.", dest);
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

                }
        }

        return r;
}

static int setup_pts(const char *dest) {
        _cleanup_free_ char *options = NULL;
        const char *p;
        int r;

#ifdef HAVE_SELINUX
        if (arg_selinux_apifs_context)
                (void) asprintf(&options,
                                "newinstance,ptmxmode=0666,mode=620,gid=" GID_FMT ",context=\"%s\"",
                                arg_uid_shift + TTY_GID,
                                arg_selinux_apifs_context);
        else
#endif
                (void) asprintf(&options,
                                "newinstance,ptmxmode=0666,mode=620,gid=" GID_FMT,
                                arg_uid_shift + TTY_GID);

        if (!options)
                return log_oom();

        /* Mount /dev/pts itself */
        p = prefix_roota(dest, "/dev/pts");
        if (mkdir(p, 0755) < 0)
                return log_error_errno(errno, "Failed to create /dev/pts: %m");
        r = mount_verbose(LOG_ERR, "devpts", p, "devpts", MS_NOSUID|MS_NOEXEC, options);
        if (r < 0)
                return r;

        /* Create /dev/ptmx symlink */
        p = prefix_roota(dest, "/dev/ptmx");
        if (symlink("pts/ptmx", p) < 0)
                return log_error_errno(errno, "Failed to create /dev/ptmx symlink: %m");

        /* And fix /dev/pts/ptmx ownership */
        p = prefix_roota(dest, "/dev/pts/ptmx");

        return 0;
}

static int setup_dev_console(const char *dest, const char *console) {
        _cleanup_umask_ mode_t u;
        const char *to;
        int r;

        assert(dest);
        assert(console);

        u = umask(0000);

        r = chmod_and_chown(console, 0600, arg_uid_shift, arg_uid_shift);
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

static int setup_hostname(void) {

        if ((arg_clone_ns_flags & CLONE_NEWUTS) == 0)
                return 0;

        if (sethostname_idempotent(arg_machine) < 0)
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

        if (arg_link_journal == LINK_NO)
                return 0;

        try = arg_link_journal_try || arg_link_journal == LINK_AUTO;

        r = sd_id128_get_machine(&this_id);
        if (r < 0)
                return log_error_errno(r, "Failed to retrieve machine ID: %m");

        if (sd_id128_equal(arg_uuid, this_id)) {
                log_full(try ? LOG_WARNING : LOG_ERR,
                         "Host and machine ids are equal (%s): refusing to link journals", sd_id128_to_string(arg_uuid, id));
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

        (void) sd_id128_to_string(arg_uuid, id);

        p = strjoina("/var/log/journal/", id);
        q = prefix_roota(directory, p);

        if (path_is_mount_point(p, NULL, 0) > 0) {
                if (try)
                        return 0;

                log_error("%s: already a mount point, refusing to use for journal", p);
                return -EEXIST;
        }

        if (path_is_mount_point(q, NULL, 0) > 0) {
                if (try)
                        return 0;

                log_error("%s: already a mount point, refusing to use for journal", q);
                return -EEXIST;
        }

        r = readlink_and_make_absolute(p, &d);
        if (r >= 0) {
                if ((arg_link_journal == LINK_GUEST ||
                     arg_link_journal == LINK_AUTO) &&
                    path_equal(d, q)) {

                        r = userns_mkdir(directory, p, 0755, 0, 0);
                        if (r < 0)
                                log_warning_errno(r, "Failed to create directory %s: %m", q);
                        return 0;
                }

                if (unlink(p) < 0)
                        return log_error_errno(errno, "Failed to remove symlink %s: %m", p);
        } else if (r == -EINVAL) {

                if (arg_link_journal == LINK_GUEST &&
                    rmdir(p) < 0) {

                        if (errno == ENOTDIR) {
                                log_error("%s already exists and is neither a symlink nor a directory", p);
                                return r;
                        } else
                                return log_error_errno(errno, "Failed to remove %s: %m", p);
                }
        } else if (r != -ENOENT)
                return log_error_errno(r, "readlink(%s) failed: %m", p);

        if (arg_link_journal == LINK_GUEST) {

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

        if (arg_link_journal == LINK_HOST) {
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
        return capability_bounding_set_drop(arg_caps_retain, false);
}

static int reset_audit_loginuid(void) {
        _cleanup_free_ char *p = NULL;
        int r;

        if ((arg_clone_ns_flags & CLONE_NEWPID) == 0)
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
        p = strjoina("/run/systemd/nspawn/propagate/", arg_machine);
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

static int setup_machine_id(const char *directory) {
        const char *etc_machine_id;
        sd_id128_t id;
        int r;

        /* If the UUID in the container is already set, then that's what counts, and we use. If it isn't set, and the
         * caller passed --uuid=, then we'll pass it in the $container_uuid env var to PID 1 of the container. The
         * assumption is that PID 1 will then write it to /etc/machine-id to make it persistent. If --uuid= is not
         * passed we generate a random UUID, and pass it via $container_uuid. In effect this means that /etc/machine-id
         * in the container and our idea of the container UUID will always be in sync (at least if PID 1 in the
         * container behaves nicely). */

        etc_machine_id = prefix_roota(directory, "/etc/machine-id");

        r = id128_read(etc_machine_id, ID128_PLAIN, &id);
        if (r < 0) {
                if (!IN_SET(r, -ENOENT, -ENOMEDIUM)) /* If the file is missing or empty, we don't mind */
                        return log_error_errno(r, "Failed to read machine ID from container image: %m");

                if (sd_id128_is_null(arg_uuid)) {
                        r = sd_id128_randomize(&arg_uuid);
                        if (r < 0)
                                return log_error_errno(r, "Failed to acquire randomized machine UUID: %m");
                }
        } else {
                if (sd_id128_is_null(id)) {
                        log_error("Machine ID in container image is zero, refusing.");
                        return -EINVAL;
                }

                arg_uuid = id;
        }

        return 0;
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
                        log_full(arg_quiet ? LOG_DEBUG : LOG_INFO, "Container %s exited successfully.", arg_machine);
                else
                        log_full(arg_quiet ? LOG_DEBUG : LOG_INFO, "Container %s failed with error code %i.", arg_machine, status.si_status);

                *container = CONTAINER_TERMINATED;
                return status.si_status;

        case CLD_KILLED:
                if (status.si_status == SIGINT) {
                        log_full(arg_quiet ? LOG_DEBUG : LOG_INFO, "Container %s has been shut down.", arg_machine);
                        *container = CONTAINER_TERMINATED;
                        return 0;

                } else if (status.si_status == SIGHUP) {
                        log_full(arg_quiet ? LOG_DEBUG : LOG_INFO, "Container %s is being rebooted.", arg_machine);
                        *container = CONTAINER_REBOOTED;
                        return 0;
                }

                /* fall through */

        case CLD_DUMPED:
                log_error("Container %s terminated by signal %s.", arg_machine, signal_to_string(status.si_status));
                return -EIO;

        default:
                log_error("Container %s failed due to unknown reason.", arg_machine);
                return -EIO;
        }
}

static int on_orderly_shutdown(sd_event_source *s, const struct signalfd_siginfo *si, void *userdata) {
        pid_t pid;

        pid = PTR_TO_PID(userdata);
        if (pid > 0) {
                if (kill(pid, arg_kill_signal) >= 0) {
                        log_info("Trying to halt container. Send SIGTERM again to trigger immediate termination.");
                        sd_event_source_set_userdata(s, NULL);
                        return 0;
                }
        }

        sd_event_exit(sd_event_source_get_event(s), 0);
        return 0;
}

static int on_sigchld(sd_event_source *s, const struct signalfd_siginfo *ssi, void *userdata) {
        for (;;) {
                siginfo_t si = {};
                if (waitid(P_ALL, 0, &si, WNOHANG|WNOWAIT|WEXITED) < 0)
                        return log_error_errno(errno, "Failed to waitid(): %m");
                if (si.si_pid == 0) /* No pending children. */
                        break;
                if (si.si_pid == PTR_TO_PID(userdata)) {
                        /* The main process we care for has exited. Return from
                         * signal handler but leave the zombie. */
                        sd_event_exit(sd_event_source_get_event(s), 0);
                        break;
                }
                /* Reap all other children. */
                (void) waitid(P_PID, si.si_pid, &si, WNOHANG|WEXITED);
        }

        return 0;
}

static int determine_names(void) {

        assert(arg_directory);

        if (!arg_machine) {

                if (arg_directory && path_equal(arg_directory, "/"))
                        arg_machine = gethostname_malloc();
                else {
                        arg_machine = strdup(basename(arg_directory));
                }
                if (!arg_machine)
                        return log_oom();

                hostname_cleanup(arg_machine);
                if (!machine_name_is_valid(arg_machine)) {
                        log_error("Failed to determine machine name automatically, please use -M.");
                        return -EINVAL;
                }
        }

        return 0;
}

static int chase_symlinks_and_update(char **p, unsigned flags) {
        char *chased;
        int r;

        assert(p);

        if (!*p)
                return 0;

        r = chase_symlinks(*p, NULL, flags, &chased);
        if (r < 0)
                return log_error_errno(r, "Failed to resolve path %s: %m", *p);

        free(*p);
        *p = chased;

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
        const char *exec_target;

        _cleanup_strv_free_ char **env_use = NULL;
        int r;

        assert(barrier);
        assert(directory);
        assert(kmsg_socket >= 0);

        r = reset_uid_gid();
        if (r < 0)
                return log_error_errno(r, "Couldn't become new root: %m");

        r = mount_all(NULL,
                      arg_mount_settings | MOUNT_IN_USERNS,
                      arg_uid_shift,
                      arg_uid_range,
                      arg_selinux_apifs_context);

        if (r < 0)
                return r;

        r = mount_sysfs(NULL, arg_mount_settings);
        if (r < 0)
                return r;

        /* Wait until we are cgroup-ified, so that we
         * can mount the right cgroup path writable */
        if (!barrier_place_and_sync(barrier)) { /* #3 */
                log_error("Parent died too early");
                return -ESRCH;
        }

        if (arg_use_cgns && cg_ns_supported()) {
                r = unshare(CLONE_NEWCGROUP);
                if (r < 0)
                        return log_error_errno(errno, "Failed to unshare cgroup namespace");
                r = mount_cgroups(
                                "",
                                arg_unified_cgroup_hierarchy,
                                false,
                                arg_uid_shift,
                                arg_uid_range,
                                arg_selinux_apifs_context,
                                true);
                if (r < 0)
                        return r;
        } else {
                r = mount_systemd_cgroup_writable("", arg_unified_cgroup_hierarchy);
                if (r < 0)
                        return r;
        }

        umask(0022);

        if (setsid() < 0)
                return log_error_errno(errno, "setsid() failed: %m");

        r = drop_capabilities();
        if (r < 0)
                return log_error_errno(r, "drop_capabilities() failed: %m");

        setup_hostname();

        if (arg_personality != PERSONALITY_INVALID) {
                if (personality(arg_personality) < 0)
                        return log_error_errno(errno, "personality() failed: %m");
        } else if (secondary) {
                if (personality(PER_LINUX32) < 0)
                        return log_error_errno(errno, "personality() failed: %m");
        }

#ifdef HAVE_SELINUX
        if (arg_selinux_context)
                if (setexeccon(arg_selinux_context) < 0)
                        return log_error_errno(errno, "setexeccon(\"%s\") failed: %m", arg_selinux_context);
#endif

        r = change_uid_gid(NULL, &home);
        if (r < 0)
                return r;

        /* LXC sets container=lxc, so follow the scheme here */
        envp[n_env++] = strjoina("container=", arg_container_service_name);

        envp[n_env] = strv_find_prefix(environ, "TERM=");
        if (envp[n_env])
                n_env++;

        if ((asprintf((char**)(envp + n_env++), "HOME=%s", home ? home: "/root") < 0) ||
            (asprintf((char**)(envp + n_env++), "USER=%s", "root") < 0) ||
            (asprintf((char**)(envp + n_env++), "LOGNAME=%s", "root") < 0))
                return log_oom();

        assert(!sd_id128_is_null(arg_uuid));

        if (asprintf((char**)(envp + n_env++), "container_uuid=%s", id128_to_uuid_string(arg_uuid, as_uuid)) < 0)
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

        env_use = strv_env_merge(2, envp, arg_setenv);
        if (!env_use)
                return log_oom();

        /* Let the parent know that we are ready and
         * wait until the parent is ready with the
         * setup, too... */
        if (!barrier_place_and_sync(barrier)) { /* #4 */
                log_error("Parent died too early");
                return -ESRCH;
        }

        if (arg_chdir)
                if (chdir(arg_chdir) < 0)
                        return log_error_errno(errno, "Failed to change to specified working directory %s: %m", arg_chdir);

        if (arg_start_mode == START_PID2) {
                r = stub_pid1(arg_uuid);
                if (r < 0)
                        return r;
        }

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

        if (arg_start_mode == START_BOOT) {
                char **a;
                size_t m;

                /* Automatically search for the init system */

                m = strv_length(arg_parameters);
                a = newa(char*, m + 2);
                memcpy_safe(a + 1, arg_parameters, m * sizeof(char*));
                a[1 + m] = NULL;

                a[0] = (char*) "/usr/lib/systemd/systemd";
                execve(a[0], a, env_use);

                a[0] = (char*) "/lib/systemd/systemd";
                execve(a[0], a, env_use);

                a[0] = (char*) "/sbin/init";
                execve(a[0], a, env_use);

                exec_target = "/usr/lib/systemd/systemd, /lib/systemd/systemd, /sbin/init";
        } else if (!strv_isempty(arg_parameters)) {
                exec_target = arg_parameters[0];
                execvpe(arg_parameters[0], arg_parameters, env_use);
        } else {
                if (!arg_chdir)
                        /* If we cannot change the directory, we'll end up in /, that is expected. */
                        (void) chdir(home ?: "/root");

                execle("/bin/bash", "-bash", NULL, env_use);
                execle("/bin/sh", "-sh", NULL, env_use);

                exec_target = "/bin/bash, /bin/sh";
        }

        r = -errno;
        (void) log_open();
        return log_error_errno(r, "execv(%s) failed: %m", exec_target);
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
                const char *directory,
                const char *console,
                DissectedImage *dissected_image,
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
        assert(directory);
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

        if (dissected_image) {
                r = dissected_image_mount(dissected_image, directory, DISSECT_IMAGE_DISCARD_ON_LOOP|(arg_read_only ? DISSECT_IMAGE_READ_ONLY : 0));
                if (r < 0)
                        return r;
        }

        arg_uid_shift = 0;

        /* Turn directory into bind mount */
        r = mount_verbose(LOG_ERR, directory, directory, NULL, MS_BIND|MS_REC, NULL);
        if (r < 0)
                return r;

        r = setup_pivot_root(directory, NULL, NULL);
        if (r < 0)
                return r;

        r = setup_volatile(
                        directory,
                        arg_volatile_mode,
                        false,
                        arg_uid_shift,
                        arg_uid_range,
                        arg_selinux_context);
        if (r < 0)
                return r;

        r = setup_volatile_state(
                        directory,
                        arg_volatile_mode,
                        false,
                        arg_uid_shift,
                        arg_uid_range,
                        arg_selinux_context);
        if (r < 0)
                return r;

        /* Mark everything as shared so our mounts get propagated down. This is
         * required to make new bind mounts available in systemd services
         * inside the containter that create a new mount namespace.
         * See https://github.com/systemd/systemd/issues/3860
         * Further submounts (such as /dev) done after this will inherit the
         * shared propagation mode. */
        r = mount_verbose(LOG_ERR, NULL, directory, NULL, MS_SHARED|MS_REC, NULL);
        if (r < 0)
                return r;

        r = base_filesystem_create(directory, arg_uid_shift, (gid_t) arg_uid_shift);
        if (r < 0)
                return r;

        if (arg_read_only) {
                r = bind_remount_recursive(directory, true, NULL);
                if (r < 0)
                        return log_error_errno(r, "Failed to make tree read-only: %m");
        }

        r = mount_all(directory,
                      arg_mount_settings,
                      arg_uid_shift,
                      arg_uid_range,
                      arg_selinux_apifs_context);
        if (r < 0)
                return r;

        r = copy_devnodes(directory);
        if (r < 0)
                return r;

        dev_setup(directory, arg_uid_shift, arg_uid_shift);

        r = setup_pts(directory);
        if (r < 0)
                return r;

        r = setup_propagate(directory);
        if (r < 0)
                return r;

        r = setup_dev_console(directory, console);
        if (r < 0)
                return r;

        r = setup_seccomp(arg_caps_retain);
        if (r < 0)
                return r;

        r = setup_timezone(directory);
        if (r < 0)
                return r;

        r = setup_resolv_conf(directory);
        if (r < 0)
                return r;

        r = setup_machine_id(directory);
        if (r < 0)
                return r;

        r = setup_journal(directory);
        if (r < 0)
                return r;

        r = mount_custom(
                        directory,
                        arg_custom_mounts,
                        arg_n_custom_mounts,
                        false,
                        arg_uid_shift,
                        arg_uid_range,
                        arg_selinux_apifs_context);
        if (r < 0)
                return r;

        if (!arg_use_cgns || !cg_ns_supported()) {
                r = mount_cgroups(
                                directory,
                                arg_unified_cgroup_hierarchy,
                                false,
                                arg_uid_shift,
                                arg_uid_range,
                                arg_selinux_apifs_context,
                                false);
                if (r < 0)
                        return r;
        }

        r = mount_move_root(directory);
        if (r < 0)
                return log_error_errno(r, "Failed to move root directory: %m");

        fd = setup_sd_notify_child();
        if (fd < 0)
                return fd;

        pid = raw_clone(SIGCHLD|CLONE_NEWNS|
                        arg_clone_ns_flags);
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

                r = inner_child(barrier, directory, secondary, kmsg_socket, rtnl_socket, fds);
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

        l = send(uuid_socket, &arg_uuid, sizeof(arg_uuid), MSG_NOSIGNAL);
        if (l < 0)
                return log_error_errno(errno, "Failed to send machine ID: %m");
        if (l != sizeof(arg_uuid)) {
                log_error("Short write while sending machine ID.");
                return -EIO;
        }

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

static int setup_sd_notify_parent(sd_event *event, int fd, pid_t *inner_child_pid, sd_event_source **notify_event_source) {
        int r;

        r = sd_event_add_io(event, notify_event_source, fd, EPOLLIN, nspawn_dispatch_notify_fd, inner_child_pid);
        if (r < 0)
                return log_error_errno(r, "Failed to allocate notify event source: %m");

        (void) sd_event_source_set_description(*notify_event_source, "nspawn-notify");

        return 0;
}

static int run(int master,
               const char* console,
               DissectedImage *dissected_image,
               bool interactive,
               bool secondary,
               FDSet *fds,
               union in_addr_union *exposed,
               pid_t *pid, int *ret) {

        static const struct sigaction sa = {
                .sa_handler = nop_signal_handler,
                .sa_flags = SA_NOCLDSTOP|SA_RESTART,
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
        _cleanup_(sd_event_source_unrefp) sd_event_source *notify_event_source = NULL;
        _cleanup_(sd_event_unrefp) sd_event *event = NULL;
        _cleanup_(pty_forward_freep) PTYForward *forward = NULL;
        _cleanup_(sd_netlink_unrefp) sd_netlink *rtnl = NULL;
        ContainerStatus container_status = 0;
        char last_char = 0;
        int r;
        ssize_t l;
        sigset_t mask_chld;

        assert_se(sigemptyset(&mask_chld) == 0);
        assert_se(sigaddset(&mask_chld, SIGCHLD) == 0);

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
                                arg_directory,
                                console,
                                dissected_image,
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
        l = recv(uuid_socket_pair[0], &arg_uuid, sizeof arg_uuid, 0);
        if (l < 0)
                return log_error_errno(errno, "Failed to read container machine ID: %m");
        if (l != sizeof(arg_uuid)) {
                log_error("Short read while reading container machined ID.");
                return -EIO;
        }

        /* We also retrieve the socket used for notifications generated by outer child */
        notify_socket = receive_one_fd(notify_socket_pair[0], 0);
        if (notify_socket < 0)
                return log_error_errno(notify_socket,
                                       "Failed to receive notification socket from the outer child: %m");

        log_debug("Init process invoked as PID "PID_FMT, *pid);

        if (arg_slice || arg_property)
                log_notice("Machine and scope registration turned off, --slice= and --property= settings will have no effect.");

        r = sync_cgroup(*pid, arg_unified_cgroup_hierarchy, arg_uid_shift);
        if (r < 0)
                return r;

        r = create_subcgroup(*pid, arg_unified_cgroup_hierarchy);
        if (r < 0)
                return r;

        r = chown_cgroup(*pid, arg_uid_shift);
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

        r = setup_sd_notify_parent(event, notify_socket, PID_TO_PTR(*pid), &notify_event_source);
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
        if (!arg_notify_ready)
                sd_notify(false, "READY=1\n");

        if (arg_kill_signal > 0) {
                /* Try to kill the init system on SIGINT or SIGTERM */
                sd_event_add_signal(event, NULL, SIGINT, on_orderly_shutdown, PID_TO_PTR(*pid));
                sd_event_add_signal(event, NULL, SIGTERM, on_orderly_shutdown, PID_TO_PTR(*pid));
        } else {
                /* Immediately exit */
                sd_event_add_signal(event, NULL, SIGINT, NULL, NULL);
                sd_event_add_signal(event, NULL, SIGTERM, NULL, NULL);
        }

        /* Exit when the child exits */
        sd_event_add_signal(event, NULL, SIGCHLD, on_sigchld, PID_TO_PTR(*pid));

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

        if (!arg_quiet && last_char != '\n')
                putc('\n', stdout);

        /* Normally redundant, but better safe than sorry */
        (void) kill(*pid, SIGKILL);

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
                if (r == EXIT_FORCE_RESTART)
                        r = EXIT_FAILURE; /* replace 133 with the general failure code */
                *ret = r;
                return 0; /* finito */
        }

        /* CONTAINER_REBOOTED, loop again */

        /* Special handling if we are running as a service: instead of simply
         * restarting the machine we want to restart the entire service, so let's
         * inform systemd about this with the special exit code 133. The service
         * file uses RestartForceExitStatus=133 so that this results in a full
         * nspawn restart. This is necessary since we might have cgroup parameters
         * set we want to have flushed out. */
        *ret = EXIT_FORCE_RESTART;
        return 0; /* finito */
}

int main(int argc, char *argv[]) {

        _cleanup_free_ char *console = NULL;
        _cleanup_close_ int master = -1;
        _cleanup_fdset_free_ FDSet *fds = NULL;
        int r, ret = EXIT_SUCCESS;
        bool secondary = false;
        pid_t pid = 0;
        union in_addr_union exposed = {};
        _cleanup_release_lock_file_ LockFile tree_global_lock = LOCK_FILE_INIT, tree_local_lock = LOCK_FILE_INIT;
        bool interactive;
        _cleanup_(loop_device_unrefp) LoopDevice *loop = NULL;
        _cleanup_(decrypted_image_unrefp) DecryptedImage *decrypted_image = NULL;
        _cleanup_(dissected_image_unrefp) DissectedImage *dissected_image = NULL;

        log_parse_environment();
        log_open();

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

        r = verify_arguments();
        if (r < 0)
                goto finish;

        if (arg_directory) {
                if (path_equal(arg_directory, "/")) {
                        log_error("Spawning container on root directory is not supported. Consider using --ephemeral.");
                        r = -EINVAL;
                        goto finish;
                }

                r = chase_symlinks_and_update(&arg_directory, 0);
                if (r < 0)
                        goto finish;

                r = image_path_lock(arg_directory, (arg_read_only ? LOCK_SH : LOCK_EX) | LOCK_NB, &tree_global_lock, &tree_local_lock);
                if (r == -EBUSY) {
                        log_error_errno(r, "Directory tree %s is currently busy.", arg_directory);
                        goto finish;
                }
                if (r < 0) {
                        log_error_errno(r, "Failed to lock %s: %m", arg_directory);
                        goto finish;
                }

                if (arg_start_mode == START_BOOT) {
                } else {
                        const char *p;

                        p = strjoina(arg_directory, "/usr/");
                        if (laccess(p, F_OK) < 0) {
                                log_error("Directory %s doesn't look like it has an OS tree. Refusing.", arg_directory);
                                r = -EINVAL;
                                goto finish;
                        }
                }

        }

        r = detect_unified_cgroup_hierarchy(arg_directory);
        if (r < 0)
                goto finish;

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

        if (unlockpt(master) < 0) {
                r = log_error_errno(errno, "Failed to unlock tty: %m");
                goto finish;
        }

        assert_se(sigprocmask_many(SIG_BLOCK, NULL, SIGCHLD, SIGWINCH, SIGTERM, SIGINT, -1) >= 0);

        if (prctl(PR_SET_CHILD_SUBREAPER, 1) < 0) {
                r = log_error_errno(errno, "Failed to become subreaper: %m");
                goto finish;
        }

                r = run(master,
                        console,
                        dissected_image,
                        interactive, secondary,
                        fds,
                        &exposed,
                        &pid, &ret);

finish:
        if (pid > 0)
                (void) kill(pid, SIGKILL);

        /* Try to flush whatever is still queued in the pty */
        if (master >= 0) {
                (void) copy_bytes(master, STDOUT_FILENO, (uint64_t) -1, 0);
                master = safe_close(master);
        }

        if (pid > 0)
                (void) wait_for_terminate(pid, NULL);

        free(arg_directory);
        free(arg_machine);
        free(arg_chdir);
        strv_free(arg_setenv);
        free(arg_network_bridge);
        strv_free(arg_network_interfaces);
        strv_free(arg_network_macvlan);
        strv_free(arg_network_ipvlan);
        strv_free(arg_parameters);
        custom_mount_free_all(arg_custom_mounts, arg_n_custom_mounts);
        free(arg_root_hash);

        return r < 0 ? EXIT_FAILURE : ret;
}
