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
include $(dir $(lastword $(MAKEFILE_LIST)))../../config.mk
include $(topsrcdir)/build-aux/Makefile.head.mk

systemd_bus_proxyd_SOURCES = \
	src/bus-proxyd/bus-proxyd.c

systemd_bus_proxyd_LDADD = \
	libbus-proxy-core.la \
	libshared.la

nodist_systemunit_DATA += \
	units/systemd-bus-proxyd.service

dist_systemunit_DATA += \
	units/systemd-bus-proxyd.socket

nodist_userunit_DATA += \
	units/user/systemd-bus-proxyd.service

dist_userunit_DATA += \
	units/user/systemd-bus-proxyd.socket

EXTRA_DIST += \
	units/systemd-bus-proxyd.service.m4.in \
	units/user/systemd-bus-proxyd.service.in

ifneq ($(HAVE_SMACK),)
bus-proxyd-set-cap-hook:
	-$(SETCAP) cap_mac_admin+ei $(DESTDIR)$(libexecdir)/systemd-bus-proxyd

INSTALL_EXEC_HOOKS += bus-proxyd-set-cap-hook
endif # HAVE_SMACK

include $(topsrcdir)/build-aux/Makefile.tail.mk
