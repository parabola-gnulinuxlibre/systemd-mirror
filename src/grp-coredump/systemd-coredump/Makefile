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

ifneq ($(ENABLE_COREDUMP),)
systemd_coredump_SOURCES = \
	src/coredump/coredump.c \
	src/coredump/coredump-vacuum.c \
	src/coredump/coredump-vacuum.h

systemd_coredump_CFLAGS = \
	$(ACL_CFLAGS)

systemd_coredump_LDADD = \
	libsystemd-shared.la \
	$(ACL_LIBS)

ifneq ($(HAVE_ELFUTILS),)
systemd_coredump_SOURCES += \
	src/coredump/stacktrace.c \
	src/coredump/stacktrace.h

systemd_coredump_LDADD += \
	$(ELFUTILS_LIBS)
endif # HAVE_ELFUTILS

nodist_systemunit_DATA += \
	units/systemd-coredump@.service

dist_systemunit_DATA += \
	units/systemd-coredump.socket

SOCKETS_TARGET_WANTS += \
	systemd-coredump.socket

rootlibexec_PROGRAMS += \
	systemd-coredump

dist_pkgsysconf_DATA += \
	src/coredump/coredump.conf

manual_tests += \
	test-coredump-vacuum

test_coredump_vacuum_SOURCES = \
	src/coredump/test-coredump-vacuum.c  \
	src/coredump/coredump-vacuum.c \
	src/coredump/coredump-vacuum.h

test_coredump_vacuum_LDADD = \
	libsystemd-shared.la

nodist_sysctl_DATA = \
	sysctl.d/50-coredump.conf

CLEANFILES += \
	sysctl.d/50-coredump.conf
endif # ENABLE_COREDUMP

EXTRA_DIST += \
	sysctl.d/50-coredump.conf.in \
	units/systemd-coredump@.service.in

include $(topsrcdir)/build-aux/Makefile.tail.mk
