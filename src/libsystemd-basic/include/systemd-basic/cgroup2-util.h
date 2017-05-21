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

#include "macro.h"

/* generic types ****************************************************/

typedef struct CGroupHierarchy CGroupHierarchy;

typedef struct CGroup {
        CGroupHierarchy *hierarchy;
        char *path;
} CGroup;

static inline void cg2_freep(CGroup *cgroup) {
        free(cgroup->path);
}

static inline void cg2_free_freep(CGroup **cgroupp) {
        if (*cgroupp) {
                cg2_freep(*cgroupp);
                free(*cgroupp);
        }
}

#define _cleanup_cgroupfree_ _cleanup_(cg2_freep)
#define _cleanup_cgroupfree_free_ _cleanup_(cg2_free_freep)

/* generic functions ************************************************/

int cg2_flush(void);
bool cg2_ns_supported(void);

int cg2_get_v1_hier(const char *controller, CGroupHierarchy **ret_hier);
int cg2_get_v2_hier(CGroupHierarchy **ret_hier);
int cg2_hier_get_version(CGroupHierarchy *hier);
char *cg2_hier_get_str(CGroupHierarchy *hier);

int cg2_pid_get_cgroups_real(pid_t pid, /* CGroupHierarchy *hier, CGroup *ret_cgroup */...) _sentinel_;
#define cg2_pid_get_cgroups(pid, ...) cg2_pid_get_cgroups_real((pid), __VA_ARGS__, NULL)

char *cg2_cgroup_get_filepath(CGroup cgroup);
char *cg2_cgroup_get_str(CGroup cgroup);

/* systemd types ****************************************************/

typedef struct SdCGroup {
        CGroup prefix;
        char *path;
} SdCGroup;

static inline void cg2_sd_freep(SdCGroup *cgroup) {
        cg2_freep(&cgroup->prefix);
        free(cgroup->path);
}

static inline void cg2_sd_free_freep(SdCGroup **cgroupp) {
        if (*cgroupp) {
                cg2_sd_freep(*cgroupp);
                free(*cgroupp);
        }
}

#define _cleanup_sdcgroupfree_ _cleanup_(cg2_sd_freep)
#define _cleanup_sdcgroupfree_free_ _cleanup_(cg2_sd_free_freep)

typedef enum SdCGroupVersion {
        CGROUP_VER_UNKNOWN = 0,
        CGROUP_VER_1 = 1,
        CGROUP_VER_2 = 2,           /* added in systemd 230 */
        CGROUP_VER_MIXED_SD232 = 3, /* added in systemd 232 */
        CGROUP_VER_MIXED_SD233 = 4, /* added in systemd 233 */
} SdCGroupVersion;

/* systemd functions ************************************************/

int cg2_sd_flush(void);
int cg2_sd_get_version(SdCGroupVersion *ret_ver);
int cg2_sd_get_root(CGroup *ret_root);

int cg2_sd_ver_get_hier_ver(SdCGroupVersion ver);

int cg2_sd_pid_get_cgroup(pid_t pid, SdCGroup *ret_cgroup);

int cg2_sd_cgroup_parse(SdCGroup cgroup, char **ret_slice, char **ret_unit, SdCGroup *ret_extra);
int cg2_sd_cgroup_get_owner_uid(SdCGroup cgroup, uid_t *ret_uid);

char *cg2_sd_cgroup_get_filepath(SdCGroup sdcgroup);
char *cg2_sd_cgroup_get_cgpath(SdCGroup sdcgroup);
char *cg2_sd_cgroup_get_str(SdCGroup sdcgroup);
