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
	libjournal-core.la

systemd_journal_remote_SOURCES += \
	src/journal-remote/microhttpd-util.h \
	src/journal-remote/microhttpd-util.c

systemd_journal_remote_CFLAGS = \
	$(MICROHTTPD_CFLAGS)

systemd_journal_remote_LDADD += \
	$(MICROHTTPD_LIBS)

ifneq ($(HAVE_GNUTLS),)
systemd_journal_remote_LDADD += \
	$(GNUTLS_LIBS)
endif # HAVE_GNUTLS

# systemd-journal-remote make sense mostly with full crypto stack
dist_systemunit_DATA += \
	units/systemd-journal-remote.socket

nodist_systemunit_DATA += \
	units/systemd-journal-remote.service

journal-remote-install-hook: journal-install-hook
	-$(MKDIR_P) $(DESTDIR)/var/log/journal/remote
	-chown 0:0 $(DESTDIR)/var/log/journal/remote
	-chmod 755 $(DESTDIR)/var/log/journal/remote

INSTALL_EXEC_HOOKS += journal-remote-install-hook

nodist_pkgsysconf_DATA += \
	src/journal-remote/journal-remote.conf

EXTRA_DIST += \
	units/systemd-journal-remote.service.in \
	src/journal-remote/journal-remote.conf.in \
	src/journal-remote/log-generator.py
endif # HAVE_MICROHTTPD

include $(topsrcdir)/build-aux/Makefile.tail.mk
