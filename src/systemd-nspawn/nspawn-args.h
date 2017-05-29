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

#include "systemd-basic/lockfile-util.h"

#include "nspawn-types.h"

Args const *get_args(void);

// parent:main()
int parse_argv(int argc, char *argv[]);
int determine_names(void);
int load_settings(void);
int verify_arguments(void);
int lock_tree_ephemeral(LockFile *ret_global_lock, LockFile *ret_local_lock);
int lock_tree_plain(LockFile *ret_global_lock, LockFile *ret_local_lock);
int lock_tree_image(LockFile *ret_global_lock, LockFile *ret_local_lock);
int custom_mounts_prepare(void);
// parent:run()
int negotiate_uid_parent(int fd, LockFile *ret_lock_file);
int negotiate_uuid_parent(int fd);
// parent:main()
void args_free(void);

// outer_child()
int determine_uid_shift(const char *directory);
int detect_unified_cgroup_hierarchy(const char *directory);
int negotiate_uid_outer_child(int fd);
int setup_machine_id(const char *directory);
int negotiate_uuid_outer_child(int fd);
