/***
  This file is part of systemd.

  Copyright 2010 Lennart Poettering
  Copyright 2017 Luke Shumaker

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

#include <errno.h>
#include <getopt.h>
#include <grp.h>   /* getgrgid */
#include <pwd.h>   /* getpwuid */
#include <sched.h> /* CLONE_NEW* */

#include "sd-id128/id128-util.h"
#include "systemd-basic/alloc-util.h"     /* mfree, _cleanup_free_ */
#include "systemd-basic/btrfs-util.h"     /* btrfs_subvol_snapshot */
#include "systemd-basic/cap-list.h"       /* capability_from_name */
#include "systemd-basic/cgroup-util.h"
#include "systemd-basic/env-util.h"       /* getenv_bool */
#include "systemd-basic/fd-util.h"        /* _cleanup_fclose_ */
#include "systemd-basic/fileio.h"         /* tempfn_random */
#include "systemd-basic/hostname-util.h"  /* machine_name_is_valid */
#include "systemd-basic/lockfile-util.h"  /* LockFile */
#include "systemd-basic/mount-util.h"     /* path_is_mount_point */
#include "systemd-basic/parse-util.h"     /* parse_boolean */
#include "systemd-basic/path-util.h"      /* path_equal, systemd_installation_has_version, parse_path_argument_and_warn */
#include "systemd-basic/process-util.h"   /* PERSONALITY_INVALID */
#include "systemd-basic/random-util.h"    /* random_u64 */
#include "systemd-basic/signal-util.h"    /* signal_from_string_try_harder */
#include "systemd-basic/socket-util.h"    /* ifname_valid */
#include "systemd-basic/stdio-util.h"     /* xsprintf */
#include "systemd-basic/string-util.h"    /* free_and_strdup, strappend, isempty, streq */
#include "systemd-basic/strv.h"
#include "systemd-basic/user-util.h"      /* UID_INVALID */
#include "systemd-shared/machine-image.h" /* Image, image_find, IMAGE_RAW */

#include "nspawn-args.h"
#include "nspawn-expose-ports.h" /* expose_port_parse */
#include "nspawn-mount.h"        /* custom_mount_compare, tmpfs_mount_parse */
#include "nspawn-network.h"      /* veth_extra_parse */
#include "nspawn-settings.h"

/* Note that devpts's gid= parameter parses GIDs as signed values, hence we stay away from the upper half of the 32bit
 * UID range here. We leave a bit of room at the lower end and a lot of room at the upper end, so that other subsystems
 * may have their own allocation ranges too. */
#define UID_SHIFT_PICK_MIN ((uid_t) UINT32_C(0x00080000))
#define UID_SHIFT_PICK_MAX ((uid_t) UINT32_C(0x6FFF0000))

static Args _args = {
        .arg_directory = NULL,
        .arg_template = NULL,
        .arg_chdir = NULL,
        .arg_user = NULL,
        .arg_uuid = {},
        .arg_machine = NULL,
        .arg_selinux_context = NULL,
        .arg_selinux_apifs_context = NULL,
        .arg_slice = NULL,
        .arg_private_network = false,
        .arg_read_only = false,
        .arg_start_mode = START_PID1,
        .arg_ephemeral = false,
        .arg_link_journal = LINK_AUTO,
        .arg_link_journal_try = false,
        .arg_caps_retain =
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
                (1ULL << CAP_SYS_TTY_CONFIG),
        .arg_custom_mounts = NULL,
        .arg_n_custom_mounts = 0,
        .arg_setenv = NULL,
        .arg_quiet = false,
        .arg_register = true,
        .arg_keep_unit = false,
        .arg_network_interfaces = NULL,
        .arg_network_macvlan = NULL,
        .arg_network_ipvlan = NULL,
        .arg_network_veth = false,
        .arg_network_veth_extra = NULL,
        .arg_network_bridge = NULL,
        .arg_network_zone = NULL,
        .arg_personality = PERSONALITY_INVALID,
        .arg_image = NULL,
        .arg_volatile_mode = VOLATILE_NO,
        .arg_expose_ports = NULL,
        .arg_property = NULL,
        .arg_userns_mode = USER_NAMESPACE_NO,
        .arg_uid_shift = UID_INVALID, .arg_uid_range = 0x10000U,
        .arg_userns_chown = false,
        .arg_kill_signal = 0,
        .arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_UNKNOWN,
        .arg_settings_mask = 0,
        .arg_settings_trusted = -1,
        .arg_parameters = NULL,
        .arg_container_service_name = "systemd-nspawn",
        .arg_notify_ready = false,
        .arg_use_cgns = true,
        .arg_clone_ns_flags = CLONE_NEWIPC|CLONE_NEWPID|CLONE_NEWUTS,
};

static Args * const args = &_args;

Args const *args_get(void) {
        return args;
}

static void help(void) {
        printf("%s [OPTIONS...] [PATH] [ARGUMENTS...]\n\n"
               "Spawn a minimal namespace container for debugging, testing and building.\n\n"
               "  -h --help                 Show this help\n"
               "     --version              Print version string\n"
               "  -q --quiet                Do not show status information\n"
               "  -D --directory=PATH       Root directory for the container\n"
               "     --template=PATH        Initialize root directory from template directory,\n"
               "                            if missing\n"
               "  -x --ephemeral            Run container with snapshot of root directory, and\n"
               "                            remove it after exit\n"
               "  -i --image=PATH           File system device or disk image for the container\n"
               "  -a --as-pid2              Maintain a stub init as PID1, invoke binary as PID2\n"
               "  -b --boot                 Boot up full system (i.e. invoke init)\n"
               "     --chdir=PATH           Set working directory in the container\n"
               "  -u --user=USER            Run the command under specified user or uid\n"
               "  -M --machine=NAME         Set the machine name for the container\n"
               "     --uuid=UUID            Set a specific machine UUID for the container\n"
               "  -S --slice=SLICE          Place the container in the specified slice\n"
               "     --property=NAME=VALUE  Set scope unit property\n"
               "     --private-users[=yes]  Run within user namespace, detect UID/GID range\n"
               "     --private-users=UIDBASE[:NUIDS]\n"
               "                            Similar, but with user configured UID/GID range\n"
               "     --private-users=pick   Similar, but autoselect an unused UID/GID range,\n"
               "                            implies --private-users-chown"
               "  -U                        If the kernel supports the user namespaces feature,\n"
               "                            equivalent to --private-users=pick; otherwise ignored\n"
               "     --private-users-chown  Adjust OS tree ownership to private UID/GID range\n"
               "     --private-network      Disable network in container\n"
               "     --network-interface=INTERFACE\n"
               "                            Assign an existing network interface to the\n"
               "                            container\n"
               "     --network-macvlan=INTERFACE\n"
               "                            Create a macvlan network interface based on an\n"
               "                            existing network interface to the container\n"
               "     --network-ipvlan=INTERFACE\n"
               "                            Create a ipvlan network interface based on an\n"
               "                            existing network interface to the container\n"
               "  -n --network-veth         Add a virtual Ethernet connection between host\n"
               "                            and container\n"
               "     --network-veth-extra=HOSTIF[:CONTAINERIF]\n"
               "                            Add an additional virtual Ethernet link between\n"
               "                            host and container\n"
               "     --network-bridge=INTERFACE\n"
               "                            Add a virtual Ethernet connection to the container\n"
               "                            and attach it to an existing bridge on the host\n"
               "     --network-zone=NAME    Similar, but attach the new interface to an\n"
               "                            an automatically managed bridge interface\n"
               "  -p --port=[PROTOCOL:]HOSTPORT[:CONTAINERPORT]\n"
               "                            Expose a container IP port on the host\n"
               "  -Z --selinux-context=SECLABEL\n"
               "                            Set the SELinux security context to be used by\n"
               "                            processes in the container\n"
               "  -L --selinux-apifs-context=SECLABEL\n"
               "                            Set the SELinux security context to be used by\n"
               "                            API/tmpfs file systems in the container\n"
               "     --capability=CAP       In addition to the default, retain specified\n"
               "                            capability\n"
               "     --drop-capability=CAP  Drop the specified capability from the default set\n"
               "     --kill-signal=SIGNAL   Select signal to use for shutting down PID 1\n"
               "     --link-journal=MODE    Link up guest journal, one of no, auto, guest, \n"
               "                            host, try-guest, try-host\n"
               "  -j                        Equivalent to --link-journal=try-guest\n"
               "     --read-only            Mount the root directory read-only\n"
               "     --bind=PATH[:PATH[:OPTIONS]]\n"
               "                            Bind mount a file or directory from the host into\n"
               "                            the container\n"
               "     --bind-ro=PATH[:PATH[:OPTIONS]\n"
               "                            Similar, but creates a read-only bind mount\n"
               "     --tmpfs=PATH:[OPTIONS] Mount an empty tmpfs to the specified directory\n"
               "     --overlay=PATH[:PATH...]:PATH\n"
               "                            Create an overlay mount from the host to \n"
               "                            the container\n"
               "     --overlay-ro=PATH[:PATH...]:PATH\n"
               "                            Similar, but creates a read-only overlay mount\n"
               "  -E --setenv=NAME=VALUE    Pass an environment variable to PID 1\n"
               "     --register=BOOLEAN     Register container as machine\n"
               "     --keep-unit            Do not register a scope for the machine, reuse\n"
               "                            the service unit nspawn is running in\n"
               "     --volatile[=MODE]      Run the system in volatile mode\n"
               "     --settings=BOOLEAN     Load additional settings from .nspawn file\n"
               "     --notify-ready=BOOLEAN Receive notifications from the child init process\n"
               , program_invocation_short_name);
}

int custom_mounts_prepare(void) {
        unsigned i;
        int r;

        /* Ensure the mounts are applied prefix first. */
        qsort_safe(args->arg_custom_mounts, args->arg_n_custom_mounts, sizeof(CustomMount), custom_mount_compare);

        /* Allocate working directories for the overlay file systems that need it */
        for (i = 0; i < args->arg_n_custom_mounts; i++) {
                CustomMount *m = &args->arg_custom_mounts[i];

                if (path_equal(m->destination, "/") && args->arg_userns_mode != USER_NAMESPACE_NO) {

                        if (args->arg_userns_chown) {
                                log_error("--private-users-chown may not be combined with custom root mounts.");
                                return -EINVAL;
                        } else if (args->arg_uid_shift == UID_INVALID) {
                                log_error("--private-users with automatic UID shift may not be combined with custom root mounts.");
                                return -EINVAL;
                        }
                }

                if (m->type != CUSTOM_MOUNT_OVERLAY)
                        continue;

                if (m->work_dir)
                        continue;

                if (m->read_only)
                        continue;

                r = tempfn_random(m->source, NULL, &m->work_dir);
                if (r < 0)
                        return log_error_errno(r, "Failed to generate work directory from %s: %m", m->source);
        }

        return 0;
}

int detect_unified_cgroup_hierarchy(const char *directory) {
        const char *e;
        int r;
        CGroupUnified outer;

        /* Allow the user to control whether the unified hierarchy is used */
        e = getenv("UNIFIED_CGROUP_HIERARCHY");
        if (e) {
                r = parse_boolean(e);
                if (r < 0)
                        return log_error_errno(r, "Failed to decide cgroup version to use: Failed to parse $UNIFIED_CGROUP_HIERARCHY.");
                if (r > 0)
                        args->arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_ALL;
                else
                        args->arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_NONE;

                return 0;
        }

        r = cg_version(&outer);
        if (r < 0)
                return log_error_errno(r, "Failed to decide cgroup version to use: Failed to determine what the host system uses: %m");

        /* Otherwise inherit the default from the host system, unless
         * the container doesn't have a new enough systemd (detected
         * by checking libsystemd-shared). */
        switch (outer) {
        case CGROUP_UNIFIED_UNKNOWN:
                assert_not_reached("Unknown host cgroup version");
                break;
        case CGROUP_UNIFIED_NONE: /* cgroup v1 */
                args->arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_NONE;
                break;
        case CGROUP_UNIFIED_ALL: /* cgroup v2 */
                /* Unified cgroup hierarchy support was added in 230. Unfortunately libsystemd-shared,
                 * which we use to sniff the systemd version, was only added in 231, so we'll have a
                 * false negative here for 230. */
                r = systemd_installation_has_version(directory, 230);
                if (r < 0)
                        return log_error_errno(r, "Failed to decide cgroup version to use: Failed to determine systemd version in container: %m");
                if (r > 0)
                        args->arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_ALL;
                else
                        args->arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_NONE;
                break;
        case CGROUP_UNIFIED_SYSTEMD: /* cgroup v1 & v2 mixed; but v2 for systemd */
                /* Mixed cgroup hierarchy support was added in 232 */
                r = systemd_installation_has_version(directory, 232);
                if (r < 0)
                        return log_error_errno(r, "Failed to decide cgroup version to use: Failed to determine systemd version in container: %m");
                if (r > 0)
                        args->arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_SYSTEMD;
                else
                        args->arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_NONE;
                break;
        }

        return 0;
}

static void parse_share_ns_env(const char *name, unsigned long ns_flag) {
        int r;

        r = getenv_bool(name);
        if (r == -ENXIO)
                return;
        if (r < 0)
                log_warning_errno(r, "Failed to parse %s from environment, defaulting to false.", name);
        args->arg_clone_ns_flags = (args->arg_clone_ns_flags & ~ns_flag) | (r > 0 ? 0 : ns_flag);
}

int parse_argv(int argc, char *argv[]) {

        enum {
                ARG_VERSION = 0x100,
                ARG_PRIVATE_NETWORK,
                ARG_UUID,
                ARG_READ_ONLY,
                ARG_CAPABILITY,
                ARG_DROP_CAPABILITY,
                ARG_LINK_JOURNAL,
                ARG_BIND,
                ARG_BIND_RO,
                ARG_TMPFS,
                ARG_OVERLAY,
                ARG_OVERLAY_RO,
                ARG_SHARE_SYSTEM,
                ARG_REGISTER,
                ARG_KEEP_UNIT,
                ARG_NETWORK_INTERFACE,
                ARG_NETWORK_MACVLAN,
                ARG_NETWORK_IPVLAN,
                ARG_NETWORK_BRIDGE,
                ARG_NETWORK_ZONE,
                ARG_NETWORK_VETH_EXTRA,
                ARG_PERSONALITY,
                ARG_VOLATILE,
                ARG_TEMPLATE,
                ARG_PROPERTY,
                ARG_PRIVATE_USERS,
                ARG_KILL_SIGNAL,
                ARG_SETTINGS,
                ARG_CHDIR,
                ARG_PRIVATE_USERS_CHOWN,
                ARG_NOTIFY_READY,
        };

        static const struct option options[] = {
                { "help",                  no_argument,       NULL, 'h'                     },
                { "version",               no_argument,       NULL, ARG_VERSION             },
                { "directory",             required_argument, NULL, 'D'                     },
                { "template",              required_argument, NULL, ARG_TEMPLATE            },
                { "ephemeral",             no_argument,       NULL, 'x'                     },
                { "user",                  required_argument, NULL, 'u'                     },
                { "private-network",       no_argument,       NULL, ARG_PRIVATE_NETWORK     },
                { "as-pid2",               no_argument,       NULL, 'a'                     },
                { "boot",                  no_argument,       NULL, 'b'                     },
                { "uuid",                  required_argument, NULL, ARG_UUID                },
                { "read-only",             no_argument,       NULL, ARG_READ_ONLY           },
                { "capability",            required_argument, NULL, ARG_CAPABILITY          },
                { "drop-capability",       required_argument, NULL, ARG_DROP_CAPABILITY     },
                { "link-journal",          required_argument, NULL, ARG_LINK_JOURNAL        },
                { "bind",                  required_argument, NULL, ARG_BIND                },
                { "bind-ro",               required_argument, NULL, ARG_BIND_RO             },
                { "tmpfs",                 required_argument, NULL, ARG_TMPFS               },
                { "overlay",               required_argument, NULL, ARG_OVERLAY             },
                { "overlay-ro",            required_argument, NULL, ARG_OVERLAY_RO          },
                { "machine",               required_argument, NULL, 'M'                     },
                { "slice",                 required_argument, NULL, 'S'                     },
                { "setenv",                required_argument, NULL, 'E'                     },
                { "selinux-context",       required_argument, NULL, 'Z'                     },
                { "selinux-apifs-context", required_argument, NULL, 'L'                     },
                { "quiet",                 no_argument,       NULL, 'q'                     },
                { "share-system",          no_argument,       NULL, ARG_SHARE_SYSTEM        }, /* not documented */
                { "register",              required_argument, NULL, ARG_REGISTER            },
                { "keep-unit",             no_argument,       NULL, ARG_KEEP_UNIT           },
                { "network-interface",     required_argument, NULL, ARG_NETWORK_INTERFACE   },
                { "network-macvlan",       required_argument, NULL, ARG_NETWORK_MACVLAN     },
                { "network-ipvlan",        required_argument, NULL, ARG_NETWORK_IPVLAN      },
                { "network-veth",          no_argument,       NULL, 'n'                     },
                { "network-veth-extra",    required_argument, NULL, ARG_NETWORK_VETH_EXTRA  },
                { "network-bridge",        required_argument, NULL, ARG_NETWORK_BRIDGE      },
                { "network-zone",          required_argument, NULL, ARG_NETWORK_ZONE        },
                { "personality",           required_argument, NULL, ARG_PERSONALITY         },
                { "image",                 required_argument, NULL, 'i'                     },
                { "volatile",              optional_argument, NULL, ARG_VOLATILE            },
                { "port",                  required_argument, NULL, 'p'                     },
                { "property",              required_argument, NULL, ARG_PROPERTY            },
                { "private-users",         optional_argument, NULL, ARG_PRIVATE_USERS       },
                { "private-users-chown",   optional_argument, NULL, ARG_PRIVATE_USERS_CHOWN },
                { "kill-signal",           required_argument, NULL, ARG_KILL_SIGNAL         },
                { "settings",              required_argument, NULL, ARG_SETTINGS            },
                { "chdir",                 required_argument, NULL, ARG_CHDIR               },
                { "notify-ready",          required_argument, NULL, ARG_NOTIFY_READY        },
                {}
        };

        int c, r;
        const char *p, *e;
        uint64_t plus = 0, minus = 0;
        bool mask_all_settings = false, mask_no_settings = false;

        assert(argc >= 0);
        assert(argv);

        while ((c = getopt_long(argc, argv, "+hD:u:abL:M:jS:Z:qi:xp:nUE:", options, NULL)) >= 0)

                switch (c) {

                case 'h':
                        help();
                        return 0;

                case ARG_VERSION:
                        return version();

                case 'D':
                        r = parse_path_argument_and_warn(optarg, false, &args->arg_directory);
                        if (r < 0)
                                return r;
                        break;

                case ARG_TEMPLATE:
                        r = parse_path_argument_and_warn(optarg, false, &args->arg_template);
                        if (r < 0)
                                return r;
                        break;

                case 'i':
                        r = parse_path_argument_and_warn(optarg, false, &args->arg_image);
                        if (r < 0)
                                return r;
                        break;

                case 'x':
                        args->arg_ephemeral = true;
                        break;

                case 'u':
                        r = free_and_strdup(&args->arg_user, optarg);
                        if (r < 0)
                                return log_oom();

                        args->arg_settings_mask |= SETTING_USER;
                        break;

                case ARG_NETWORK_ZONE: {
                        char *j;

                        j = strappend("vz-", optarg);
                        if (!j)
                                return log_oom();

                        if (!ifname_valid(j)) {
                                log_error("Network zone name not valid: %s", j);
                                free(j);
                                return -EINVAL;
                        }

                        free(args->arg_network_zone);
                        args->arg_network_zone = j;

                        args->arg_network_veth = true;
                        args->arg_private_network = true;
                        args->arg_settings_mask |= SETTING_NETWORK;
                        break;
                }

                case ARG_NETWORK_BRIDGE:

                        if (!ifname_valid(optarg)) {
                                log_error("Bridge interface name not valid: %s", optarg);
                                return -EINVAL;
                        }

                        r = free_and_strdup(&args->arg_network_bridge, optarg);
                        if (r < 0)
                                return log_oom();

                        /* fall through */

                case 'n':
                        args->arg_network_veth = true;
                        args->arg_private_network = true;
                        args->arg_settings_mask |= SETTING_NETWORK;
                        break;

                case ARG_NETWORK_VETH_EXTRA:
                        r = veth_extra_parse(&args->arg_network_veth_extra, optarg);
                        if (r < 0)
                                return log_error_errno(r, "Failed to parse --network-veth-extra= parameter: %s", optarg);

                        args->arg_private_network = true;
                        args->arg_settings_mask |= SETTING_NETWORK;
                        break;

                case ARG_NETWORK_INTERFACE:

                        if (!ifname_valid(optarg)) {
                                log_error("Network interface name not valid: %s", optarg);
                                return -EINVAL;
                        }

                        if (strv_extend(&args->arg_network_interfaces, optarg) < 0)
                                return log_oom();

                        args->arg_private_network = true;
                        args->arg_settings_mask |= SETTING_NETWORK;
                        break;

                case ARG_NETWORK_MACVLAN:

                        if (!ifname_valid(optarg)) {
                                log_error("MACVLAN network interface name not valid: %s", optarg);
                                return -EINVAL;
                        }

                        if (strv_extend(&args->arg_network_macvlan, optarg) < 0)
                                return log_oom();

                        args->arg_private_network = true;
                        args->arg_settings_mask |= SETTING_NETWORK;
                        break;

                case ARG_NETWORK_IPVLAN:

                        if (!ifname_valid(optarg)) {
                                log_error("IPVLAN network interface name not valid: %s", optarg);
                                return -EINVAL;
                        }

                        if (strv_extend(&args->arg_network_ipvlan, optarg) < 0)
                                return log_oom();

                        /* fall through */

                case ARG_PRIVATE_NETWORK:
                        args->arg_private_network = true;
                        args->arg_settings_mask |= SETTING_NETWORK;
                        break;

                case 'b':
                        if (args->arg_start_mode == START_PID2) {
                                log_error("--boot and --as-pid2 may not be combined.");
                                return -EINVAL;
                        }

                        args->arg_start_mode = START_BOOT;
                        args->arg_settings_mask |= SETTING_START_MODE;
                        break;

                case 'a':
                        if (args->arg_start_mode == START_BOOT) {
                                log_error("--boot and --as-pid2 may not be combined.");
                                return -EINVAL;
                        }

                        args->arg_start_mode = START_PID2;
                        args->arg_settings_mask |= SETTING_START_MODE;
                        break;

                case ARG_UUID:
                        r = sd_id128_from_string(optarg, &args->arg_uuid);
                        if (r < 0)
                                return log_error_errno(r, "Invalid UUID: %s", optarg);

                        if (sd_id128_is_null(args->arg_uuid)) {
                                log_error("Machine UUID may not be all zeroes.");
                                return -EINVAL;
                        }

                        args->arg_settings_mask |= SETTING_MACHINE_ID;
                        break;

                case 'S':
                        args->arg_slice = optarg;
                        break;

                case 'M':
                        if (isempty(optarg))
                                args->arg_machine = mfree(args->arg_machine);
                        else {
                                if (!machine_name_is_valid(optarg)) {
                                        log_error("Invalid machine name: %s", optarg);
                                        return -EINVAL;
                                }

                                r = free_and_strdup(&args->arg_machine, optarg);
                                if (r < 0)
                                        return log_oom();

                                break;
                        }

                case 'Z':
                        args->arg_selinux_context = optarg;
                        break;

                case 'L':
                        args->arg_selinux_apifs_context = optarg;
                        break;

                case ARG_READ_ONLY:
                        args->arg_read_only = true;
                        args->arg_settings_mask |= SETTING_READ_ONLY;
                        break;

                case ARG_CAPABILITY:
                case ARG_DROP_CAPABILITY: {
                        p = optarg;
                        for (;;) {
                                _cleanup_free_ char *t = NULL;

                                r = extract_first_word(&p, &t, ",", 0);
                                if (r < 0)
                                        return log_error_errno(r, "Failed to parse capability %s.", t);

                                if (r == 0)
                                        break;

                                if (streq(t, "all")) {
                                        if (c == ARG_CAPABILITY)
                                                plus = (uint64_t) -1;
                                        else
                                                minus = (uint64_t) -1;
                                } else {
                                        int cap;

                                        cap = capability_from_name(t);
                                        if (cap < 0) {
                                                log_error("Failed to parse capability %s.", t);
                                                return -EINVAL;
                                        }

                                        if (c == ARG_CAPABILITY)
                                                plus |= 1ULL << (uint64_t) cap;
                                        else
                                                minus |= 1ULL << (uint64_t) cap;
                                }
                        }

                        args->arg_settings_mask |= SETTING_CAPABILITY;
                        break;
                }

                case 'j':
                        args->arg_link_journal = LINK_GUEST;
                        args->arg_link_journal_try = true;
                        break;

                case ARG_LINK_JOURNAL:
                        if (streq(optarg, "auto")) {
                                args->arg_link_journal = LINK_AUTO;
                                args->arg_link_journal_try = false;
                        } else if (streq(optarg, "no")) {
                                args->arg_link_journal = LINK_NO;
                                args->arg_link_journal_try = false;
                        } else if (streq(optarg, "guest")) {
                                args->arg_link_journal = LINK_GUEST;
                                args->arg_link_journal_try = false;
                        } else if (streq(optarg, "host")) {
                                args->arg_link_journal = LINK_HOST;
                                args->arg_link_journal_try = false;
                        } else if (streq(optarg, "try-guest")) {
                                args->arg_link_journal = LINK_GUEST;
                                args->arg_link_journal_try = true;
                        } else if (streq(optarg, "try-host")) {
                                args->arg_link_journal = LINK_HOST;
                                args->arg_link_journal_try = true;
                        } else {
                                log_error("Failed to parse link journal mode %s", optarg);
                                return -EINVAL;
                        }

                        break;

                case ARG_BIND:
                case ARG_BIND_RO:
                        r = bind_mount_parse(&args->arg_custom_mounts, &args->arg_n_custom_mounts, optarg, c == ARG_BIND_RO);
                        if (r < 0)
                                return log_error_errno(r, "Failed to parse --bind(-ro)= argument %s: %m", optarg);

                        args->arg_settings_mask |= SETTING_CUSTOM_MOUNTS;
                        break;

                case ARG_TMPFS:
                        r = tmpfs_mount_parse(&args->arg_custom_mounts, &args->arg_n_custom_mounts, optarg);
                        if (r < 0)
                                return log_error_errno(r, "Failed to parse --tmpfs= argument %s: %m", optarg);

                        args->arg_settings_mask |= SETTING_CUSTOM_MOUNTS;
                        break;

                case ARG_OVERLAY:
                case ARG_OVERLAY_RO: {
                        _cleanup_free_ char *upper = NULL, *destination = NULL;
                        _cleanup_strv_free_ char **lower = NULL;
                        CustomMount *m;
                        unsigned n = 0;
                        char **i;

                        r = strv_split_extract(&lower, optarg, ":", EXTRACT_DONT_COALESCE_SEPARATORS);
                        if (r == -ENOMEM)
                                return log_oom();
                        else if (r < 0) {
                                log_error("Invalid overlay specification: %s", optarg);
                                return r;
                        }

                        STRV_FOREACH(i, lower) {
                                if (!path_is_absolute(*i)) {
                                        log_error("Overlay path %s is not absolute.", *i);
                                        return -EINVAL;
                                }

                                n++;
                        }

                        if (n < 2) {
                                log_error("--overlay= needs at least two colon-separated directories specified.");
                                return -EINVAL;
                        }

                        if (n == 2) {
                                /* If two parameters are specified,
                                 * the first one is the lower, the
                                 * second one the upper directory. And
                                 * we'll also define the destination
                                 * mount point the same as the upper. */
                                upper = lower[1];
                                lower[1] = NULL;

                                destination = strdup(upper);
                                if (!destination)
                                        return log_oom();

                        } else {
                                upper = lower[n - 2];
                                destination = lower[n - 1];
                                lower[n - 2] = NULL;
                        }

                        m = custom_mount_add(&args->arg_custom_mounts, &args->arg_n_custom_mounts, CUSTOM_MOUNT_OVERLAY);
                        if (!m)
                                return log_oom();

                        m->destination = destination;
                        m->source = upper;
                        m->lower = lower;
                        m->read_only = c == ARG_OVERLAY_RO;

                        upper = destination = NULL;
                        lower = NULL;

                        args->arg_settings_mask |= SETTING_CUSTOM_MOUNTS;
                        break;
                }

                case 'E': {
                        char **n;

                        if (!env_assignment_is_valid(optarg)) {
                                log_error("Environment variable assignment '%s' is not valid.", optarg);
                                return -EINVAL;
                        }

                        n = strv_env_set(args->arg_setenv, optarg);
                        if (!n)
                                return log_oom();

                        strv_free(args->arg_setenv);
                        args->arg_setenv = n;

                        args->arg_settings_mask |= SETTING_ENVIRONMENT;
                        break;
                }

                case 'q':
                        args->arg_quiet = true;
                        break;

                case ARG_SHARE_SYSTEM:
                        /* We don't officially support this anymore, except for compat reasons. People should use the
                         * $SYSTEMD_NSPAWN_SHARE_* environment variables instead. */
                        args->arg_clone_ns_flags = 0;
                        break;

                case ARG_REGISTER:
                        r = parse_boolean(optarg);
                        if (r < 0) {
                                log_error("Failed to parse --register= argument: %s", optarg);
                                return r;
                        }

                        args->arg_register = r;
                        break;

                case ARG_KEEP_UNIT:
                        args->arg_keep_unit = true;
                        break;

                case ARG_PERSONALITY:

                        args->arg_personality = personality_from_string(optarg);
                        if (args->arg_personality == PERSONALITY_INVALID) {
                                log_error("Unknown or unsupported personality '%s'.", optarg);
                                return -EINVAL;
                        }

                        args->arg_settings_mask |= SETTING_PERSONALITY;
                        break;

                case ARG_VOLATILE:

                        if (!optarg)
                                args->arg_volatile_mode = VOLATILE_YES;
                        else {
                                VolatileMode m;

                                m = volatile_mode_from_string(optarg);
                                if (m < 0) {
                                        log_error("Failed to parse --volatile= argument: %s", optarg);
                                        return -EINVAL;
                                } else
                                        args->arg_volatile_mode = m;
                        }

                        args->arg_settings_mask |= SETTING_VOLATILE_MODE;
                        break;

                case 'p':
                        r = expose_port_parse(&args->arg_expose_ports, optarg);
                        if (r == -EEXIST)
                                return log_error_errno(r, "Duplicate port specification: %s", optarg);
                        if (r < 0)
                                return log_error_errno(r, "Failed to parse host port %s: %m", optarg);

                        args->arg_settings_mask |= SETTING_EXPOSE_PORTS;
                        break;

                case ARG_PROPERTY:
                        if (strv_extend(&args->arg_property, optarg) < 0)
                                return log_oom();

                        break;

                case ARG_PRIVATE_USERS: {
                        int boolean = -1;

                        if (!optarg)
                                boolean = true;
                        else if (!in_charset(optarg, DIGITS))
                                /* do *not* parse numbers as booleans */
                                boolean = parse_boolean(optarg);

                        if (boolean == false) {
                                /* no: User namespacing off */
                                args->arg_userns_mode = USER_NAMESPACE_NO;
                                args->arg_uid_shift = UID_INVALID;
                                args->arg_uid_range = UINT32_C(0x10000);
                        } else if (boolean == true) {
                                /* yes: User namespacing on, UID range is read from root dir */
                                args->arg_userns_mode = USER_NAMESPACE_FIXED;
                                args->arg_uid_shift = UID_INVALID;
                                args->arg_uid_range = UINT32_C(0x10000);
                        } else if (streq(optarg, "pick")) {
                                /* pick: User namespacing on, UID range is picked randomly */
                                args->arg_userns_mode = USER_NAMESPACE_PICK;
                                args->arg_uid_shift = UID_INVALID;
                                args->arg_uid_range = UINT32_C(0x10000);
                        } else {
                                _cleanup_free_ char *buffer = NULL;
                                const char *range, *shift;

                                /* anything else: User namespacing on, UID range is explicitly configured */

                                range = strchr(optarg, ':');
                                if (range) {
                                        buffer = strndup(optarg, range - optarg);
                                        if (!buffer)
                                                return log_oom();
                                        shift = buffer;

                                        range++;
                                        r = safe_atou32(range, &args->arg_uid_range);
                                        if (r < 0)
                                                return log_error_errno(r, "Failed to parse UID range \"%s\": %m", range);
                                } else
                                        shift = optarg;

                                r = parse_uid(shift, &args->arg_uid_shift);
                                if (r < 0)
                                        return log_error_errno(r, "Failed to parse UID \"%s\": %m", optarg);

                                args->arg_userns_mode = USER_NAMESPACE_FIXED;
                        }

                        if (args->arg_uid_range <= 0) {
                                log_error("UID range cannot be 0.");
                                return -EINVAL;
                        }

                        args->arg_settings_mask |= SETTING_USERNS;
                        break;
                }

                case 'U':
                        if (userns_supported()) {
                                args->arg_userns_mode = USER_NAMESPACE_PICK;
                                args->arg_uid_shift = UID_INVALID;
                                args->arg_uid_range = UINT32_C(0x10000);

                                args->arg_settings_mask |= SETTING_USERNS;
                        }

                        break;

                case ARG_PRIVATE_USERS_CHOWN:
                        args->arg_userns_chown = true;

                        args->arg_settings_mask |= SETTING_USERNS;
                        break;

                case ARG_KILL_SIGNAL:
                        args->arg_kill_signal = signal_from_string_try_harder(optarg);
                        if (args->arg_kill_signal < 0) {
                                log_error("Cannot parse signal: %s", optarg);
                                return -EINVAL;
                        }

                        args->arg_settings_mask |= SETTING_KILL_SIGNAL;
                        break;

                case ARG_SETTINGS:

                        /* no               → do not read files
                         * yes              → read files, do not override cmdline, trust only subset
                         * override         → read files, override cmdline, trust only subset
                         * trusted          → read files, do not override cmdline, trust all
                         */

                        r = parse_boolean(optarg);
                        if (r < 0) {
                                if (streq(optarg, "trusted")) {
                                        mask_all_settings = false;
                                        mask_no_settings = false;
                                        args->arg_settings_trusted = true;

                                } else if (streq(optarg, "override")) {
                                        mask_all_settings = false;
                                        mask_no_settings = true;
                                        args->arg_settings_trusted = -1;
                                } else
                                        return log_error_errno(r, "Failed to parse --settings= argument: %s", optarg);
                        } else if (r > 0) {
                                /* yes */
                                mask_all_settings = false;
                                mask_no_settings = false;
                                args->arg_settings_trusted = -1;
                        } else {
                                /* no */
                                mask_all_settings = true;
                                mask_no_settings = false;
                                args->arg_settings_trusted = false;
                        }

                        break;

                case ARG_CHDIR:
                        if (!path_is_absolute(optarg)) {
                                log_error("Working directory %s is not an absolute path.", optarg);
                                return -EINVAL;
                        }

                        r = free_and_strdup(&args->arg_chdir, optarg);
                        if (r < 0)
                                return log_oom();

                        args->arg_settings_mask |= SETTING_WORKING_DIRECTORY;
                        break;

                case ARG_NOTIFY_READY:
                        r = parse_boolean(optarg);
                        if (r < 0) {
                                log_error("%s is not a valid notify mode. Valid modes are: yes, no, and ready.", optarg);
                                return -EINVAL;
                        }
                        args->arg_notify_ready = r;
                        args->arg_settings_mask |= SETTING_NOTIFY_READY;
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

        if (!(args->arg_clone_ns_flags & CLONE_NEWPID) ||
            !(args->arg_clone_ns_flags & CLONE_NEWUTS)) {
                args->arg_register = false;
                if (args->arg_start_mode != START_PID1) {
                        log_error("--boot cannot be used without namespacing.");
                        return -EINVAL;
                }
        }

        if (args->arg_userns_mode == USER_NAMESPACE_PICK)
                args->arg_userns_chown = true;

        if (args->arg_keep_unit && cg_pid_get_owner_uid(0, NULL) >= 0) {
                log_error("--keep-unit may not be used when invoked from a user session.");
                return -EINVAL;
        }

        if (args->arg_directory && args->arg_image) {
                log_error("--directory= and --image= may not be combined.");
                return -EINVAL;
        }

        if (args->arg_template && args->arg_image) {
                log_error("--template= and --image= may not be combined.");
                return -EINVAL;
        }

        if (args->arg_template && !(args->arg_directory || args->arg_machine)) {
                log_error("--template= needs --directory= or --machine=.");
                return -EINVAL;
        }

        if (args->arg_ephemeral && args->arg_template) {
                log_error("--ephemeral and --template= may not be combined.");
                return -EINVAL;
        }

        if (args->arg_ephemeral && args->arg_image) {
                log_error("--ephemeral and --image= may not be combined.");
                return -EINVAL;
        }

        if (args->arg_ephemeral && !IN_SET(args->arg_link_journal, LINK_NO, LINK_AUTO)) {
                log_error("--ephemeral and --link-journal= may not be combined.");
                return -EINVAL;
        }

        if (args->arg_userns_mode != USER_NAMESPACE_NO && !userns_supported()) {
                log_error("--private-users= is not supported, kernel compiled without user namespace support.");
                return -EOPNOTSUPP;
        }

        if (args->arg_userns_chown && args->arg_read_only) {
                log_error("--read-only and --private-users-chown may not be combined.");
                return -EINVAL;
        }

        if (args->arg_network_bridge && args->arg_network_zone) {
                log_error("--network-bridge= and --network-zone= may not be combined.");
                return -EINVAL;
        }

        if (argc > optind) {
                args->arg_parameters = strv_copy(argv + optind);
                if (!args->arg_parameters)
                        return log_oom();

                args->arg_settings_mask |= SETTING_START_MODE;
        }

        /* Load all settings from .nspawn files */
        if (mask_no_settings)
                args->arg_settings_mask = 0;

        /* Don't load any settings from .nspawn files */
        if (mask_all_settings)
                args->arg_settings_mask = _SETTINGS_MASK_ALL;

        args->arg_caps_retain = (args->arg_caps_retain | plus | (args->arg_private_network ? 1ULL << CAP_NET_ADMIN : 0)) & ~minus;

        e = getenv("SYSTEMD_NSPAWN_CONTAINER_SERVICE");
        if (e)
                args->arg_container_service_name = e;

        r = getenv_bool("SYSTEMD_NSPAWN_USE_CGNS");
        if (r < 0)
                args->arg_use_cgns = cg_ns_supported();
        else
                args->arg_use_cgns = r;

        return 1;
}

int verify_arguments(void) {

        if (args->arg_volatile_mode != VOLATILE_NO && args->arg_read_only) {
                log_error("Cannot combine --read-only with --volatile. Note that --volatile already implies a read-only base hierarchy.");
                return -EINVAL;
        }

        if (args->arg_expose_ports && !args->arg_private_network) {
                log_error("Cannot use --port= without private networking.");
                return -EINVAL;
        }

#ifndef HAVE_LIBIPTC
        if (args->arg_expose_ports) {
                log_error("--port= is not supported, compiled without libiptc support.");
                return -EOPNOTSUPP;
        }
#endif

        if (args->arg_start_mode == START_BOOT && args->arg_kill_signal <= 0)
                args->arg_kill_signal = SIGRTMIN+3;

        return 0;
}

int load_settings(void) {
        _cleanup_(settings_freep) Settings *settings = NULL;
        _cleanup_fclose_ FILE *f = NULL;
        _cleanup_free_ char *p = NULL;
        const char *fn, *i;
        int r;

        /* If all settings are masked, there's no point in looking for
         * the settings file */
        if ((args->arg_settings_mask & _SETTINGS_MASK_ALL) == _SETTINGS_MASK_ALL)
                return 0;

        fn = strjoina(args->arg_machine, ".nspawn");

        /* We first look in the admin's directories in /etc and /run */
        FOREACH_STRING(i, "/etc/systemd/nspawn", "/run/systemd/nspawn") {
                _cleanup_free_ char *j = NULL;

                j = strjoin(i, "/", fn, NULL);
                if (!j)
                        return log_oom();

                f = fopen(j, "re");
                if (f) {
                        p = j;
                        j = NULL;

                        /* By default, we trust configuration from /etc and /run */
                        if (args->arg_settings_trusted < 0)
                                args->arg_settings_trusted = true;

                        break;
                }

                if (errno != ENOENT)
                        return log_error_errno(errno, "Failed to open %s: %m", j);
        }

        if (!f) {
                /* After that, let's look for a file next to the
                 * actual image we shall boot. */

                if (args->arg_image) {
                        p = file_in_same_dir(args->arg_image, fn);
                        if (!p)
                                return log_oom();
                } else if (args->arg_directory) {
                        p = file_in_same_dir(args->arg_directory, fn);
                        if (!p)
                                return log_oom();
                }

                if (p) {
                        f = fopen(p, "re");
                        if (!f && errno != ENOENT)
                                return log_error_errno(errno, "Failed to open %s: %m", p);

                        /* By default, we do not trust configuration from /var/lib/machines */
                        if (args->arg_settings_trusted < 0)
                                args->arg_settings_trusted = false;
                }
        }

        if (!f)
                return 0;

        log_debug("Settings are trusted: %s", yes_no(args->arg_settings_trusted));

        r = settings_load(f, p, &settings);
        if (r < 0)
                return r;

        /* Copy over bits from the settings, unless they have been
         * explicitly masked by command line switches. */

        if ((args->arg_settings_mask & SETTING_START_MODE) == 0 &&
            settings->start_mode >= 0) {
                args->arg_start_mode = settings->start_mode;

                strv_free(args->arg_parameters);
                args->arg_parameters = settings->parameters;
                settings->parameters = NULL;
        }

        if ((args->arg_settings_mask & SETTING_WORKING_DIRECTORY) == 0 &&
            settings->working_directory) {
                free(args->arg_chdir);
                args->arg_chdir = settings->working_directory;
                settings->working_directory = NULL;
        }

        if ((args->arg_settings_mask & SETTING_ENVIRONMENT) == 0 &&
            settings->environment) {
                strv_free(args->arg_setenv);
                args->arg_setenv = settings->environment;
                settings->environment = NULL;
        }

        if ((args->arg_settings_mask & SETTING_USER) == 0 &&
            settings->user) {
                free(args->arg_user);
                args->arg_user = settings->user;
                settings->user = NULL;
        }

        if ((args->arg_settings_mask & SETTING_CAPABILITY) == 0) {
                uint64_t plus;

                plus = settings->capability;
                if (settings_private_network(settings))
                        plus |= (1ULL << CAP_NET_ADMIN);

                if (!args->arg_settings_trusted && plus != 0) {
                        if (settings->capability != 0)
                                log_warning("Ignoring Capability= setting, file %s is not trusted.", p);
                } else
                        args->arg_caps_retain |= plus;

                args->arg_caps_retain &= ~settings->drop_capability;
        }

        if ((args->arg_settings_mask & SETTING_KILL_SIGNAL) == 0 &&
            settings->kill_signal > 0)
                args->arg_kill_signal = settings->kill_signal;

        if ((args->arg_settings_mask & SETTING_PERSONALITY) == 0 &&
            settings->personality != PERSONALITY_INVALID)
                args->arg_personality = settings->personality;

        if ((args->arg_settings_mask & SETTING_MACHINE_ID) == 0 &&
            !sd_id128_is_null(settings->machine_id)) {

                if (!args->arg_settings_trusted)
                        log_warning("Ignoring MachineID= setting, file %s is not trusted.", p);
                else
                        args->arg_uuid = settings->machine_id;
        }

        if ((args->arg_settings_mask & SETTING_READ_ONLY) == 0 &&
            settings->read_only >= 0)
                args->arg_read_only = settings->read_only;

        if ((args->arg_settings_mask & SETTING_VOLATILE_MODE) == 0 &&
            settings->volatile_mode != _VOLATILE_MODE_INVALID)
                args->arg_volatile_mode = settings->volatile_mode;

        if ((args->arg_settings_mask & SETTING_CUSTOM_MOUNTS) == 0 &&
            settings->n_custom_mounts > 0) {

                if (!args->arg_settings_trusted)
                        log_warning("Ignoring TemporaryFileSystem=, Bind= and BindReadOnly= settings, file %s is not trusted.", p);
                else {
                        custom_mount_free_all(args->arg_custom_mounts, args->arg_n_custom_mounts);
                        args->arg_custom_mounts = settings->custom_mounts;
                        args->arg_n_custom_mounts = settings->n_custom_mounts;

                        settings->custom_mounts = NULL;
                        settings->n_custom_mounts = 0;
                }
        }

        if ((args->arg_settings_mask & SETTING_NETWORK) == 0 &&
            (settings->private_network >= 0 ||
             settings->network_veth >= 0 ||
             settings->network_bridge ||
             settings->network_zone ||
             settings->network_interfaces ||
             settings->network_macvlan ||
             settings->network_ipvlan ||
             settings->network_veth_extra)) {

                if (!args->arg_settings_trusted)
                        log_warning("Ignoring network settings, file %s is not trusted.", p);
                else {
                        args->arg_network_veth = settings_network_veth(settings);
                        args->arg_private_network = settings_private_network(settings);

                        strv_free(args->arg_network_interfaces);
                        args->arg_network_interfaces = settings->network_interfaces;
                        settings->network_interfaces = NULL;

                        strv_free(args->arg_network_macvlan);
                        args->arg_network_macvlan = settings->network_macvlan;
                        settings->network_macvlan = NULL;

                        strv_free(args->arg_network_ipvlan);
                        args->arg_network_ipvlan = settings->network_ipvlan;
                        settings->network_ipvlan = NULL;

                        strv_free(args->arg_network_veth_extra);
                        args->arg_network_veth_extra = settings->network_veth_extra;
                        settings->network_veth_extra = NULL;

                        free(args->arg_network_bridge);
                        args->arg_network_bridge = settings->network_bridge;
                        settings->network_bridge = NULL;

                        free(args->arg_network_zone);
                        args->arg_network_zone = settings->network_zone;
                        settings->network_zone = NULL;
                }
        }

        if ((args->arg_settings_mask & SETTING_EXPOSE_PORTS) == 0 &&
            settings->expose_ports) {

                if (!args->arg_settings_trusted)
                        log_warning("Ignoring Port= setting, file %s is not trusted.", p);
                else {
                        expose_port_free_all(args->arg_expose_ports);
                        args->arg_expose_ports = settings->expose_ports;
                        settings->expose_ports = NULL;
                }
        }

        if ((args->arg_settings_mask & SETTING_USERNS) == 0 &&
            settings->userns_mode != _USER_NAMESPACE_MODE_INVALID) {

                if (!args->arg_settings_trusted)
                        log_warning("Ignoring PrivateUsers= and PrivateUsersChown= settings, file %s is not trusted.", p);
                else {
                        args->arg_userns_mode = settings->userns_mode;
                        args->arg_uid_shift = settings->uid_shift;
                        args->arg_uid_range = settings->uid_range;
                        args->arg_userns_chown = settings->userns_chown;
                }
        }

        if ((args->arg_settings_mask & SETTING_NOTIFY_READY) == 0)
                args->arg_notify_ready = settings->notify_ready;

        return 0;
}

int determine_names(void) {
        int r;

        if (args->arg_template && !args->arg_directory && args->arg_machine) {

                /* If --template= was specified then we should not
                 * search for a machine, but instead create a new one
                 * in /var/lib/machine. */

                args->arg_directory = strjoin("/var/lib/machines/", args->arg_machine, NULL);
                if (!args->arg_directory)
                        return log_oom();
        }

        if (!args->arg_image && !args->arg_directory) {
                if (args->arg_machine) {
                        _cleanup_(image_unrefp) Image *i = NULL;

                        r = image_find(args->arg_machine, &i);
                        if (r < 0)
                                return log_error_errno(r, "Failed to find image for machine '%s': %m", args->arg_machine);
                        else if (r == 0) {
                                log_error("No image for machine '%s': %m", args->arg_machine);
                                return -ENOENT;
                        }

                        if (i->type == IMAGE_RAW)
                                r = free_and_strdup(&args->arg_image, i->path);
                        else
                                r = free_and_strdup(&args->arg_directory, i->path);
                        if (r < 0)
                                return log_error_errno(r, "Invalid image directory: %m");

                        if (!args->arg_ephemeral)
                                args->arg_read_only = args->arg_read_only || i->read_only;
                } else
                        args->arg_directory = get_current_dir_name();

                if (!args->arg_directory && !args->arg_machine) {
                        log_error("Failed to determine path, please use -D or -i.");
                        return -EINVAL;
                }
        }

        if (!args->arg_machine) {
                if (args->arg_directory && path_equal(args->arg_directory, "/"))
                        args->arg_machine = gethostname_malloc();
                else
                        args->arg_machine = strdup(basename(args->arg_image ?: args->arg_directory));

                if (!args->arg_machine)
                        return log_oom();

                hostname_cleanup(args->arg_machine);
                if (!machine_name_is_valid(args->arg_machine)) {
                        log_error("Failed to determine machine name automatically, please use -M.");
                        return -EINVAL;
                }

                if (args->arg_ephemeral) {
                        char *b;

                        /* Add a random suffix when this is an
                         * ephemeral machine, so that we can run many
                         * instances at once without manually having
                         * to specify -M each time. */

                        if (asprintf(&b, "%s-%016" PRIx64, args->arg_machine, random_u64()) < 0)
                                return log_oom();

                        free(args->arg_machine);
                        args->arg_machine = b;
                }
        }

        return 0;
}

int setup_machine_id(const char *directory) {
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

                if (sd_id128_is_null(args->arg_uuid)) {
                        r = sd_id128_randomize(&args->arg_uuid);
                        if (r < 0)
                                return log_error_errno(r, "Failed to acquire randomized machine UUID: %m");
                }
        } else {
                if (sd_id128_is_null(id)) {
                        log_error("Machine ID in container image is zero, refusing.");
                        return -EINVAL;
                }

                args->arg_uuid = id;
        }

        return 0;
}

int determine_uid_shift(const char *directory) {
        int r;

        if (args->arg_userns_mode == USER_NAMESPACE_NO) {
                args->arg_uid_shift = 0;
                return 0;
        }

        if (args->arg_uid_shift == UID_INVALID) {
                struct stat st;

                r = stat(directory, &st);
                if (r < 0)
                        return log_error_errno(errno, "Failed to determine UID base of %s: %m", directory);

                args->arg_uid_shift = st.st_uid & UINT32_C(0xffff0000);

                if (args->arg_uid_shift != (st.st_gid & UINT32_C(0xffff0000))) {
                        log_error("UID and GID base of %s don't match.", directory);
                        return -EINVAL;
                }

                args->arg_uid_range = UINT32_C(0x10000);
        }

        if (args->arg_uid_shift > (uid_t) -1 - args->arg_uid_range) {
                log_error("UID base too high for UID range.");
                return -EINVAL;
        }

        return 0;
}

int negotiate_uid_outer_child(int fd) {
        ssize_t l;

        if (args->arg_userns_mode == USER_NAMESPACE_NO)
                return 0;
                        
        /* Let the parent know which UID shift we read from the image */
        l = send(fd, &args->arg_uid_shift, sizeof(args->arg_uid_shift), MSG_NOSIGNAL);
        if (l < 0)
                return log_error_errno(errno, "Failed to send UID shift: %m");
        if (l != sizeof(args->arg_uid_shift)) {
                log_error("Short write while sending UID shift.");
                return -EIO;
        }

        if (args->arg_userns_mode == USER_NAMESPACE_PICK) {
                /* When we are supposed to pick the UID shift, the parent will check now whether the UID shift
                 * we just read from the image is available. If yes, it will send the UID shift back to us, if
                 * not it will pick a different one, and send it back to us. */

                l = recv(fd, &args->arg_uid_shift, sizeof(args->arg_uid_shift), 0);
                if (l < 0)
                        return log_error_errno(errno, "Failed to recv UID shift: %m");
                if (l != sizeof(args->arg_uid_shift)) {
                        log_error("Short read while receiving UID shift.");
                        return -EIO;
                }
        }

        log_info("Selected user namespace base " UID_FMT " and range " UID_FMT ".", args->arg_uid_shift, args->arg_uid_range);
        return 0;
}

static int uid_shift_pick(uid_t *shift, LockFile *ret_lock_file) {
        unsigned n_tries = 100;
        uid_t candidate;
        int r;

        assert(shift);
        assert(ret_lock_file);
        assert(args->arg_userns_mode == USER_NAMESPACE_PICK);
        assert(args->arg_uid_range == 0x10000U);

        candidate = *shift;

        (void) mkdir("/run/systemd/nspawn-uid", 0755);

        for (;;) {
                char lock_path[strlen("/run/systemd/nspawn-uid/") + DECIMAL_STR_MAX(uid_t) + 1];
                _cleanup_release_lock_file_ LockFile lf = LOCK_FILE_INIT;

                if (--n_tries <= 0)
                        return -EBUSY;

                if (candidate < UID_SHIFT_PICK_MIN || candidate > UID_SHIFT_PICK_MAX)
                        goto next;
                if ((candidate & UINT32_C(0xFFFF)) != 0)
                        goto next;

                xsprintf(lock_path, "/run/systemd/nspawn-uid/" UID_FMT, candidate);
                r = make_lock_file(lock_path, LOCK_EX|LOCK_NB, &lf);
                if (r == -EBUSY) /* Range already taken by another nspawn instance */
                        goto next;
                if (r < 0)
                        return r;

                /* Make some superficial checks whether the range is currently known in the user database */
                if (getpwuid(candidate))
                        goto next;
                if (getpwuid(candidate + UINT32_C(0xFFFE)))
                        goto next;
                if (getgrgid(candidate))
                        goto next;
                if (getgrgid(candidate + UINT32_C(0xFFFE)))
                        goto next;

                *ret_lock_file = lf;
                lf = (struct LockFile) LOCK_FILE_INIT;
                *shift = candidate;
                return 0;

        next:
                random_bytes(&candidate, sizeof(candidate));
                candidate = (candidate % (UID_SHIFT_PICK_MAX - UID_SHIFT_PICK_MIN)) + UID_SHIFT_PICK_MIN;
                candidate &= (uid_t) UINT32_C(0xFFFF0000);
        }
}

int negotiate_uid_parent(int fd, LockFile *ret_lock_file) {
        int r;
        ssize_t l;
        
        if (args->arg_userns_mode == USER_NAMESPACE_NO)
                return 0;
        
        /* The child just let us know the UID shift it might have read from the image. */
        l = recv(fd, &args->arg_uid_shift, sizeof args->arg_uid_shift, 0);
        if (l < 0)
                return log_error_errno(errno, "Failed to read UID shift: %m");

        if (l != sizeof args->arg_uid_shift) {
                log_error("Short read while reading UID shift.");
                return -EIO;
        }

        if (args->arg_userns_mode == USER_NAMESPACE_PICK) {
                /* If we are supposed to pick the UID shift, let's try to use the shift read from the
                 * image, but if that's already in use, pick a new one, and report back to the child,
                 * which one we now picked. */

                r = uid_shift_pick(&args->arg_uid_shift, ret_lock_file);
                if (r < 0)
                        return log_error_errno(r, "Failed to pick suitable UID/GID range: %m");

                l = send(fd, &args->arg_uid_shift, sizeof args->arg_uid_shift, MSG_NOSIGNAL);
                if (l < 0)
                        return log_error_errno(errno, "Failed to send UID shift: %m");
                if (l != sizeof args->arg_uid_shift) {
                        log_error("Short write while writing UID shift.");
                        return -EIO;
                }
        }

        return 0;
}

int negotiate_uuid_outer_child(int fd) {
        ssize_t l;
        
        l = send(fd, &args->arg_uuid, sizeof(args->arg_uuid), MSG_NOSIGNAL);
        if (l < 0)
                return log_error_errno(errno, "Failed to send machine ID: %m");
        if (l != sizeof(args->arg_uuid)) {
                log_error("Short write while sending machine ID.");
                return -EIO;
        }

        return 0;
}

int negotiate_uuid_parent(int fd) {
        ssize_t l;

        l = recv(fd, &args->arg_uuid, sizeof args->arg_uuid, 0);
        if (l < 0)
                return log_error_errno(errno, "Failed to read container machine ID: %m");
        if (l != sizeof(args->arg_uuid)) {
                log_error("Short read while reading container machined ID.");
                return -EIO;
        }

        return 0;
}

int lock_tree_ephemeral(LockFile *ret_global_lock, LockFile *ret_local_lock) {
        int r;
        _cleanup_free_ char *np = NULL;

        /* If the specified path is a mount point we
         * generate the new snapshot immediately
         * inside it under a random name. However if
         * the specified is not a mount point we
         * create the new snapshot in the parent
         * directory, just next to it. */
        r = path_is_mount_point(args->arg_directory, 0);
        if (r < 0)
                return log_error_errno(r, "Failed to determine whether directory %s is mount point: %m", args->arg_directory);
        if (r > 0)
                r = tempfn_random_child(args->arg_directory, "machine.", &np);
        else
                r = tempfn_random(args->arg_directory, "machine.", &np);
        if (r < 0)
                return log_error_errno(r, "Failed to generate name for snapshot: %m");

        r = image_path_lock(np, (args->arg_read_only ? LOCK_SH : LOCK_EX) | LOCK_NB, ret_global_lock, ret_local_lock);
        if (r < 0)
                return log_error_errno(r, "Failed to lock %s: %m", np);

        r = btrfs_subvol_snapshot(args->arg_directory, np, (args->arg_read_only ? BTRFS_SNAPSHOT_READ_ONLY : 0) | BTRFS_SNAPSHOT_FALLBACK_COPY | BTRFS_SNAPSHOT_RECURSIVE | BTRFS_SNAPSHOT_QUOTA);
        if (r < 0)
                return log_error_errno(r, "Failed to create snapshot %s from %s: %m", np, args->arg_directory);

        free(args->arg_directory);
        args->arg_directory = np;
        np = NULL;

        return 0;
}

int lock_tree_plain(LockFile *ret_global_lock, LockFile *ret_local_lock) {
        int r;

        r = image_path_lock(args->arg_directory, (args->arg_read_only ? LOCK_SH : LOCK_EX) | LOCK_NB, ret_global_lock, ret_local_lock);
        if (r == -EBUSY)
                return log_error_errno(r, "Directory tree %s is currently busy.", args->arg_directory);
        if (r < 0)
                return log_error_errno(r, "Failed to lock %s: %m", args->arg_directory);

        return 0;
}

int lock_tree_image(LockFile *ret_global_lock, LockFile *ret_local_lock) {
        int r;
        char template[] = "/tmp/nspawn-root-XXXXXX";

        r = image_path_lock(args->arg_image, (args->arg_read_only ? LOCK_SH : LOCK_EX) | LOCK_NB, ret_global_lock, ret_local_lock);
        if (r == -EBUSY)
                return log_error_errno(r, "Disk image %s is currently busy.", args->arg_image);
        if (r < 0)
                return log_error_errno(r, "Failed to create image lock: %m");

        if (!mkdtemp(template)) {
                log_error_errno(errno, "Failed to create temporary directory: %m");
                return -errno;
        }

        args->arg_directory = strdup(template);
        if (!args->arg_directory)
                return log_oom();

        return 0;
}

void args_free(void) {
        free(args->arg_directory);
        free(args->arg_template);
        free(args->arg_image);
        free(args->arg_machine);
        free(args->arg_user);
        free(args->arg_chdir);
        strv_free(args->arg_setenv);
        free(args->arg_network_bridge);
        strv_free(args->arg_network_interfaces);
        strv_free(args->arg_network_macvlan);
        strv_free(args->arg_network_ipvlan);
        strv_free(args->arg_network_veth_extra);
        strv_free(args->arg_parameters);
        custom_mount_free_all(args->arg_custom_mounts, args->arg_n_custom_mounts);
        expose_port_free_all(args->arg_expose_ports);
}
