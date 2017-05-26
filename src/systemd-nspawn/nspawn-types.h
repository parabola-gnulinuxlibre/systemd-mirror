#pragma once

/***
  This file is part of systemd.

  Copyright 2010, 2015 Lennart Poettering
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

#include <stdbool.h>          /* bool */
#include <stdint.h>           /* uint64_t */
#include <sys/types.h>        /* uid_t */

#include <systemd/sd-id128.h> /* sd_id128_t */

#include "systemd-basic/list.h"

typedef enum StartMode {
        START_PID1, /* Run parameters as command line as process 1 */
        START_PID2, /* Use stub init process as PID 1, run parameters as command line as process 2 */
        START_BOOT, /* Search for init system, pass arguments as parameters */
        _START_MODE_MAX,
        _START_MODE_INVALID = -1
} StartMode;

typedef enum UserNamespaceMode {
        USER_NAMESPACE_NO,
        USER_NAMESPACE_FIXED,
        USER_NAMESPACE_PICK,
        _USER_NAMESPACE_MODE_MAX,
        _USER_NAMESPACE_MODE_INVALID = -1,
} UserNamespaceMode;

typedef enum SettingsMask {
        SETTING_START_MODE        = 1 << 0,
        SETTING_ENVIRONMENT       = 1 << 1,
        SETTING_USER              = 1 << 2,
        SETTING_CAPABILITY        = 1 << 3,
        SETTING_KILL_SIGNAL       = 1 << 4,
        SETTING_PERSONALITY       = 1 << 5,
        SETTING_MACHINE_ID        = 1 << 6,
        SETTING_NETWORK           = 1 << 7,
        SETTING_EXPOSE_PORTS      = 1 << 8,
        SETTING_READ_ONLY         = 1 << 9,
        SETTING_VOLATILE_MODE     = 1 << 10,
        SETTING_CUSTOM_MOUNTS     = 1 << 11,
        SETTING_WORKING_DIRECTORY = 1 << 12,
        SETTING_USERNS            = 1 << 13,
        SETTING_NOTIFY_READY      = 1 << 14,
        _SETTINGS_MASK_ALL        = (1 << 15) -1
} SettingsMask;

typedef enum VolatileMode {
        VOLATILE_NO,
        VOLATILE_YES,
        VOLATILE_STATE,
        _VOLATILE_MODE_MAX,
        _VOLATILE_MODE_INVALID = -1
} VolatileMode;

typedef enum CustomMountType {
        CUSTOM_MOUNT_BIND,
        CUSTOM_MOUNT_TMPFS,
        CUSTOM_MOUNT_OVERLAY,
        _CUSTOM_MOUNT_TYPE_MAX,
        _CUSTOM_MOUNT_TYPE_INVALID = -1
} CustomMountType;

typedef struct CustomMount {
        CustomMountType type;
        bool read_only;
        char *source; /* for overlayfs this is the upper directory */
        char *destination;
        char *options;
        char *work_dir;
        char **lower;
} CustomMount;

typedef struct ExposePort {
        int protocol;
        uint16_t host_port;
        uint16_t container_port;
        LIST_FIELDS(struct ExposePort, ports);
} ExposePort;

typedef enum LinkJournal {
        LINK_NO,
        LINK_AUTO,
        LINK_HOST,
        LINK_GUEST
} LinkJournal;

typedef enum CGroupMode {
        CGROUP_VER_NONE,
        CGROUP_VER_INHERIT,
        CGROUP_VER_1_SD,
        CGROUP_VER_2,
        CGROUP_VER_MIXED_SD232,
} CGroupMode;

typedef struct Args {
        char *arg_directory;
        char *arg_template;
        char *arg_chdir;
        char *arg_user;
        sd_id128_t arg_uuid;
        char *arg_machine;
        const char *arg_selinux_context;
        const char *arg_selinux_apifs_context;
        const char *arg_slice;
        bool arg_private_network;
        bool arg_read_only;
        StartMode arg_start_mode;
        bool arg_ephemeral;
        LinkJournal arg_link_journal;
        bool arg_link_journal_try;
        uint64_t arg_caps_retain;
        CustomMount *arg_custom_mounts;
        unsigned arg_n_custom_mounts;
        char **arg_setenv;
        bool arg_quiet;
        bool arg_register;
        bool arg_keep_unit;
        char **arg_network_interfaces;
        char **arg_network_macvlan;
        char **arg_network_ipvlan;
        bool arg_network_veth;
        char **arg_network_veth_extra;
        char *arg_network_bridge;
        char *arg_network_zone;
        unsigned long arg_personality;
        char *arg_image;
        VolatileMode arg_volatile_mode;
        ExposePort *arg_expose_ports;
        char **arg_property;
        UserNamespaceMode arg_userns_mode;
        uid_t arg_uid_shift, arg_uid_range;
        bool arg_userns_chown;
        int arg_kill_signal;
        CGroupMode arg_unified_cgroup_hierarchy;
        SettingsMask arg_settings_mask;
        int arg_settings_trusted;
        char **arg_parameters;
        const char *arg_container_service_name;
        bool arg_notify_ready;
        bool arg_use_cgns;
        unsigned long arg_clone_ns_flags;
} Args;
