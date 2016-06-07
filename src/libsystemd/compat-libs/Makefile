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

ifneq ($(ENABLE_COMPAT_LIBS),)
libsystemd-%.c: src/compat-libs/libsystemd-%.sym
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)sed -r -n 's/^ +(sd_.*);/obsolete_lib(\1,$(notdir $(basename $<)));/p' <$< >$@

BUILT_SOURCES += \
	libsystemd-journal.c \
	libsystemd-login.c \
	libsystemd-id128.c \
	libsystemd-daemon.c

nodist_libsystemd_journal_la_SOURCES = \
	libsystemd-journal.c

libsystemd_journal_la_SOURCES = \
	src/compat-libs/libsystemd-journal.sym

libsystemd_journal_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-imacros$(top_srcdir)/src/compat-libs/linkwarning.h

libsystemd_journal_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info $(LIBSYSTEMD_JOURNAL_CURRENT):$(LIBSYSTEMD_JOURNAL_REVISION):$(LIBSYSTEMD_JOURNAL_AGE) \
	-Wl,--version-script=$(top_srcdir)/src/compat-libs/libsystemd-journal.sym

libsystemd_journal_la_LIBADD = \
	libsystemd-journal-internal.la \
	libsystemd-internal.la

nodist_libsystemd_login_la_SOURCES = \
	libsystemd-login.c

libsystemd_login_la_SOURCES = \
	src/compat-libs/libsystemd-login.sym

libsystemd_login_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-imacros$(top_srcdir)/src/compat-libs/linkwarning.h

libsystemd_login_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info $(LIBSYSTEMD_LOGIN_CURRENT):$(LIBSYSTEMD_LOGIN_REVISION):$(LIBSYSTEMD_LOGIN_AGE) \
	-Wl,--version-script=$(top_srcdir)/src/compat-libs/libsystemd-login.sym

libsystemd_login_la_LIBADD = \
	libsystemd-internal.la

nodist_libsystemd_id128_la_SOURCES = \
	libsystemd-id128.c

libsystemd_id128_la_SOURCES = \
	src/compat-libs/libsystemd-id128.sym

libsystemd_id128_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-imacros$(top_srcdir)/src/compat-libs/linkwarning.h

libsystemd_id128_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info $(LIBSYSTEMD_ID128_CURRENT):$(LIBSYSTEMD_ID128_REVISION):$(LIBSYSTEMD_ID128_AGE) \
	-Wl,--version-script=$(top_srcdir)/src/compat-libs/libsystemd-id128.sym

libsystemd_id128_la_LIBADD = \
	libsystemd-internal.la

nodist_libsystemd_daemon_la_SOURCES = \
	libsystemd-daemon.c

libsystemd_daemon_la_SOURCES = \
	src/compat-libs/libsystemd-daemon.sym

libsystemd_daemon_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-imacros$(top_srcdir)/src/compat-libs/linkwarning.h

libsystemd_daemon_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info $(LIBSYSTEMD_DAEMON_CURRENT):$(LIBSYSTEMD_DAEMON_REVISION):$(LIBSYSTEMD_DAEMON_AGE) \
	-Wl,--version-script=$(top_srcdir)/src/compat-libs/libsystemd-daemon.sym

libsystemd_daemon_la_LIBADD = \
	libsystemd-internal.la

lib_LTLIBRARIES += \
	libsystemd-journal.la \
	libsystemd-login.la \
	libsystemd-id128.la \
	libsystemd-daemon.la

pkgconfiglib_DATA += \
	src/compat-libs/libsystemd-journal.pc \
	src/compat-libs/libsystemd-login.pc \
	src/compat-libs/libsystemd-id128.pc \
	src/compat-libs/libsystemd-daemon.pc

# move lib from $(libdir) to $(libdir) and update devel link, if needed
compat-lib-install-hook:
	libname=libsystemd-login.so && $(move-to-libdir)
	libname=libsystemd-journal.so && $(move-to-libdir)
	libname=libsystemd-id128.so && $(move-to-libdir)
	libname=libsystemd-daemon.so && $(move-to-libdir)

compat-lib-uninstall-hook:
	rm -f $(DESTDIR)$(libdir)/libsystemd-login.so*
	rm -f $(DESTDIR)$(libdir)/libsystemd-journal.so*
	rm -f $(DESTDIR)$(libdir)/libsystemd-id128.so*
	rm -f $(DESTDIR)$(libdir)/libsystemd-daemon.so*

INSTALL_EXEC_HOOKS += compat-lib-install-hook
UNINSTALL_EXEC_HOOKS += compat-lib-uninstall-hook
endif # ENABLE_COMPAT_LIBS

EXTRA_DIST += \
	src/compat-libs/linkwarning.h \
	src/compat-libs/libsystemd-journal.pc.in \
	src/compat-libs/libsystemd-login.pc.in \
	src/compat-libs/libsystemd-id128.pc.in \
	src/compat-libs/libsystemd-daemon.pc.in

include $(topsrcdir)/build-aux/Makefile.tail.mk
