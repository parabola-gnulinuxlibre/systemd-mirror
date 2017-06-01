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

Args const *args_get(void);

/* parent:main() ****************************************************/
int parse_argv(int argc, char *argv[]);
/* geteuid() == 0 */
int determine_names(void);
int load_settings(void);
int verify_arguments(void);
int lock_tree_ephemeral(LockFile *ret_global_lock, LockFile *ret_local_lock);
int lock_tree_plain(LockFile *ret_global_lock, LockFile *ret_local_lock);
int lock_tree_image(LockFile *ret_global_lock, LockFile *ret_local_lock);
int custom_mounts_prepare(void);
int pick_cgroup_ver(const char *directory, CGroupMode *ret_cgver);
/* set up the PTY */
/* parent:run() *****************************************************/
/* ... */
/* if (raw_clone(UNSHARE_MOUNT) == 0) { exit(outer_child() == 0 ? EXIT_SUCCESS : EXIT_FAILURE); } */
int negotiate_uid_parent(int fd, LockFile *ret_lock_file);
/* wait_for_terminate_and_warn("namespace helper, *pid, NULL) */
/* recv(pid_socket_pair[0], pid, sizeof *pid, 0) */
int recv_uuid_parent(int fd);
/* ... */
/* parent:main() ****************************************************/
/* some more teardown */
void args_free(void);

/* outer_child() ****************************************************/
/* ... */
int determine_uid_shift(const char *directory);
int negotiate_uid_outer_child(int fd);
/* setup_volatile(...) */
/* setup_volatile_state(...) */
/* ... */
/* mount_all(in_userns=false, ...) */
/* ... */
int setup_machine_id(const char *directory);
/* ... */
int send_uuid_outer_child(int fd);
/* mount_custom(...) */
/* if (!args->arg_use_cgns) { mount_cgroups(...); } */
/* ... */
/* if (raw_clone() == 0) { exit(inner_child() == 0 ? EXIT_SUCCESS : EXIT_FAILURE); } */
/* ... */

/* inner_child() ****************************************************/
/*
   mount_all(in_userns=true, ...);
   mount_sysfs(NULL);
   if (args->use_cgns && cgns_supported())
        mount_cgroups(...);
   else
        mount_systemd_cgroup_writable(...);
   ...
*/
