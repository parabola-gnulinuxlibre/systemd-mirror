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

ifneq ($(HAVE_LIBCURL),)
rootlibexec_PROGRAMS += \
        systemd-journal-upload

systemd_journal_upload_SOURCES = \
        src/journal-remote/journal-upload.h \
        src/journal-remote/journal-upload.c \
        src/journal-remote/journal-upload-journal.c

systemd_journal_upload_CFLAGS = \
        $(LIBCURL_CFLAGS)

systemd_journal_upload_LDADD = \
        libsystemd-shared.la \
        $(LIBCURL_LIBS)

nodist_systemunit_DATA += \
        units/systemd-journal-upload.service

nodist_pkgsysconf_DATA += \
        src/journal-remote/journal-upload.conf

endif # HAVE_LIBCURL

EXTRA_DIST += \
        units/systemd-journal-upload.service.in \
        src/journal-remote/journal-upload.conf.in

include $(topsrcdir)/build-aux/Makefile.tail.mk
