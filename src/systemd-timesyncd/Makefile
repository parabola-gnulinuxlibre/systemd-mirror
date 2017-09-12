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
include $(dir $(lastword $(MAKEFILE_LIST)))/../../config.mk
include $(topsrcdir)/build-aux/Makefile.head.mk

ifneq ($(ENABLE_TIMESYNCD),)
systemd_timesyncd_SOURCES = \
	src/timesync/timesyncd.c \
	src/timesync/timesyncd-manager.c \
	src/timesync/timesyncd-manager.h \
	src/timesync/timesyncd-conf.c \
	src/timesync/timesyncd-conf.h \
	src/timesync/timesyncd-server.c \
	src/timesync/timesyncd-server.h

nodist_systemd_timesyncd_SOURCES = \
	src/timesync/timesyncd-gperf.c

systemd_timesyncd_LDADD = \
	libsystemd-network.la \
	libsystemd-shared.la \
	-lm

rootlibexec_PROGRAMS += \
	systemd-timesyncd

nodist_systemunit_DATA += \
	units/systemd-timesyncd.service

GENERAL_ALIASES += \
	$(systemunitdir)/systemd-timesyncd.service $(pkgsysconfdir)/system/sysinit.target.wants/systemd-timesyncd.service

nodist_pkgsysconf_DATA += \
	src/timesync/timesyncd.conf

endif # ENABLE_TIMESYNCD

gperf_gperf_sources += \
	src/timesync/timesyncd-gperf.gperf

EXTRA_DIST += \
	units/systemd-timesyncd.service.in \
	src/timesync/timesyncd.conf.in

include $(topsrcdir)/build-aux/Makefile.tail.mk
