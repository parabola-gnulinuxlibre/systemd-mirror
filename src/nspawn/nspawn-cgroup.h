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

#include "cgroup-util.h"

typedef struct CGMount CGMount;
typedef struct CGMounts {
        CGMount *mounts;
        size_t n;
} CGMounts;

int cgroup_setup(pid_t pid, CGroupUnified outer_cgver, CGroupUnified inner_cgver, uid_t uid_shift);
int cgroup_decide_mounts(CGMounts *ret_mounts, CGroupUnified outer_cgver, CGroupUnified inner_cgver, bool use_cgns);
int cgroup_mount_mounts(CGMounts mounts, bool use_cgns, uid_t uid_shift, const char *selinux_apifs_context);
void cgroup_free_mounts(CGMounts *mounts);

int mount_systemd_cgroup_writable(const char *dest, CGroupUnified inner_cgver);
