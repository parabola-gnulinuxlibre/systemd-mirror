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

#include <errno.h>
#include <glob.h>

#include "systemd-basic/alloc-util.h"   /* realloc_multiply */
#include "systemd-basic/cgroup2-util.h"
#include "systemd-basic/fd-util.h"      /* _cleanp_fclose_ */
#include "systemd-basic/fileio.h"       /* FOREACH_LINE */
#include "systemd-basic/glob-util.h"    /* _cleanup_globfree_ */
#include "systemd-basic/parse-util.h"   /* safe_atoi */
#include "systemd-basic/process-util.h" /* procfs_file_alloca */
#include "systemd-basic/special.h"
#include "systemd-basic/stat-util.h"    /* F_TYPE_EQUAL */
#include "systemd-basic/string-util.h"  /* startswith, endswith, FOREACH_WORD_SEPARATOR */
#include "systemd-basic/strv.h"         /* STRV_FOREACH */
#include "systemd-basic/unit-name.h"    /* unit_name_is_valid */
#include "systemd-basic/user-util.h"    /* parse_uid */

static int hier_init_mountpoint(CGroupHierarchy *hier);
static void cg2_unescape(const char **p, size_t *n);
static bool valid_slice_name(const char *p, size_t n);

/* generic ***********************************************************/

struct CGroupHierarchy {
        int id;
        char *controllers;
        char *mountpoint;
};

static thread_local struct {
        bool initialized;
        size_t cap;
        CGroupHierarchy *list;
} cg2_cache = { 0 };

int cg2_flush(void) {
        cg2_cache.initialized = false;
        for (size_t i = 0; i < cg2_cache.cap; i++) {
                free(cg2_cache.list[i].controllers);
                free(cg2_cache.list[i].mountpoint);
        }
        free(cg2_cache.list);
        cg2_cache.list = NULL;
        cg2_cache.cap = 0;
        return cg2_sd_flush();
}

static int cg2_init(void) {
        _cleanup_fclose_ FILE *f = NULL;
        char line[LINE_MAX];

        if (cg2_cache.initialized)
                return 0;

        cg2_flush();

        f = fopen("/proc/self/cgroup", "re");
        if (!f) {
                /* turn "no such file" in to "no such process" */
                return errno == ENOENT ? -ESRCH : -errno;
        }

        FOREACH_LINE(line, f, return -errno) {
                int id, r;
                char *id_str, *controllers;
                char *rest = line;
                id_str = strsep(&rest, ":");
                controllers = strsep(&rest, ":");
                /*path =*/ strsep(&rest, "\n"); /* discard the path */
                if (!rest || rest[0] != '\0')
                        return -ENODATA;
                if (safe_atoi(id_str, &id) < 0)
                        return -ENODATA;
                if (id < 0)
                        return -ENODATA;
                if ( (id == 0) != (controllers[0] == '\0') )
                        return -ENODATA;

                if ((size_t)id >= cg2_cache.cap) {
                        size_t cap = id+1;
                        CGroupHierarchy *list = realloc_multiply(cg2_cache.list, sizeof(cg2_cache.list[0]), cap);
                        if (!list)
                                return -ENOMEM;
                        cg2_cache.list = list;
                        while (cg2_cache.cap < cap) {
                                list[cg2_cache.cap].id = -1;
                                list[cg2_cache.cap].controllers = NULL;
                                list[cg2_cache.cap].mountpoint = NULL;
                                cg2_cache.cap++;
                        }
                }

                cg2_cache.list[id].id = id;
                cg2_cache.list[id].controllers = strdup(controllers);
                if (!cg2_cache.list[id].controllers)
                        return -ENOMEM;
                r = hier_init_mountpoint(&cg2_cache.list[id]);
                if (r < 0)
                        return r;
        }
        return 0;
}

static int hier_init_mountpoint(CGroupHierarchy *hier) {
        assert(hier);

        if (hier->id == 0) {
                /* cgroup v2 hierarchy */
                _cleanup_globfree_ glob_t g = {};
                struct statfs fs;
                int r;
                char **tmp;

                /* first check "/sys/fs/cgroup/" */
                if (statfs("/sys/fs/cgroup/", &fs) < 0)
                        return -errno;
                if (F_TYPE_EQUAL(fs.f_type, CGROUP2_SUPER_MAGIC)) {
                        hier->mountpoint = strdup("/sys/fs/group");
                        if (!hier->mountpoint)
                                return -ENOMEM;
                        return 0;
                }

                /* then check "/sys/fs/cgroup/X/" */
                r = glob("/sys/fs/cgroup/*/", GLOB_ERR, NULL, &g);
                if (r == GLOB_NOMATCH)
                        return -ENOENT;
                if (r == GLOB_NOSPACE)
                        return -ENOMEM;
                if (r != 0)
                        return errno > 0 ? -errno : -EIO;
                STRV_FOREACH(tmp, g.gl_pathv) {
                        if (statfs(*tmp, &fs) < 0)
                                continue;
                        if (F_TYPE_EQUAL(fs.f_type, CGROUP2_SUPER_MAGIC)) {
                                hier->mountpoint = canonicalize_file_name(*tmp);
                                if (!hier->mountpoint)
                                        return -ENOMEM;
                                return 0;
                        }
                }
                return -ENOENT;
        } else {
                /* cgroup v1 hierarchy */
                char *controller, *tmp;

                controller = strdupa(hier->controllers);
                strchrnul(controller, ',')[0] = '\0';
                tmp = startswith(controller, "name=");
                if (tmp)
                        controller = tmp;

                hier->mountpoint = canonicalize_file_name(strjoina("/sys/fs/cgroup/", controller, NULL));
                if (!hier->mountpoint)
                        return -errno;
                return 0;
        }
}

bool cg2_ns_supported(void) {
        static thread_local int enabled = -1;

        if (enabled >= 0)
                return enabled;

        if (access("/proc/self/ns/cgroup", F_OK) == 0)
                enabled = 1;
        else
                enabled = 0;

        return enabled;
}

int cg2_get_v1_hier(const char *selector, CGroupHierarchy **ret_hier) {
        size_t selector_len;
        int r;

        assert(selector);

        r = cg2_init();
        if (r < 0)
                return r;

        selector_len = strlen(selector);
        for (int id = 0; (size_t)id < cg2_cache.cap; id++) {
                const char *controller, *state;
                size_t controller_len;
                if (cg2_cache.list[id].id != id)
                        continue;

                FOREACH_WORD_SEPARATOR(controller, controller_len, cg2_cache.list[id].controllers, ",", state) {
                        if (controller_len == selector_len && memcmp(controller, selector, selector_len) == 0) {
                                if (ret_hier)
                                        *ret_hier = &cg2_cache.list[id];
                                return 0;
                        }
                }
        }
        return -ENOENT;
}

int cg2_get_v2_hier(CGroupHierarchy **ret_hier) {
        int r;

        r = cg2_init();
        if (r < 0)
                return r;

        if (cg2_cache.cap < 1 || cg2_cache.list[0].id != 0)
                return -ENOENT;

        if (ret_hier)
                *ret_hier = &cg2_cache.list[0];
        return 0;
}


int cg2_hier_get_version(CGroupHierarchy *hier) {
        assert(hier);
        if (hier->id < 0) {
                return -EINVAL;
        } else if (hier->id == 0) {
                return 2;
        } else {
                return 1;
        }
}

char *cg2_hier_get_str(CGroupHierarchy *hier) {
        char *ret;

        assert(hier);
        assert(hier->controllers);

        if (asprintf(&ret, "%d:%s", hier->id, hier->controllers) < 0)
                return NULL;
        return ret;
}

int cg2_pid_get_cgroups_real(pid_t pid, ...) {
        const char *filename;
        _cleanup_fclose_ FILE *file = NULL;
        va_list ap;
        char line[LINE_MAX];
        int n, r;

        r = cg2_init();
        if (r < 0)
                return r;

        if (pid == 0)
                filename = "/proc/self/cgroup";
        else
                filename = procfs_file_alloca(pid, "cgroup");

        file = fopen(filename, "re");
        if (!file) {
                /* turn "no such file" in to "no such process" */
                return errno == ENOENT ? -ESRCH : -errno;
        }

        n = 0;
        FOREACH_LINE(line, file, return -errno) {
                CGroupHierarchy *hier;
                int id;
                char *id_str, *controllers, *path;
                char *rest = line;
                id_str = strsep(&rest, ":");
                controllers = strsep(&rest, ":");
                path = strsep(&rest, "\n");
                if (!rest || rest[0] != '\0')
                        continue;
                if (safe_atoi(id_str, &id) < 0)
                        continue;
                if ( (id == 0) != (controllers[0] == '\0') )
                        continue;

                va_start(ap, pid);
                while ((hier = va_arg(ap, CGroupHierarchy *))) {
                        CGroup *ret_cgroup = va_arg(ap, CGroup *);
                        if (id == hier->id) {
                                if (ret_cgroup) {
                                        ret_cgroup->hierarchy = hier;
                                        ret_cgroup->path = path;
                                }
                                n++;
                        }
                }
                va_end(ap);
        }
        return n;
}

char *cg2_cgroup_get_filepath(CGroup cgroup) {
        assert(cgroup.hierarchy);
        assert(cgroup.hierarchy->mountpoint);
        assert(cgroup.path);

        return strjoin(cgroup.hierarchy->mountpoint, cgroup.path, NULL);
}

char *cg2_cgroup_get_str(CGroup cgroup) {
        _cleanup_free_ char *hierstr;
        char *ret;

        hierstr = cg2_hier_get_str(cgroup.hierarchy);
        if (!hierstr)
                return NULL;

        if (asprintf(&ret, "%s:%s", hierstr, cgroup.path) < 0)
                return NULL;
        return ret;
}

/* systemd **********************************************************/

static thread_local struct {
        bool have_ver;
        SdCGroupVersion ver;
        bool have_hier;
        CGroupHierarchy *hier;
        bool have_root;
        CGroup *root;
} cg2_sd_cache = { 0 };

int cg2_sd_flush(void) {
        cg2_sd_cache.ver = CGROUP_VER_UNKNOWN;
        cg2_sd_cache.have_ver = false;

        cg2_sd_cache.hier = NULL;
        cg2_sd_cache.have_hier = false;

        cg2_free_freep(&cg2_sd_cache.root);
        cg2_sd_cache.root = NULL;
        cg2_sd_cache.have_root = false;

        return 0;
}

static int cg2_sd_init_version(void) {
        struct statfs fs;

        if (cg2_sd_cache.have_ver)
                return 0;

        if (statfs("/sys/fs/cgroup/", &fs) < 0)
                return -errno;
        if (F_TYPE_EQUAL(fs.f_type, CGROUP2_SUPER_MAGIC)) {
                cg2_sd_cache.ver = CGROUP_VER_2;
                cg2_sd_cache.have_ver = true;
                return 0;
        }

        if (statfs("/sys/fs/cgroup/unified/", &fs) == 0 &&
            F_TYPE_EQUAL(fs.f_type, CGROUP2_SUPER_MAGIC)) {
                cg2_sd_cache.ver = CGROUP_VER_MIXED_SD233;
                cg2_sd_cache.have_ver = true;
                return 0;
        }

        if (statfs("/sys/fs/cgroup/systemd/", &fs) < 0)
                return -errno;
        if (F_TYPE_EQUAL(fs.f_type, CGROUP2_SUPER_MAGIC)) {
                cg2_sd_cache.ver = CGROUP_VER_MIXED_SD232;
                cg2_sd_cache.have_ver = true;
                return 0;
        }
        if (F_TYPE_EQUAL(fs.f_type, CGROUP_SUPER_MAGIC)) {
                cg2_sd_cache.ver = CGROUP_VER_1;
                cg2_sd_cache.have_ver = true;
                return 0;
        }

        return -ENOMEDIUM;
}

static int cg2_sd_init_hier(void) {
        int r;

        if (cg2_sd_cache.have_hier)
                return 0;

        r = cg2_sd_init_version();
        if (r < 0)
                return r;

        switch (cg2_sd_cache.ver) {
        case CGROUP_VER_UNKNOWN:
                assert_not_reached("Unknown systemd cgroup version");
                break;
        case CGROUP_VER_1:
                r = cg2_get_v1_hier("name=systemd", &cg2_sd_cache.hier);
                if (r < 0)
                        return r;
                break;
        case CGROUP_VER_2:
        case CGROUP_VER_MIXED_SD232:
        case CGROUP_VER_MIXED_SD233:
                r = cg2_get_v2_hier(&cg2_sd_cache.hier);
                if (r < 0)
                        return r;
                break;
        }
        cg2_sd_cache.have_hier = true;
        return 0;
}

int cg2_sd_get_root(CGroup *ret_root) {
        CGroup cg;
        int r;
        char *e;

        r = cg2_sd_init_hier();
        if (r < 0)
                return r;

        r = cg2_pid_get_cgroups(1, cg2_sd_cache.hier, &cg);
        if (r < 0)
                return r;

        e = endswith(cg.path, "/" SPECIAL_INIT_SCOPE); /* "/init.scope" */
        if (!e)
                e = endswith(cg.path, "/" SPECIAL_SYSTEM_SLICE); /* "/system.slice" (legacy) */
        if (!e)
                e = endswith(cg.path, "/system"); /* (even more legacy) */
        if (e)
                *e = 0;

        if (ret_root)
                *ret_root = cg;
        return 0;
}

int cg2_sd_ver_get_hier_ver(SdCGroupVersion ver) {
        switch (ver) {
        default:
        case CGROUP_VER_UNKNOWN:
                return -EINVAL;
        case CGROUP_VER_1:
                return 1;
        case CGROUP_VER_2:
        case CGROUP_VER_MIXED_SD232:
        case CGROUP_VER_MIXED_SD233:
                return 2;
        }
}

int cg2_sd_pid_get_cgroup(pid_t pid, SdCGroup *ret_cgroup) {
        _cleanup_cgroupfree_ CGroup root, mine;
        int r;
        const char *p;

        r = cg2_sd_get_root(&root);
        if (r < 0)
                return r;

        r = cg2_pid_get_cgroups(pid, root.hierarchy, &mine);
        if (r < 0)
                return r;

        p = startswith(mine.path, root.path);
        if (!p)
                return -ENXIO;

        if (ret_cgroup) {
                char *prefix, *path;
                prefix = strdup(root.path);
                if (!prefix)
                        goto enomem;
                path = strdup(p);
                if (!path)
                        goto enomem;
                ret_cgroup->prefix.hierarchy = root.hierarchy;
                ret_cgroup->prefix.path = prefix;
                ret_cgroup->path = path;
                return 0;
        enomem:
                free(prefix);
                free(path);
                return -ENOMEM;
        }
        return 0;
}

int cg2_sd_cgroup_parse(SdCGroup cgroup, char **ret_slice, char **ret_unit, SdCGroup *ret_extra) {
        const char *rest, *slice, *unit, *prefix, *extra;
        size_t slice_len, unit_len, prefix_len, extra_len;
        char *hslice = NULL, *hunit = NULL;
        SdCGroup sextra;

        assert(cgroup.path);
        assert(cgroup.prefix.path);
        assert(cgroup.prefix.hierarchy);

        /* Given
         *   cgroup.path = "/foo.slice/bar.slice/baz.slice/unit.service/extra..."
         * we return
         *   *ret_slice = "baz.slice"
         *   *ret_unit = "unit.service"
         *   ret_extra->prefix.hierarchy = cgroup.prefix.hierarchy
         *   ret_extra->prefix.path = strjoin(cgroup.prefix.path, "/foo.slice/bar.slize/baz.slice/unit.service", NULL)
         *   ret_extra->path = "/extra..."
         *
         * The input path my contain 0 or more leading ".slice"
         * segments; we return the rightmost.  If there are no
         * ".slice" segments, we return SPECIAL_ROOT_SLICE
         * ("-.slice").
         */

        rest = cgroup.path;

        /* slice */
        slice = SPECIAL_ROOT_SLICE;
        slice_len = strlen(slice);
        for (;;) {
                const char *part, *tmprest;
                size_t part_len;

                /* trim leading "/"s */
                tmprest = rest + strspn(rest, "/");

                /* split off the first part */
                part = tmprest;
                part_len = strcspn(part, "/");
                tmprest += part_len;

                if (valid_slice_name(part, part_len)) {
                        /* accept this iteration */
                        slice = part;
                        slice_len = part_len;
                        rest = tmprest;
                } else {
                        /* reject this iteration; we have found the first
                         * non-slice segment. */
                        break;
                }
        }
        cg2_unescape(&slice, &slice_len);

        /* unit */
        rest += strspn(rest, "/");
        unit = rest;
        unit_len = strcspn(unit, "/");
        rest += unit_len;
        cg2_unescape(&unit, &unit_len);
        if (!unit_name_is_valid(strndupa(unit, unit_len), UNIT_NAME_PLAIN|UNIT_NAME_INSTANCE))
                return -ENXIO;

        /* extra */
        extra = rest;
        extra_len = strlen(rest);
        prefix = cgroup.path;
        prefix_len = extra - prefix;

        /* allocate return values */
        if (ret_slice) {
                hslice = strndup(slice, slice_len);
                if (!hslice)
                        goto enomem;
        }
        if (ret_unit) {
                hunit = strndup(unit, unit_len);
                if (!hslice)
                        goto enomem;
        }
        if (ret_extra) {
                sextra.prefix.hierarchy = cgroup.prefix.hierarchy;
                sextra.prefix.path = strndup(prefix, prefix_len);
                if (!sextra.prefix.path)
                        goto enomem;
                sextra.prefix.path = strjoin(cgroup.prefix.path, sextra.prefix.path, NULL);
                if (!sextra.prefix.path)
                        goto enomem;
                sextra.path = strndup(extra, extra_len);
                if (!sextra.path)
                        goto enomem;
        }

        /* return */
        if (ret_slice)
                *ret_slice = hslice;
        if (ret_unit)
                *ret_unit = hunit;
        if (ret_extra)
                *ret_extra = sextra;
        return 0;

 enomem:
        free(hslice);
        free(hunit);
        cg2_sd_freep(&sextra);
        return -ENOMEM;
}


int cg2_sd_cgroup_get_owner_uid(SdCGroup cgroup, uid_t *ret_uid) {
        _cleanup_free_ char *slice = NULL;
        char *start, *end;
        int r;

        r = cg2_sd_cgroup_parse(cgroup, &slice, NULL, NULL);
        if (r < 0)
                return r;

        start = startswith(slice, "user-");
        if (!start)
                return -ENXIO;
        end = endswith(start, ".slice");
        if (!end)
                return -ENXIO;

        *end = '\0';
        if (parse_uid(start, ret_uid) < 0)
                return -ENXIO;
        return 0;
}

static int cg2_sd_cgroup_get_cgroup(SdCGroup sdcgroup, CGroup *ret_cgroup) {
        assert(sdcgroup.prefix.path);
        assert(sdcgroup.path);

        if (ret_cgroup) {
                ret_cgroup->path = strjoin(sdcgroup.prefix.path, sdcgroup.path, NULL);
                if (!ret_cgroup->path)
                        return -ENOMEM;
                ret_cgroup->hierarchy = sdcgroup.prefix.hierarchy;
        }
        return 0;
}

char *cg2_sd_cgroup_get_filepath(SdCGroup sdcgroup) {
        _cleanup_cgroupfree_ CGroup cgroup;

        if (cg2_sd_cgroup_get_cgroup(sdcgroup, &cgroup) < 0)
                return NULL;

        return cg2_cgroup_get_filepath(cgroup);
}

char *cg2_sd_cgroup_get_cgpath(SdCGroup sdcgroup) {
        CGroup cgroup;

        if (cg2_sd_cgroup_get_cgroup(sdcgroup, &cgroup) < 0)
                return NULL;

        return cgroup.path;
}

char *cg2_sd_cgroup_get_str(SdCGroup sdcgroup) {
        _cleanup_cgroupfree_ CGroup cgroup;

        if (cg2_sd_cgroup_get_cgroup(sdcgroup, &cgroup) < 0)
                return NULL;

        return cg2_cgroup_get_str(cgroup);
}

/* basically copied from old cgroup-util ****************************/

static void cg2_unescape(const char **p, size_t *n) {
        size_t sn;

        assert(p);

        if (!n)
                n = &sn;

        /* The return value of this function (unlike cg_escape())
         * doesn't need free()! */

        if (*n >= 1 && (*p)[0] == '_') {
                (*p)++;
                (*n)--;
        }
}

static bool valid_slice_name(const char *p, size_t n) {

        if (!p)
                return false;

        if (n < strlen("x.slice"))
                return false;

        if (memcmp(p + n - 6, ".slice", 6) == 0) {
                const char *c = strndupa(p, n);
                cg2_unescape(&c, &n);
                return unit_name_is_valid(c, UNIT_NAME_PLAIN);
        }

        return false;
}
