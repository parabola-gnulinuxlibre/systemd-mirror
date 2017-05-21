typedef enum LinkJournal {
        LINK_NO,
        LINK_AUTO,
        LINK_HOST,
        LINK_GUEST
} LinkJournal;

static char *arg_directory = NULL;
static char *arg_template = NULL;
static char *arg_chdir = NULL;
static char *arg_user = NULL;
static sd_id128_t arg_uuid = {};
static char *arg_machine = NULL;
static const char *arg_selinux_context = NULL;
static const char *arg_selinux_apifs_context = NULL;
static const char *arg_slice = NULL;
static bool arg_private_network = false;
static bool arg_read_only = false;
static StartMode arg_start_mode = START_PID1;
static bool arg_ephemeral = false;
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
static bool arg_register = true;
static bool arg_keep_unit = false;
static char **arg_network_interfaces = NULL;
static char **arg_network_macvlan = NULL;
static char **arg_network_ipvlan = NULL;
static bool arg_network_veth = false;
static char **arg_network_veth_extra = NULL;
static char *arg_network_bridge = NULL;
static char *arg_network_zone = NULL;
static unsigned long arg_personality = PERSONALITY_INVALID;
static char *arg_image = NULL;
static VolatileMode arg_volatile_mode = VOLATILE_NO;
static ExposePort *arg_expose_ports = NULL;
static char **arg_property = NULL;
static UserNamespaceMode arg_userns_mode = USER_NAMESPACE_NO;
static uid_t arg_uid_shift = UID_INVALID, arg_uid_range = 0x10000U;
static bool arg_userns_chown = false;
static int arg_kill_signal = 0;
static CGroupUnified arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_UNKNOWN;
static SettingsMask arg_settings_mask = 0;
static int arg_settings_trusted = -1;
static char **arg_parameters = NULL;
static const char *arg_container_service_name = "systemd-nspawn";
static bool arg_notify_ready = false;
static bool arg_use_cgns = true;
static unsigned long arg_clone_ns_flags = CLONE_NEWIPC|CLONE_NEWPID|CLONE_NEWUTS;

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

static int custom_mounts_prepare(void) {
        unsigned i;
        int r;

        /* Ensure the mounts are applied prefix first. */
        qsort_safe(arg_custom_mounts, arg_n_custom_mounts, sizeof(CustomMount), custom_mount_compare);

        /* Allocate working directories for the overlay file systems that need it */
        for (i = 0; i < arg_n_custom_mounts; i++) {
                CustomMount *m = &arg_custom_mounts[i];

                if (path_equal(m->destination, "/") && arg_userns_mode != USER_NAMESPACE_NO) {

                        if (arg_userns_chown) {
                                log_error("--private-users-chown may not be combined with custom root mounts.");
                                return -EINVAL;
                        } else if (arg_uid_shift == UID_INVALID) {
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

static int detect_unified_cgroup_hierarchy(const char *directory) {
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
                        arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_ALL;
                else
                        arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_NONE;

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
                arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_NONE;
                break;
        case CGROUP_UNIFIED_ALL: /* cgroup v2 */
                /* Unified cgroup hierarchy support was added in 230. Unfortunately libsystemd-shared,
                 * which we use to sniff the systemd version, was only added in 231, so we'll have a
                 * false negative here for 230. */
                r = systemd_installation_has_version(directory, 230);
                if (r < 0)
                        return log_error_errno(r, "Failed to decide cgroup version to use: Failed to determine systemd version in container: %m");
                if (r > 0)
                        arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_ALL;
                else
                        arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_NONE;
                break;
        case CGROUP_UNIFIED_SYSTEMD: /* cgroup v1 & v2 mixed; but v2 for systemd */
                /* Mixed cgroup hierarchy support was added in 232 */
                r = systemd_installation_has_version(directory, 232);
                if (r < 0)
                        return log_error_errno(r, "Failed to decide cgroup version to use: Failed to determine systemd version in container: %m");
                if (r > 0)
                        arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_SYSTEMD;
                else
                        arg_unified_cgroup_hierarchy = CGROUP_UNIFIED_NONE;
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
        arg_clone_ns_flags = (arg_clone_ns_flags & ~ns_flag) | (r > 0 ? 0 : ns_flag);
}

static int parse_argv(int argc, char *argv[]) {

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
                        r = parse_path_argument_and_warn(optarg, false, &arg_directory);
                        if (r < 0)
                                return r;
                        break;

                case ARG_TEMPLATE:
                        r = parse_path_argument_and_warn(optarg, false, &arg_template);
                        if (r < 0)
                                return r;
                        break;

                case 'i':
                        r = parse_path_argument_and_warn(optarg, false, &arg_image);
                        if (r < 0)
                                return r;
                        break;

                case 'x':
                        arg_ephemeral = true;
                        break;

                case 'u':
                        r = free_and_strdup(&arg_user, optarg);
                        if (r < 0)
                                return log_oom();

                        arg_settings_mask |= SETTING_USER;
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

                        free(arg_network_zone);
                        arg_network_zone = j;

                        arg_network_veth = true;
                        arg_private_network = true;
                        arg_settings_mask |= SETTING_NETWORK;
                        break;
                }

                case ARG_NETWORK_BRIDGE:

                        if (!ifname_valid(optarg)) {
                                log_error("Bridge interface name not valid: %s", optarg);
                                return -EINVAL;
                        }

                        r = free_and_strdup(&arg_network_bridge, optarg);
                        if (r < 0)
                                return log_oom();

                        /* fall through */

                case 'n':
                        arg_network_veth = true;
                        arg_private_network = true;
                        arg_settings_mask |= SETTING_NETWORK;
                        break;

                case ARG_NETWORK_VETH_EXTRA:
                        r = veth_extra_parse(&arg_network_veth_extra, optarg);
                        if (r < 0)
                                return log_error_errno(r, "Failed to parse --network-veth-extra= parameter: %s", optarg);

                        arg_private_network = true;
                        arg_settings_mask |= SETTING_NETWORK;
                        break;

                case ARG_NETWORK_INTERFACE:

                        if (!ifname_valid(optarg)) {
                                log_error("Network interface name not valid: %s", optarg);
                                return -EINVAL;
                        }

                        if (strv_extend(&arg_network_interfaces, optarg) < 0)
                                return log_oom();

                        arg_private_network = true;
                        arg_settings_mask |= SETTING_NETWORK;
                        break;

                case ARG_NETWORK_MACVLAN:

                        if (!ifname_valid(optarg)) {
                                log_error("MACVLAN network interface name not valid: %s", optarg);
                                return -EINVAL;
                        }

                        if (strv_extend(&arg_network_macvlan, optarg) < 0)
                                return log_oom();

                        arg_private_network = true;
                        arg_settings_mask |= SETTING_NETWORK;
                        break;

                case ARG_NETWORK_IPVLAN:

                        if (!ifname_valid(optarg)) {
                                log_error("IPVLAN network interface name not valid: %s", optarg);
                                return -EINVAL;
                        }

                        if (strv_extend(&arg_network_ipvlan, optarg) < 0)
                                return log_oom();

                        /* fall through */

                case ARG_PRIVATE_NETWORK:
                        arg_private_network = true;
                        arg_settings_mask |= SETTING_NETWORK;
                        break;

                case 'b':
                        if (arg_start_mode == START_PID2) {
                                log_error("--boot and --as-pid2 may not be combined.");
                                return -EINVAL;
                        }

                        arg_start_mode = START_BOOT;
                        arg_settings_mask |= SETTING_START_MODE;
                        break;

                case 'a':
                        if (arg_start_mode == START_BOOT) {
                                log_error("--boot and --as-pid2 may not be combined.");
                                return -EINVAL;
                        }

                        arg_start_mode = START_PID2;
                        arg_settings_mask |= SETTING_START_MODE;
                        break;

                case ARG_UUID:
                        r = sd_id128_from_string(optarg, &arg_uuid);
                        if (r < 0)
                                return log_error_errno(r, "Invalid UUID: %s", optarg);

                        if (sd_id128_is_null(arg_uuid)) {
                                log_error("Machine UUID may not be all zeroes.");
                                return -EINVAL;
                        }

                        arg_settings_mask |= SETTING_MACHINE_ID;
                        break;

                case 'S':
                        arg_slice = optarg;
                        break;

                case 'M':
                        if (isempty(optarg))
                                arg_machine = mfree(arg_machine);
                        else {
                                if (!machine_name_is_valid(optarg)) {
                                        log_error("Invalid machine name: %s", optarg);
                                        return -EINVAL;
                                }

                                r = free_and_strdup(&arg_machine, optarg);
                                if (r < 0)
                                        return log_oom();

                                break;
                        }

                case 'Z':
                        arg_selinux_context = optarg;
                        break;

                case 'L':
                        arg_selinux_apifs_context = optarg;
                        break;

                case ARG_READ_ONLY:
                        arg_read_only = true;
                        arg_settings_mask |= SETTING_READ_ONLY;
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

                        arg_settings_mask |= SETTING_CAPABILITY;
                        break;
                }

                case 'j':
                        arg_link_journal = LINK_GUEST;
                        arg_link_journal_try = true;
                        break;

                case ARG_LINK_JOURNAL:
                        if (streq(optarg, "auto")) {
                                arg_link_journal = LINK_AUTO;
                                arg_link_journal_try = false;
                        } else if (streq(optarg, "no")) {
                                arg_link_journal = LINK_NO;
                                arg_link_journal_try = false;
                        } else if (streq(optarg, "guest")) {
                                arg_link_journal = LINK_GUEST;
                                arg_link_journal_try = false;
                        } else if (streq(optarg, "host")) {
                                arg_link_journal = LINK_HOST;
                                arg_link_journal_try = false;
                        } else if (streq(optarg, "try-guest")) {
                                arg_link_journal = LINK_GUEST;
                                arg_link_journal_try = true;
                        } else if (streq(optarg, "try-host")) {
                                arg_link_journal = LINK_HOST;
                                arg_link_journal_try = true;
                        } else {
                                log_error("Failed to parse link journal mode %s", optarg);
                                return -EINVAL;
                        }

                        break;

                case ARG_BIND:
                case ARG_BIND_RO:
                        r = bind_mount_parse(&arg_custom_mounts, &arg_n_custom_mounts, optarg, c == ARG_BIND_RO);
                        if (r < 0)
                                return log_error_errno(r, "Failed to parse --bind(-ro)= argument %s: %m", optarg);

                        arg_settings_mask |= SETTING_CUSTOM_MOUNTS;
                        break;

                case ARG_TMPFS:
                        r = tmpfs_mount_parse(&arg_custom_mounts, &arg_n_custom_mounts, optarg);
                        if (r < 0)
                                return log_error_errno(r, "Failed to parse --tmpfs= argument %s: %m", optarg);

                        arg_settings_mask |= SETTING_CUSTOM_MOUNTS;
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

                        m = custom_mount_add(&arg_custom_mounts, &arg_n_custom_mounts, CUSTOM_MOUNT_OVERLAY);
                        if (!m)
                                return log_oom();

                        m->destination = destination;
                        m->source = upper;
                        m->lower = lower;
                        m->read_only = c == ARG_OVERLAY_RO;

                        upper = destination = NULL;
                        lower = NULL;

                        arg_settings_mask |= SETTING_CUSTOM_MOUNTS;
                        break;
                }

                case 'E': {
                        char **n;

                        if (!env_assignment_is_valid(optarg)) {
                                log_error("Environment variable assignment '%s' is not valid.", optarg);
                                return -EINVAL;
                        }

                        n = strv_env_set(arg_setenv, optarg);
                        if (!n)
                                return log_oom();

                        strv_free(arg_setenv);
                        arg_setenv = n;

                        arg_settings_mask |= SETTING_ENVIRONMENT;
                        break;
                }

                case 'q':
                        arg_quiet = true;
                        break;

                case ARG_SHARE_SYSTEM:
                        /* We don't officially support this anymore, except for compat reasons. People should use the
                         * $SYSTEMD_NSPAWN_SHARE_* environment variables instead. */
                        arg_clone_ns_flags = 0;
                        break;

                case ARG_REGISTER:
                        r = parse_boolean(optarg);
                        if (r < 0) {
                                log_error("Failed to parse --register= argument: %s", optarg);
                                return r;
                        }

                        arg_register = r;
                        break;

                case ARG_KEEP_UNIT:
                        arg_keep_unit = true;
                        break;

                case ARG_PERSONALITY:

                        arg_personality = personality_from_string(optarg);
                        if (arg_personality == PERSONALITY_INVALID) {
                                log_error("Unknown or unsupported personality '%s'.", optarg);
                                return -EINVAL;
                        }

                        arg_settings_mask |= SETTING_PERSONALITY;
                        break;

                case ARG_VOLATILE:

                        if (!optarg)
                                arg_volatile_mode = VOLATILE_YES;
                        else {
                                VolatileMode m;

                                m = volatile_mode_from_string(optarg);
                                if (m < 0) {
                                        log_error("Failed to parse --volatile= argument: %s", optarg);
                                        return -EINVAL;
                                } else
                                        arg_volatile_mode = m;
                        }

                        arg_settings_mask |= SETTING_VOLATILE_MODE;
                        break;

                case 'p':
                        r = expose_port_parse(&arg_expose_ports, optarg);
                        if (r == -EEXIST)
                                return log_error_errno(r, "Duplicate port specification: %s", optarg);
                        if (r < 0)
                                return log_error_errno(r, "Failed to parse host port %s: %m", optarg);

                        arg_settings_mask |= SETTING_EXPOSE_PORTS;
                        break;

                case ARG_PROPERTY:
                        if (strv_extend(&arg_property, optarg) < 0)
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
                                arg_userns_mode = USER_NAMESPACE_NO;
                                arg_uid_shift = UID_INVALID;
                                arg_uid_range = UINT32_C(0x10000);
                        } else if (boolean == true) {
                                /* yes: User namespacing on, UID range is read from root dir */
                                arg_userns_mode = USER_NAMESPACE_FIXED;
                                arg_uid_shift = UID_INVALID;
                                arg_uid_range = UINT32_C(0x10000);
                        } else if (streq(optarg, "pick")) {
                                /* pick: User namespacing on, UID range is picked randomly */
                                arg_userns_mode = USER_NAMESPACE_PICK;
                                arg_uid_shift = UID_INVALID;
                                arg_uid_range = UINT32_C(0x10000);
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
                                        r = safe_atou32(range, &arg_uid_range);
                                        if (r < 0)
                                                return log_error_errno(r, "Failed to parse UID range \"%s\": %m", range);
                                } else
                                        shift = optarg;

                                r = parse_uid(shift, &arg_uid_shift);
                                if (r < 0)
                                        return log_error_errno(r, "Failed to parse UID \"%s\": %m", optarg);

                                arg_userns_mode = USER_NAMESPACE_FIXED;
                        }

                        if (arg_uid_range <= 0) {
                                log_error("UID range cannot be 0.");
                                return -EINVAL;
                        }

                        arg_settings_mask |= SETTING_USERNS;
                        break;
                }

                case 'U':
                        if (userns_supported()) {
                                arg_userns_mode = USER_NAMESPACE_PICK;
                                arg_uid_shift = UID_INVALID;
                                arg_uid_range = UINT32_C(0x10000);

                                arg_settings_mask |= SETTING_USERNS;
                        }

                        break;

                case ARG_PRIVATE_USERS_CHOWN:
                        arg_userns_chown = true;

                        arg_settings_mask |= SETTING_USERNS;
                        break;

                case ARG_KILL_SIGNAL:
                        arg_kill_signal = signal_from_string_try_harder(optarg);
                        if (arg_kill_signal < 0) {
                                log_error("Cannot parse signal: %s", optarg);
                                return -EINVAL;
                        }

                        arg_settings_mask |= SETTING_KILL_SIGNAL;
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
                                        arg_settings_trusted = true;

                                } else if (streq(optarg, "override")) {
                                        mask_all_settings = false;
                                        mask_no_settings = true;
                                        arg_settings_trusted = -1;
                                } else
                                        return log_error_errno(r, "Failed to parse --settings= argument: %s", optarg);
                        } else if (r > 0) {
                                /* yes */
                                mask_all_settings = false;
                                mask_no_settings = false;
                                arg_settings_trusted = -1;
                        } else {
                                /* no */
                                mask_all_settings = true;
                                mask_no_settings = false;
                                arg_settings_trusted = false;
                        }

                        break;

                case ARG_CHDIR:
                        if (!path_is_absolute(optarg)) {
                                log_error("Working directory %s is not an absolute path.", optarg);
                                return -EINVAL;
                        }

                        r = free_and_strdup(&arg_chdir, optarg);
                        if (r < 0)
                                return log_oom();

                        arg_settings_mask |= SETTING_WORKING_DIRECTORY;
                        break;

                case ARG_NOTIFY_READY:
                        r = parse_boolean(optarg);
                        if (r < 0) {
                                log_error("%s is not a valid notify mode. Valid modes are: yes, no, and ready.", optarg);
                                return -EINVAL;
                        }
                        arg_notify_ready = r;
                        arg_settings_mask |= SETTING_NOTIFY_READY;
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

        if (!(arg_clone_ns_flags & CLONE_NEWPID) ||
            !(arg_clone_ns_flags & CLONE_NEWUTS)) {
                arg_register = false;
                if (arg_start_mode != START_PID1) {
                        log_error("--boot cannot be used without namespacing.");
                        return -EINVAL;
                }
        }

        if (arg_userns_mode == USER_NAMESPACE_PICK)
                arg_userns_chown = true;

        if (arg_keep_unit && cg_pid_get_owner_uid(0, NULL) >= 0) {
                log_error("--keep-unit may not be used when invoked from a user session.");
                return -EINVAL;
        }

        if (arg_directory && arg_image) {
                log_error("--directory= and --image= may not be combined.");
                return -EINVAL;
        }

        if (arg_template && arg_image) {
                log_error("--template= and --image= may not be combined.");
                return -EINVAL;
        }

        if (arg_template && !(arg_directory || arg_machine)) {
                log_error("--template= needs --directory= or --machine=.");
                return -EINVAL;
        }

        if (arg_ephemeral && arg_template) {
                log_error("--ephemeral and --template= may not be combined.");
                return -EINVAL;
        }

        if (arg_ephemeral && arg_image) {
                log_error("--ephemeral and --image= may not be combined.");
                return -EINVAL;
        }

        if (arg_ephemeral && !IN_SET(arg_link_journal, LINK_NO, LINK_AUTO)) {
                log_error("--ephemeral and --link-journal= may not be combined.");
                return -EINVAL;
        }

        if (arg_userns_mode != USER_NAMESPACE_NO && !userns_supported()) {
                log_error("--private-users= is not supported, kernel compiled without user namespace support.");
                return -EOPNOTSUPP;
        }

        if (arg_userns_chown && arg_read_only) {
                log_error("--read-only and --private-users-chown may not be combined.");
                return -EINVAL;
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

        arg_caps_retain = (arg_caps_retain | plus | (arg_private_network ? 1ULL << CAP_NET_ADMIN : 0)) & ~minus;

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

        if (arg_expose_ports && !arg_private_network) {
                log_error("Cannot use --port= without private networking.");
                return -EINVAL;
        }

#ifndef HAVE_LIBIPTC
        if (arg_expose_ports) {
                log_error("--port= is not supported, compiled without libiptc support.");
                return -EOPNOTSUPP;
        }
#endif

        if (arg_start_mode == START_BOOT && arg_kill_signal <= 0)
                arg_kill_signal = SIGRTMIN+3;

        return 0;
}

static int load_settings(void) {
        _cleanup_(settings_freep) Settings *settings = NULL;
        _cleanup_fclose_ FILE *f = NULL;
        _cleanup_free_ char *p = NULL;
        const char *fn, *i;
        int r;

        /* If all settings are masked, there's no point in looking for
         * the settings file */
        if ((arg_settings_mask & _SETTINGS_MASK_ALL) == _SETTINGS_MASK_ALL)
                return 0;

        fn = strjoina(arg_machine, ".nspawn");

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
                        if (arg_settings_trusted < 0)
                                arg_settings_trusted = true;

                        break;
                }

                if (errno != ENOENT)
                        return log_error_errno(errno, "Failed to open %s: %m", j);
        }

        if (!f) {
                /* After that, let's look for a file next to the
                 * actual image we shall boot. */

                if (arg_image) {
                        p = file_in_same_dir(arg_image, fn);
                        if (!p)
                                return log_oom();
                } else if (arg_directory) {
                        p = file_in_same_dir(arg_directory, fn);
                        if (!p)
                                return log_oom();
                }

                if (p) {
                        f = fopen(p, "re");
                        if (!f && errno != ENOENT)
                                return log_error_errno(errno, "Failed to open %s: %m", p);

                        /* By default, we do not trust configuration from /var/lib/machines */
                        if (arg_settings_trusted < 0)
                                arg_settings_trusted = false;
                }
        }

        if (!f)
                return 0;

        log_debug("Settings are trusted: %s", yes_no(arg_settings_trusted));

        r = settings_load(f, p, &settings);
        if (r < 0)
                return r;

        /* Copy over bits from the settings, unless they have been
         * explicitly masked by command line switches. */

        if ((arg_settings_mask & SETTING_START_MODE) == 0 &&
            settings->start_mode >= 0) {
                arg_start_mode = settings->start_mode;

                strv_free(arg_parameters);
                arg_parameters = settings->parameters;
                settings->parameters = NULL;
        }

        if ((arg_settings_mask & SETTING_WORKING_DIRECTORY) == 0 &&
            settings->working_directory) {
                free(arg_chdir);
                arg_chdir = settings->working_directory;
                settings->working_directory = NULL;
        }

        if ((arg_settings_mask & SETTING_ENVIRONMENT) == 0 &&
            settings->environment) {
                strv_free(arg_setenv);
                arg_setenv = settings->environment;
                settings->environment = NULL;
        }

        if ((arg_settings_mask & SETTING_USER) == 0 &&
            settings->user) {
                free(arg_user);
                arg_user = settings->user;
                settings->user = NULL;
        }

        if ((arg_settings_mask & SETTING_CAPABILITY) == 0) {
                uint64_t plus;

                plus = settings->capability;
                if (settings_private_network(settings))
                        plus |= (1ULL << CAP_NET_ADMIN);

                if (!arg_settings_trusted && plus != 0) {
                        if (settings->capability != 0)
                                log_warning("Ignoring Capability= setting, file %s is not trusted.", p);
                } else
                        arg_caps_retain |= plus;

                arg_caps_retain &= ~settings->drop_capability;
        }

        if ((arg_settings_mask & SETTING_KILL_SIGNAL) == 0 &&
            settings->kill_signal > 0)
                arg_kill_signal = settings->kill_signal;

        if ((arg_settings_mask & SETTING_PERSONALITY) == 0 &&
            settings->personality != PERSONALITY_INVALID)
                arg_personality = settings->personality;

        if ((arg_settings_mask & SETTING_MACHINE_ID) == 0 &&
            !sd_id128_is_null(settings->machine_id)) {

                if (!arg_settings_trusted)
                        log_warning("Ignoring MachineID= setting, file %s is not trusted.", p);
                else
                        arg_uuid = settings->machine_id;
        }

        if ((arg_settings_mask & SETTING_READ_ONLY) == 0 &&
            settings->read_only >= 0)
                arg_read_only = settings->read_only;

        if ((arg_settings_mask & SETTING_VOLATILE_MODE) == 0 &&
            settings->volatile_mode != _VOLATILE_MODE_INVALID)
                arg_volatile_mode = settings->volatile_mode;

        if ((arg_settings_mask & SETTING_CUSTOM_MOUNTS) == 0 &&
            settings->n_custom_mounts > 0) {

                if (!arg_settings_trusted)
                        log_warning("Ignoring TemporaryFileSystem=, Bind= and BindReadOnly= settings, file %s is not trusted.", p);
                else {
                        custom_mount_free_all(arg_custom_mounts, arg_n_custom_mounts);
                        arg_custom_mounts = settings->custom_mounts;
                        arg_n_custom_mounts = settings->n_custom_mounts;

                        settings->custom_mounts = NULL;
                        settings->n_custom_mounts = 0;
                }
        }

        if ((arg_settings_mask & SETTING_NETWORK) == 0 &&
            (settings->private_network >= 0 ||
             settings->network_veth >= 0 ||
             settings->network_bridge ||
             settings->network_zone ||
             settings->network_interfaces ||
             settings->network_macvlan ||
             settings->network_ipvlan ||
             settings->network_veth_extra)) {

                if (!arg_settings_trusted)
                        log_warning("Ignoring network settings, file %s is not trusted.", p);
                else {
                        arg_network_veth = settings_network_veth(settings);
                        arg_private_network = settings_private_network(settings);

                        strv_free(arg_network_interfaces);
                        arg_network_interfaces = settings->network_interfaces;
                        settings->network_interfaces = NULL;

                        strv_free(arg_network_macvlan);
                        arg_network_macvlan = settings->network_macvlan;
                        settings->network_macvlan = NULL;

                        strv_free(arg_network_ipvlan);
                        arg_network_ipvlan = settings->network_ipvlan;
                        settings->network_ipvlan = NULL;

                        strv_free(arg_network_veth_extra);
                        arg_network_veth_extra = settings->network_veth_extra;
                        settings->network_veth_extra = NULL;

                        free(arg_network_bridge);
                        arg_network_bridge = settings->network_bridge;
                        settings->network_bridge = NULL;

                        free(arg_network_zone);
                        arg_network_zone = settings->network_zone;
                        settings->network_zone = NULL;
                }
        }

        if ((arg_settings_mask & SETTING_EXPOSE_PORTS) == 0 &&
            settings->expose_ports) {

                if (!arg_settings_trusted)
                        log_warning("Ignoring Port= setting, file %s is not trusted.", p);
                else {
                        expose_port_free_all(arg_expose_ports);
                        arg_expose_ports = settings->expose_ports;
                        settings->expose_ports = NULL;
                }
        }

        if ((arg_settings_mask & SETTING_USERNS) == 0 &&
            settings->userns_mode != _USER_NAMESPACE_MODE_INVALID) {

                if (!arg_settings_trusted)
                        log_warning("Ignoring PrivateUsers= and PrivateUsersChown= settings, file %s is not trusted.", p);
                else {
                        arg_userns_mode = settings->userns_mode;
                        arg_uid_shift = settings->uid_shift;
                        arg_uid_range = settings->uid_range;
                        arg_userns_chown = settings->userns_chown;
                }
        }

        if ((arg_settings_mask & SETTING_NOTIFY_READY) == 0)
                arg_notify_ready = settings->notify_ready;

        return 0;
}
