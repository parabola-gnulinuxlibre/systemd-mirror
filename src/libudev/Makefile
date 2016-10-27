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

LIBUDEV_CURRENT=7
LIBUDEV_REVISION=5
LIBUDEV_AGE=6

include_HEADERS += \
	src/libudev/libudev.h

rootlib_LTLIBRARIES += \
	libudev.la

libudev_la_LDFLAGS = \
	-version-info $(LIBUDEV_CURRENT):$(LIBUDEV_REVISION):$(LIBUDEV_AGE) \
	-Wl,--version-script=$(srcdir)/libudev.sym

libudev_la_LIBADD = \
	libsystemd-internal.la \
	libsystemd-basic.la

pkgconfiglib_DATA += \
	src/libudev/libudev.pc

EXTRA_DIST += \
	src/libudev/libudev.pc.in

test-libudev-sym.c: \
		src/libudev/libudev.sym \
		src/udev/udev.h
	$(generate-sym-test)

nodist_test_libudev_sym_SOURCES = \
	test-libudev-sym.c
test_libudev_sym_CFLAGS = \
	-Wno-deprecated-declarations
test_libudev_sym_LDADD = \
	libudev.la

nested.subdirs += src

include $(topsrcdir)/build-aux/Makefile.tail.mk
