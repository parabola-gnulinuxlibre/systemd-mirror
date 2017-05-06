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

libnss_resolve_la_SOURCES = \
	src/nss-resolve/nss-resolve.sym \
	src/nss-resolve/nss-resolve.c

libnss_resolve_la_LDFLAGS = \
	-module \
	-export-dynamic \
	-avoid-version \
	-shared \
	-shrext .so.2 \
	-Wl,--version-script=$(srcdir)/nss-resolve.sym

libnss_resolve_la_LIBADD = \
	libsystemd-internal.la \
	libsystemd-basic.la \
        -ldl

rootlib_LTLIBRARIES += \
	libnss_resolve.la

include $(topsrcdir)/build-aux/Makefile.tail.mk
