/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <stdbool.h>
#include <sys/types.h>

#include "cgroup-util.h"

typedef struct CGMount CGMount;
typedef struct CGMounts {
        CGMount *mounts;
        size_t n;
} CGMounts;

int cgroup_setup(pid_t pid, CGroupUnified outer_cgver, CGroupUnified inner_cgver, uid_t uid_shift, bool keep_unit);
int cgroup_decide_mounts(CGMounts *ret_mounts, CGroupUnified outer_cgver, CGroupUnified inner_cgver, bool use_cgns);
int cgroup_mount_mounts(CGMounts mounts, FILE *cgfile, uid_t uid_shift, const char *selinux_apifs_context);
void cgroup_free_mounts(CGMounts *mounts);
