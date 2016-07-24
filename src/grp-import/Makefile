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

ifneq ($(ENABLE_IMPORTD),)

ifneq ($(HAVE_LIBCURL),)
ifneq ($(HAVE_XZ),)
ifneq ($(HAVE_ZLIB),)
ifneq ($(HAVE_BZIP2),)
ifneq ($(HAVE_GCRYPT),)

dist_rootlibexec_DATA = \
	src/import/import-pubring.gpg

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

manual_tests += \
	test-qcow2

test_qcow2_SOURCES = \
	src/import/test-qcow2.c \
	src/import/qcow2-util.c \
	src/import/qcow2-util.h

test_qcow2_CFLAGS = \
	$(AM_CFLAGS) \
	$(ZLIB_CFLAGS)

test_qcow2_LDADD = \
	libshared.la \
	$(ZLIB_LIBS)

endif # HAVE_GCRYPT
endif # HAVE_BZIP2
endif # HAVE_ZLIB
endif # HAVE_XZ
endif # HAVE_LIBCURL

endif # ENABLE_IMPORTD

polkitpolicy_in_files += \
	src/import/org.freedesktop.import1.policy.in

EXTRA_DIST += \
	units/systemd-importd.service.in


include $(topsrcdir)/build-aux/Makefile.tail.mk
