#pragma once

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

#include <stdbool.h>
#include <sys/types.h>

#include "systemd-basic/macro.h"

#include "nspawn-types.h"

CGroupMode cgroup_outerver(void);
int cgroup_setup(pid_t pid, CGroupMode outer_cgver, CGroupMode inner_cgver, uid_t uid_shift, bool keep_unit);
int cgroup_decide_mounts(CGMounts *ret_mounts, CGroupMode outer_cgver, CGroupMode inner_cgver, bool use_cgns);
int cgroup_mount_mounts(CGMounts mounts, bool use_cgns, bool use_userns, const char *selinux_apifs_context);
void cgroup_free_mounts(CGMounts *mounts);
