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
include $(dir $(lastword $(MAKEFILE_LIST)))/../../../config.mk
include $(topsrcdir)/build-aux/Makefile.head.mk

systemd_machined_SOURCES = \
	src/machine/machined.c \
	src/machine/machined.h

systemd_machined_LDADD = \
	libmachine-core.la

rootlibexec_PROGRAMS += \
	systemd-machined

libmachine_core_la_SOURCES = \
	src/machine/machine.c \
	src/machine/machine.h \
	src/machine/machined-dbus.c \
	src/machine/machine-dbus.c \
	src/machine/machine-dbus.h \
	src/machine/image-dbus.c \
	src/machine/image-dbus.h \
	src/machine/operation.c \
	src/machine/operation.h

libmachine_core_la_LIBADD = \
	libshared.la

noinst_LTLIBRARIES += \
	libmachine-core.la

test_machine_tables_SOURCES = \
	src/machine/test-machine-tables.c

test_machine_tables_LDADD = \
	libmachine-core.la

tests += \
	test-machine-tables

nodist_systemunit_DATA += \
	units/systemd-machined.service

dist_systemunit_DATA += \
	units/machine.slice

dist_systemunit_DATA_busnames += \
	units/org.freedesktop.machine1.busname

dist_dbussystemservice_DATA += \
	src/machine/org.freedesktop.machine1.service

dist_dbuspolicy_DATA += \
	src/machine/org.freedesktop.machine1.conf

polkitpolicy_files += \
	src/machine/org.freedesktop.machine1.policy

SYSTEM_UNIT_ALIASES += \
	systemd-machined.service dbus-org.freedesktop.machine1.service

BUSNAMES_TARGET_WANTS += \
	org.freedesktop.machine1.busname

polkitpolicy_in_files += \
	src/machine/org.freedesktop.machine1.policy.in

EXTRA_DIST += \
	units/systemd-machined.service.in

include $(topsrcdir)/build-aux/Makefile.tail.mk
