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

LIBSYSTEMD_CURRENT=14
LIBSYSTEMD_REVISION=0
LIBSYSTEMD_AGE=14

# The following four libraries only exist for compatibility reasons,
# their version info should not be bumped anymore
LIBSYSTEMD_LOGIN_CURRENT=9
LIBSYSTEMD_LOGIN_REVISION=3
LIBSYSTEMD_LOGIN_AGE=9

LIBSYSTEMD_DAEMON_CURRENT=0
LIBSYSTEMD_DAEMON_REVISION=12
LIBSYSTEMD_DAEMON_AGE=0

LIBSYSTEMD_ID128_CURRENT=0
LIBSYSTEMD_ID128_REVISION=28
LIBSYSTEMD_ID128_AGE=0

LIBSYSTEMD_JOURNAL_CURRENT=11
LIBSYSTEMD_JOURNAL_REVISION=5
LIBSYSTEMD_JOURNAL_AGE=11

EXTRA_DIST += \
	src/libsystemd/libsystemd.pc.in \
	src/libsystemd/sd-bus/DIFFERENCES \
	src/libsystemd/sd-bus/GVARIANT-SERIALIZATION

libsystemd_la_SOURCES = \
	$(libsystemd_internal_la_SOURCES) \
	$(libsystemd_journal_internal_la_SOURCES)

nodist_libsystemd_la_SOURCES = \
	$(nodist_libsystemd_internal_la_SOURCES)

libsystemd_la_CFLAGS = \
	$(libsystemd_internal_la_CFLAGS) \
	$(libsystemd_journal_internal_la_CFLAGS)

libsystemd_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info $(LIBSYSTEMD_CURRENT):$(LIBSYSTEMD_REVISION):$(LIBSYSTEMD_AGE) \
	-Wl,--version-script=$(@D)/libsystemd.sym

libsystemd_la_LIBADD = \
	$(libsystemd_internal_la_LIBADD) \
	$(libsystemd_journal_internal_la_LIBADD)

pkgconfiglib_DATA += \
	src/libsystemd/libsystemd.pc

pkginclude_HEADERS += \
	src/systemd/sd-bus.h \
	src/systemd/sd-bus-protocol.h \
	src/systemd/sd-bus-vtable.h \
	src/systemd/sd-event.h \
	src/systemd/sd-login.h \
	src/systemd/sd-id128.h \
	src/systemd/sd-daemon.h

lib_LTLIBRARIES += \
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
	test-bus-proxy \
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

bin_PROGRAMS += \
	busctl

test_bus_marshal_SOURCES = \
	src/libsystemd/sd-bus/test-bus-marshal.c

test_bus_marshal_LDADD = \
	libshared.la \
	$(GLIB_LIBS) \
	$(DBUS_LIBS)

test_bus_marshal_CFLAGS = \
	$(AM_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(DBUS_CFLAGS)

test_bus_signature_SOURCES = \
	src/libsystemd/sd-bus/test-bus-signature.c

test_bus_signature_LDADD = \
	libshared.la

test_bus_chat_SOURCES = \
	src/libsystemd/sd-bus/test-bus-chat.c

test_bus_chat_LDADD = \
	libshared.la

test_bus_cleanup_SOURCES = \
	src/libsystemd/sd-bus/test-bus-cleanup.c

test_bus_cleanup_CFLAGS = \
	$(AM_CFLAGS) \
	$(SECCOMP_CFLAGS)

test_bus_cleanup_LDADD = \
	libshared.la

test_bus_server_SOURCES = \
	src/libsystemd/sd-bus/test-bus-server.c

test_bus_server_LDADD = \
	libshared.la

test_bus_objects_SOURCES = \
	src/libsystemd/sd-bus/test-bus-objects.c

test_bus_objects_LDADD = \
	libshared.la

test_bus_error_SOURCES = \
	src/libsystemd/sd-bus/test-bus-error.c

test_bus_error_LDADD = \
	libshared.la

test_bus_gvariant_SOURCES = \
	src/libsystemd/sd-bus/test-bus-gvariant.c

test_bus_gvariant_LDADD = \
	libshared.la \
	$(GLIB_LIBS)

test_bus_gvariant_CFLAGS = \
	$(AM_CFLAGS) \
	$(GLIB_CFLAGS)

test_bus_creds_SOURCES = \
	src/libsystemd/sd-bus/test-bus-creds.c

test_bus_creds_LDADD = \
	libshared.la

test_bus_match_SOURCES = \
	src/libsystemd/sd-bus/test-bus-match.c

test_bus_match_LDADD = \
	libshared.la

test_bus_proxy_SOURCES = \
	src/libsystemd/sd-bus/test-bus-proxy.c

test_bus_proxy_LDADD = \
	libshared.la

test_bus_kernel_SOURCES = \
	src/libsystemd/sd-bus/test-bus-kernel.c

test_bus_kernel_LDADD = \
	libshared.la

test_bus_kernel_bloom_SOURCES = \
	src/libsystemd/sd-bus/test-bus-kernel-bloom.c

test_bus_kernel_bloom_LDADD = \
	libshared.la

test_bus_benchmark_SOURCES = \
	src/libsystemd/sd-bus/test-bus-benchmark.c

test_bus_benchmark_LDADD = \
	libshared.la

test_bus_zero_copy_SOURCES = \
	src/libsystemd/sd-bus/test-bus-zero-copy.c

test_bus_zero_copy_LDADD = \
	libshared.la

test_bus_introspect_SOURCES = \
	src/libsystemd/sd-bus/test-bus-introspect.c

test_bus_introspect_LDADD = \
	libshared.la

test_event_SOURCES = \
	src/libsystemd/sd-event/test-event.c

test_event_LDADD = \
	libshared.la

test_netlink_SOURCES = \
	src/libsystemd/sd-netlink/test-netlink.c

test_netlink_LDADD = \
	libshared.la

test_local_addresses_SOURCES = \
	src/libsystemd/sd-netlink/test-local-addresses.c

test_local_addresses_LDADD = \
	libshared.la

test_resolve_SOURCES = \
	src/libsystemd/sd-resolve/test-resolve.c

test_resolve_LDADD = \
	libshared.la

busctl_SOURCES = \
	src/libsystemd/sd-bus/busctl.c \
	src/libsystemd/sd-bus/busctl-introspect.c \
	src/libsystemd/sd-bus/busctl-introspect.h

busctl_LDADD = \
	libshared.la

test-libsystemd-sym.c: \
		$(top_builddir)/src/libsystemd/libsystemd.sym \
		src/systemd/sd-journal.h \
		src/systemd/sd-daemon.h \
		src/systemd/sd-login.h \
		src/systemd/sd-bus.h \
		src/systemd/sd-utf8.h \
		src/systemd/sd-resolve.h \
		src/systemd/sd-path.h \
		src/systemd/sd-event.h
	$(generate-sym-test)

nodist_test_libsystemd_sym_SOURCES = \
	test-libsystemd-sym.c
test_libsystemd_sym_LDADD = \
	libsystemd.la

$(eval $(value automake2autothing))
at.subdirs += compat-libs libsystemd-internal libsystem-journal-internal
include $(topsrcdir)/build-aux/Makefile.tail.mk
