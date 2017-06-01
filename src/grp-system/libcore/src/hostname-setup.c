/***
  This file is part of systemd.

  Copyright 2010 Lennart Poettering

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
#include <stdio.h>
#include <stdlib.h>

#include "core/hostname-setup.h"
#include "systemd-basic/alloc-util.h"
#include "systemd-basic/fileio.h"
#include "systemd-basic/hostname-util.h"
#include "systemd-basic/log.h"
#include "systemd-basic/macro.h"
#include "systemd-basic/string-util.h"
#include "systemd-basic/util.h"

int hostname_setup(void) {
        int r;
        _cleanup_free_ char *b = NULL;
        const char *hn;
        bool enoent = false;

        r = read_hostname_config("/etc/hostname", &b);
        if (r < 0) {
                if (r == -ENOENT)
                        enoent = true;
                else
                        log_warning_errno(r, "Failed to read configured hostname: %m");

                hn = NULL;
        } else
                hn = b;

        if (isempty(hn)) {
                /* Don't override the hostname if it is already set
                 * and not explicitly configured */
                if (hostname_is_set())
                        return 0;

                if (enoent)
                        log_info("No hostname configured.");

                hn = "localhost";
        }

        r = sethostname_idempotent(hn);
        if (r < 0)
                return log_warning_errno(r, "Failed to set hostname to <%s>: %m", hn);

        log_info("Set hostname to <%s>.", hn);
        return 0;
}
