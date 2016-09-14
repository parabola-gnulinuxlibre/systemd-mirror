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

LIBSYSTEMD_CURRENT=16
LIBSYSTEMD_REVISION=0
LIBSYSTEMD_AGE=16

EXTRA_DIST += \
	src/libsystemd/libsystemd.pc.in \
	src/libsystemd/sd-bus/DIFFERENCES \
	src/libsystemd/sd-bus/GVARIANT-SERIALIZATION

#libsystemd_la_SOURCES = \
	$(libsystemd_internal_la_SOURCES) \
	$(libsystemd_journal_internal_la_SOURCES)

#nodist_libsystemd_la_SOURCES = \
	$(nodist_libsystemd_internal_la_SOURCES)

#libsystemd_la_CFLAGS = \
	$(libsystemd_internal_la_CFLAGS) \
	$(libsystemd_journal_internal_la_CFLAGS)

libsystemd_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info $(LIBSYSTEMD_CURRENT):$(LIBSYSTEMD_REVISION):$(LIBSYSTEMD_AGE) \
	-Wl,--version-script=$(srcdir)/libsystemd.sym

libsystemd_la_LIBADD = \
	libsystemd-internal.la \
	libbasic.la \
	libsystemd-journal-internal.la

libsystemd_la_LIBADD = $(libsystemd-internal.DEPENDS)

pkgconfiglib_DATA += \
	src/libsystemd/libsystemd.pc

_pkginclude_HEADERS += \
	src/systemd/sd-bus.h \
	src/systemd/sd-bus-protocol.h \
	src/systemd/sd-bus-vtable.h \
	src/systemd/sd-event.h \
	src/systemd/sd-login.h \
	src/systemd/sd-id128.h \
	src/systemd/sd-daemon.h

_pkginclude_HEADERS += \
	src/systemd/sd-journal.h \
	src/systemd/sd-messages.h \
	src/systemd/_sd-common.h

rootlib_LTLIBRARIES += \
	libsystemd.la

# ------------------------------------------------------------------------------

tests += \
	test-bus-marshal \
	test-bus-signature \
	test-bus-benchmark \
	test-bus-chat \
	test-bus-cleanup \
	test-bus-server \
	test-bus-match \
	test-bus-kernel \
	test-bus-kernel-bloom \
	test-bus-zero-copy \
	test-bus-introspect \
	test-bus-objects \
	test-bus-error \
	test-bus-creds \
	test-bus-gvariant \
	test-event \
	test-netlink \
	test-local-addresses \
	test-resolve

$(outdir)/test-libsystemd-sym.c: \
		$(srcdir)/include/systemd/sd-journal.h \
		$(srcdir)/include/systemd/sd-daemon.h \
		$(srcdir)/include/systemd/sd-login.h \
		$(srcdir)/include/systemd/sd-bus.h \
		$(srcdir)/include/systemd/sd-utf8.h \
		$(srcdir)/include/systemd/sd-resolve.h \
		$(srcdir)/include/systemd/sd-path.h \
		$(srcdir)/include/systemd/sd-event.h

nodist_test_libsystemd_sym_SOURCES = \
	test-libsystemd-sym.c
test_libsystemd_sym_LDADD = \
	libsystemd.la

$(DESTDIR)$(pkgincludedir)/%.h: $(srcdir)/include/systemd/%.h
	@$(NORMAL_INSTALL)
	$(am.INSTALL_HEADERS)

files.sys.all += $(addprefix $(pkgincludedir)/,$(notdir $(_pkginclude_HEADERS)))
$(outdir)/libsystemd.la: $(srcdir)/libsystemd.sym

nested.subdirs += src

include $(topsrcdir)/build-aux/Makefile.tail.mk
