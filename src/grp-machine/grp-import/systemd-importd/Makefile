#  -*- Mode: makefile; indent-tabs-mode: t -*-
#
#  This file is part of systemd.
#
#  Copyright 2010-2012 Lennart Poettering
#  Copyright 2010-2012 Kay Sievers
#  Copyright 2013 Zbigniew Jędrzejewski-Szmek
#  Copyright 2013 David Strauss
#  Copyright 2016 Luke Shumaker
#
#  systemd is free software; you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation; either version 2.1 of the License, or
#  (at your option) any later version.
#
#  systemd is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with systemd; If not, see <http://www.gnu.org/licenses/>.
include $(dir $(lastword $(MAKEFILE_LIST)))/../../../../config.mk
include $(topsrcdir)/build-aux/Makefile.head.mk

rootlibexec_PROGRAMS += systemd-importd
systemd_importd_SOURCES = \
	src/import/importd.c

systemd_importd_CFLAGS = \
	-D SYSTEMD_PULL_PATH=\"$(rootlibexecdir)/systemd-pull\" \
	-D SYSTEMD_IMPORT_PATH=\"$(rootlibexecdir)/systemd-import\" \
	-D SYSTEMD_EXPORT_PATH=\"$(rootlibexecdir)/systemd-export\"

systemd_importd_LDADD = \
	libsystemd-shared.la

nodist_systemunit_DATA += \
	units/systemd-importd.service

dist_systemunit_DATA_busnames += \
	units/org.freedesktop.import1.busname

BUSNAMES_TARGET_WANTS += \
	org.freedesktop.import1.busname

SYSTEM_UNIT_ALIASES += \
	systemd-importd.service dbus-org.freedesktop.import1.service

dist_dbussystemservice_DATA += \
	src/import/org.freedesktop.import1.service

dist_dbuspolicy_DATA += \
	src/import/org.freedesktop.import1.conf

polkitpolicy_files += \
	src/import/org.freedesktop.import1.policy

polkitpolicy_in_files += \
	src/import/org.freedesktop.import1.policy.in

EXTRA_DIST += \
	units/systemd-importd.service.in

include $(topsrcdir)/build-aux/Makefile.tail.mk
