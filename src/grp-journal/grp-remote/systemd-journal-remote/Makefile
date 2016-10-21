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

ifneq ($(HAVE_MICROHTTPD),)
rootlibexec_PROGRAMS += \
	systemd-journal-remote

systemd_journal_remote_SOURCES = \
	src/journal-remote/journal-remote-parse.h \
	src/journal-remote/journal-remote-parse.c \
	src/journal-remote/journal-remote-write.h \
	src/journal-remote/journal-remote-write.c \
	src/journal-remote/journal-remote.h \
	src/journal-remote/journal-remote.c

systemd_journal_remote_LDADD = \
	libsystemd-microhttpd.la \
	libjournal-core.la

ifneq ($(ENABLE_TMPFILES),)
dist_tmpfiles_DATA += \
	tmpfiles.d/systemd-remote.conf
endif # ENABLE_TMPFILES

# systemd-journal-remote make sense mostly with full crypto stack
dist_systemunit_DATA += \
	units/systemd-journal-remote.socket

dist_sysusers_DATA += \
	sysusers.d/systemd-journal-remote.conf

nodist_systemunit_DATA += \
	units/systemd-journal-remote.service

files.sys.all += /var/log/journal/remote
$(DESTDIR)/var/log/journal/remote:
	$(MKDIR_P) $@

nodist_pkgsysconf_DATA += \
	src/journal-remote/journal-remote.conf

EXTRA_DIST += \
	units/systemd-journal-remote.service.in \
	src/journal-remote/journal-remote.conf.in \
	src/journal-remote/log-generator.py
endif # HAVE_MICROHTTPD

sd.CPPFLAGS += -DPKGSYSCONFDIR=\"$(pkgsysconfdir)\"
sd.CPPFLAGS += -DCERTIFICATE_ROOT=\"$(CERTIFICATEROOT)\"

include $(topsrcdir)/build-aux/Makefile.tail.mk
