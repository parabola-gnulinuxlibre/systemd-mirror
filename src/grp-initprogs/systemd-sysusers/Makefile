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

ifneq ($(ENABLE_SYSUSERS),)
systemd_sysusers_SOURCES = \
	src/sysusers/sysusers.c

systemd_sysusers_LDADD = \
	libsystemd-shared.la

rootbin_PROGRAMS += \
	systemd-sysusers

nodist_systemunit_DATA += \
	units/systemd-sysusers.service

SYSINIT_TARGET_WANTS += \
	systemd-sysusers.service

nodist_sysusers_DATA = \
	sysusers.d/systemd.conf \
	sysusers.d/basic.conf

INSTALL_DIRS += \
	$(sysusersdir)
endif # ENABLE_SYSUSERS

EXTRA_DIST += \
	units/systemd-sysusers.service.in \
	sysusers.d/systemd.conf.m4 \
	sysusers.d/systemd-remote.conf.m4 \
	sysusers.d/basic.conf.in

include $(topsrcdir)/build-aux/Makefile.tail.mk
