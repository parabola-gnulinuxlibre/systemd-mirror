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

libsystemd_internal_la_SOURCES = \
	src/systemd/sd-bus.h \
	src/systemd/sd-bus-protocol.h \
	src/systemd/sd-bus-vtable.h \
	src/systemd/sd-utf8.h \
	src/systemd/sd-event.h \
	src/systemd/sd-netlink.h \
	src/systemd/sd-resolve.h \
	src/systemd/sd-login.h \
	src/systemd/sd-id128.h \
	src/systemd/sd-daemon.h \
	src/systemd/sd-path.h \
	src/systemd/sd-network.h \
	src/systemd/sd-hwdb.h \
	src/systemd/sd-device.h \
	src/libsystemd/libsystemd.sym \
	src/libsystemd/sd-bus/sd-bus.c \
	src/libsystemd/sd-bus/bus-control.c \
	src/libsystemd/sd-bus/bus-control.h \
	src/libsystemd/sd-bus/bus-error.c \
	src/libsystemd/sd-bus/bus-error.h \
	src/libsystemd/sd-bus/bus-common-errors.h \
	src/libsystemd/sd-bus/bus-common-errors.c \
	src/libsystemd/sd-bus/bus-internal.c \
	src/libsystemd/sd-bus/bus-internal.h \
	src/libsystemd/sd-bus/bus-socket.c \
	src/libsystemd/sd-bus/bus-socket.h \
	src/libsystemd/sd-bus/bus-kernel.c \
	src/libsystemd/sd-bus/bus-kernel.h \
	src/libsystemd/sd-bus/bus-container.c \
	src/libsystemd/sd-bus/bus-container.h \
	src/libsystemd/sd-bus/bus-message.c \
	src/libsystemd/sd-bus/bus-message.h \
	src/libsystemd/sd-bus/bus-creds.c \
	src/libsystemd/sd-bus/bus-creds.h \
	src/libsystemd/sd-bus/bus-signature.c \
	src/libsystemd/sd-bus/bus-signature.h \
	src/libsystemd/sd-bus/bus-type.c \
	src/libsystemd/sd-bus/bus-type.h \
	src/libsystemd/sd-bus/bus-match.c \
	src/libsystemd/sd-bus/bus-match.h \
	src/libsystemd/sd-bus/bus-bloom.c \
	src/libsystemd/sd-bus/bus-bloom.h \
	src/libsystemd/sd-bus/bus-introspect.c \
	src/libsystemd/sd-bus/bus-introspect.h \
	src/libsystemd/sd-bus/bus-objects.c \
	src/libsystemd/sd-bus/bus-objects.h \
	src/libsystemd/sd-bus/bus-gvariant.c \
	src/libsystemd/sd-bus/bus-gvariant.h \
	src/libsystemd/sd-bus/bus-convenience.c \
	src/libsystemd/sd-bus/bus-track.c \
	src/libsystemd/sd-bus/bus-track.h \
	src/libsystemd/sd-bus/bus-slot.c \
	src/libsystemd/sd-bus/bus-slot.h \
	src/libsystemd/sd-bus/bus-protocol.h \
	src/libsystemd/sd-bus/kdbus.h \
	src/libsystemd/sd-bus/bus-dump.c \
	src/libsystemd/sd-bus/bus-dump.h \
	src/libsystemd/sd-utf8/sd-utf8.c \
	src/libsystemd/sd-event/sd-event.c \
	src/libsystemd/sd-netlink/sd-netlink.c \
	src/libsystemd/sd-netlink/netlink-internal.h \
	src/libsystemd/sd-netlink/netlink-message.c \
	src/libsystemd/sd-netlink/netlink-socket.c \
	src/libsystemd/sd-netlink/rtnl-message.c \
	src/libsystemd/sd-netlink/netlink-types.h \
	src/libsystemd/sd-netlink/netlink-types.c \
	src/libsystemd/sd-netlink/netlink-util.h \
	src/libsystemd/sd-netlink/netlink-util.c \
	src/libsystemd/sd-netlink/local-addresses.h \
	src/libsystemd/sd-netlink/local-addresses.c \
	src/libsystemd/sd-id128/sd-id128.c \
	src/libsystemd/sd-daemon/sd-daemon.c \
	src/libsystemd/sd-login/sd-login.c \
	src/libsystemd/sd-path/sd-path.c \
	src/libsystemd/sd-network/sd-network.c \
	src/libsystemd/sd-network/network-util.h \
	src/libsystemd/sd-network/network-util.c \
	src/libsystemd/sd-hwdb/sd-hwdb.c \
	src/libsystemd/sd-hwdb/hwdb-util.h \
	src/libsystemd/sd-hwdb/hwdb-internal.h \
	src/libsystemd/sd-device/device-internal.h \
	src/libsystemd/sd-device/device-util.h \
	src/libsystemd/sd-device/device-enumerator.c \
	src/libsystemd/sd-device/device-enumerator-private.h \
	src/libsystemd/sd-device/sd-device.c \
	src/libsystemd/sd-device/device-private.c \
	src/libsystemd/sd-device/device-private.h \
	src/libsystemd/sd-resolve/sd-resolve.c

libsystemd_internal_la_LIBADD = \
	libbasic.la \
	-lresolv

noinst_LTLIBRARIES += \
	libsystemd-internal.la

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

pkginclude_HEADERS += \
	src/systemd/sd-journal.h \
	src/systemd/sd-messages.h \
	src/systemd/_sd-common.h

libsystemd_journal_internal_la_SOURCES = \
	src/journal/sd-journal.c \
	src/systemd/sd-journal.h \
	src/systemd/_sd-common.h \
	src/journal/journal-file.c \
	src/journal/journal-file.h \
	src/journal/journal-vacuum.c \
	src/journal/journal-vacuum.h \
	src/journal/journal-verify.c \
	src/journal/journal-verify.h \
	src/journal/lookup3.c \
	src/journal/lookup3.h \
	src/journal/journal-send.c \
	src/journal/journal-def.h \
	src/journal/compress.h \
	src/journal/catalog.c \
	src/journal/catalog.h \
	src/journal/mmap-cache.c \
	src/journal/mmap-cache.h \
	src/journal/compress.c \
	src/journal/audit-type.h \
	src/journal/audit-type.c \
	src/shared/gcrypt-util.h \
	src/shared/gcrypt-util.c

nodist_libsystemd_journal_internal_la_SOURCES = \
	src/journal/audit_type-to-name.h

gperf_txt_sources += \
	src/journal/audit_type-list.txt

# using _CFLAGS = in the conditional below would suppress AM_CFLAGS
libsystemd_journal_internal_la_CFLAGS = \
	$(AM_CFLAGS)

libsystemd_journal_internal_la_LIBADD =

ifneq ($(HAVE_XZ),)
libsystemd_journal_internal_la_CFLAGS += \
	$(XZ_CFLAGS)

libsystemd_journal_internal_la_LIBADD += \
	$(XZ_LIBS)
endif # HAVE_XZ

ifneq ($(HAVE_LZ4),)
libsystemd_journal_internal_la_CFLAGS += \
	$(LZ4_CFLAGS)

libsystemd_journal_internal_la_LIBADD += \
	$(LZ4_LIBS)
endif # HAVE_LZ4

ifneq ($(HAVE_GCRYPT),)
libsystemd_journal_internal_la_SOURCES += \
	src/journal/journal-authenticate.c \
	src/journal/journal-authenticate.h \
	src/journal/fsprg.c \
	src/journal/fsprg.h

libsystemd_journal_internal_la_LIBADD += \
	$(GCRYPT_LIBS)

# fsprg.c is a drop-in file using void pointer arithmetic
libsystemd_journal_internal_la_CFLAGS += \
	$(GCRYPT_CFLAGS) \
	-Wno-pointer-arith
endif # HAVE_GCRYPT

noinst_LTLIBRARIES += \
	libsystemd-journal-internal.la
include $(topsrcdir)/build-aux/Makefile.tail.mk
