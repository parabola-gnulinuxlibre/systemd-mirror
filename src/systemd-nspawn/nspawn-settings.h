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

#include <stdio.h>

#include "systemd-basic/macro.h"

#include "nspawn-types.h"

typedef struct Settings {
        /* [Run] */
        StartMode start_mode;
        char **parameters;
        char **environment;
        char *user;
        uint64_t capability;
        uint64_t drop_capability;
        int kill_signal;
        unsigned long personality;
        sd_id128_t machine_id;
        char *working_directory;
        UserNamespaceMode userns_mode;
        uid_t uid_shift, uid_range;
        bool notify_ready;

        /* [Image] */
        int read_only;
        VolatileMode volatile_mode;
        CustomMount *custom_mounts;
        unsigned n_custom_mounts;
        int userns_chown;

        /* [Network] */
        int private_network;
        int network_veth;
        char *network_bridge;
        char *network_zone;
        char **network_interfaces;
        char **network_macvlan;
        char **network_ipvlan;
        char **network_veth_extra;
        ExposePort *expose_ports;
} Settings;

int settings_load(FILE *f, const char *path, Settings **ret);
Settings* settings_free(Settings *s);

bool settings_network_veth(Settings *s);
bool settings_private_network(Settings *s);

DEFINE_TRIVIAL_CLEANUP_FUNC(Settings*, settings_free);
