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

ifneq ($(HAVE_LIBIPTC),)
noinst_LTLIBRARIES += \
	libsystemd-firewall.la

libsystemd_firewall_la_SOURCES = \
	src/shared/firewall-util.h \
	src/shared/firewall-util.c

libsystemd_firewall_la_CFLAGS = \
	$(LIBIPTC_CFLAGS)

libsystemd_firewall_la_LIBADD = \
	$(LIBIPTC_LIBS)
endif # HAVE_LIBIPTC

include $(topsrcdir)/build-aux/Makefile.tail.mk
