/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <stdbool.h>
#include <sys/types.h>

#include "cgroup-util.h"

int cgroup_setup(pid_t pid, CGroupUnified outer_cgver, CGroupUnified inner_cgver, uid_t uid_shift, bool keep_unit);

int mount_cgroups(const char *dest, CGroupUnified outer_cgver, CGroupUnified inner_cgver, bool userns, uid_t uid_shift, uid_t uid_range, const char *selinux_apifs_context, bool use_cgns);
int mount_systemd_cgroup_writable(const char *dest, CGroupUnified inner_cgver);
