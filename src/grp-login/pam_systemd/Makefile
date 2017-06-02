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

ifneq ($(HAVE_PAM),)
pam_systemd_la_SOURCES = \
	src/login/pam_systemd.sym \
	src/login/pam_systemd.c

pam_systemd_la_CFLAGS = \
	$(PAM_CFLAGS)

pam_systemd_la_LDFLAGS = \
	-module \
	-export-dynamic \
	-avoid-version \
	-shared \
	-Wl,--version-script=$(srcdir)/pam_systemd.sym

pam_systemd_la_LIBADD = \
	libsystemd-shared.la \
	$(PAM_LIBS)

pamlib_LTLIBRARIES = \
	pam_systemd.la

ifneq ($(ENABLE_PAM_CONFIG),)
dist_pamconf_DATA = \
	src/login/systemd-user
endif # ENABLE_PAM_CONFIG

EXTRA_DIST += \
	src/login/systemd-user.m4
endif # HAVE_PAM

include $(topsrcdir)/build-aux/Makefile.tail.mk
