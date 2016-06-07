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

libexec_PROGRAMS += \
	systemd-importd \
	systemd-pull \
	systemd-import \
	systemd-export

systemd_importd_SOURCES = \
	src/import/importd.c

systemd_importd_CFLAGS = \
	$(AM_CFLAGS) \
	-D SYSTEMD_PULL_PATH=\"$(libexecdir)/systemd-pull\" \
	-D SYSTEMD_IMPORT_PATH=\"$(libexecdir)/systemd-import\" \
	-D SYSTEMD_EXPORT_PATH=\"$(libexecdir)/systemd-export\"

systemd_importd_LDADD = \
	libshared.la

systemd_pull_SOURCES = \
	src/import/pull.c \
	src/import/pull-raw.c \
	src/import/pull-raw.h \
	src/import/pull-tar.c \
	src/import/pull-tar.h \
	src/import/pull-job.c \
	src/import/pull-job.h \
	src/import/pull-common.c \
	src/import/pull-common.h \
	src/import/import-common.c \
	src/import/import-common.h \
	src/import/import-compress.c \
	src/import/import-compress.h \
	src/import/curl-util.c \
	src/import/curl-util.h \
	src/import/aufs-util.c \
	src/import/aufs-util.h \
	src/import/qcow2-util.c \
	src/import/qcow2-util.h

systemd_pull_CFLAGS = \
	$(AM_CFLAGS) \
	$(LIBCURL_CFLAGS) \
	$(XZ_CFLAGS) \
	$(ZLIB_CFLAGS) \
	$(BZIP2_CFLAGS) \
	$(GCRYPT_CFLAGS) \
	-D VENDOR_KEYRING_PATH=\"$(libexecdir)/import-pubring.gpg\" \
	-D USER_KEYRING_PATH=\"$(pkgsysconfdir)/import-pubring.gpg\"

systemd_pull_LDADD = \
	libshared.la \
	$(LIBCURL_LIBS) \
	$(XZ_LIBS) \
	$(ZLIB_LIBS) \
	$(BZIP2_LIBS) \
	$(GCRYPT_LIBS)

systemd_import_SOURCES = \
	src/import/import.c \
	src/import/import-raw.c \
	src/import/import-raw.h \
	src/import/import-tar.c \
	src/import/import-tar.h \
	src/import/import-common.c \
	src/import/import-common.h \
	src/import/import-compress.c \
	src/import/import-compress.h \
	src/import/qcow2-util.c \
	src/import/qcow2-util.h

systemd_import_CFLAGS = \
	$(AM_CFLAGS) \
	$(XZ_CFLAGS) \
	$(ZLIB_CFLAGS) \
	$(BZIP2_CFLAGS)

systemd_import_LDADD = \
	libshared.la \
	$(XZ_LIBS) \
	$(ZLIB_LIBS) \
	$(BZIP2_LIBS)

systemd_export_SOURCES = \
	src/import/export.c \
	src/import/export-tar.c \
	src/import/export-tar.h \
	src/import/export-raw.c \
	src/import/export-raw.h \
	src/import/import-common.c \
	src/import/import-common.h \
	src/import/import-compress.c \
	src/import/import-compress.h

systemd_export_CFLAGS = \
	$(AM_CFLAGS) \
	$(XZ_CFLAGS) \
	$(ZLIB_CFLAGS) \
	$(BZIP2_CFLAGS)

systemd_export_LDADD = \
	libshared.la \
	$(XZ_LIBS) \
	$(ZLIB_LIBS) \
	$(BZIP2_LIBS)

dist_libexec_DATA = \
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


$(eval $(value automake2autothing))
include $(topsrcdir)/build-aux/Makefile.tail.mk
