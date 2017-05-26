#pragma once

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

#include <stdbool.h>                   /* bool */
#include <stdint.h>                    /* uint64_t */
#include <sys/types.h>                 /* uid_t */

#include <systemd/sd-id128.h>          /* sd_id128_t */

#include "nspawn-settings.h"           /* StartMode, UserNamespaceMode, SettingsMask */
#include "nspawn-mount.h"              /* CustomMount, VolatileMode */
#include "nspawn-expose-ports.h"       /* ExposePort */

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

Args const *get_args(void);

int parse_argv(int argc, char *argv[]);
int determine_names(void);
int load_settings(void);
int verify_arguments(void);

int detect_unified_cgroup_hierarchy(const char *directory);
int setup_machine_id(const char *directory);
int determine_uid_shift(const char *directory);
int custom_mounts_prepare(void);

int negotiate_uid_outer_child(int fd);
int negotiate_uid_parent(int fd, LockFile *ret_lock_file);

int negotiate_uuid_outer_child(int fd);
int negotiate_uuid_parent(int fd);

int lock_tree_ephemeral(LockFile *ret_global_lock, LockFile *ret_local_lock);
int lock_tree_plain(LockFile *ret_global_lock, LockFile *ret_local_lock);
int lock_tree_image(LockFile *ret_global_lock, LockFile *ret_local_lock);
