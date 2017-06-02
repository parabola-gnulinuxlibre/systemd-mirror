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
include $(dir $(lastword $(MAKEFILE_LIST)))/../../../../config.mk
include $(topsrcdir)/build-aux/Makefile.head.mk

rootlibexec_PROGRAMS += systemd-pull
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
	src/import/qcow2-util.c \
	src/import/qcow2-util.h

systemd_pull_CFLAGS = \
	$(LIBCURL_CFLAGS) \
	$(XZ_CFLAGS) \
	$(ZLIB_CFLAGS) \
	$(BZIP2_CFLAGS) \
	$(GCRYPT_CFLAGS) \
	-D VENDOR_KEYRING_PATH=\"$(rootlibexecdir)/import-pubring.gpg\" \
	-D USER_KEYRING_PATH=\"$(pkgsysconfdir)/import-pubring.gpg\"

systemd_pull_LDADD = \
	libsystemd-shared.la \
	$(LIBCURL_LIBS) \
	$(XZ_LIBS) \
	$(ZLIB_LIBS) \
	$(BZIP2_LIBS) \
	$(GCRYPT_LIBS)

dist_rootlibexec_DATA += \
	src/import/import-pubring.gpg

include $(topsrcdir)/build-aux/Makefile.tail.mk
