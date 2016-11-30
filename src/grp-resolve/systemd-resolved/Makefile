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

ifneq ($(ENABLE_RESOLVED),)

systemd_resolved_SOURCES = \
	src/resolve/resolved.c \
	src/resolve/resolved-manager.c \
	src/resolve/resolved-manager.h \
	src/resolve/resolved-conf.c \
	src/resolve/resolved-conf.h \
	src/resolve/resolved-resolv-conf.c \
	src/resolve/resolved-resolv-conf.h \
	src/resolve/resolved-bus.c \
	src/resolve/resolved-bus.h \
	src/resolve/resolved-link.h \
	src/resolve/resolved-link.c \
	src/resolve/resolved-link-bus.c \
	src/resolve/resolved-link-bus.h \
	src/resolve/resolved-llmnr.h \
	src/resolve/resolved-llmnr.c \
	src/resolve/resolved-mdns.h \
	src/resolve/resolved-mdns.c \
	src/resolve/resolved-def.h \
	$(basic_dns_sources) \
	src/resolve/resolved-dns-query.h \
	src/resolve/resolved-dns-query.c \
	src/resolve/resolved-dns-synthesize.h \
	src/resolve/resolved-dns-synthesize.c \
	src/resolve/resolved-dns-transaction.h \
	src/resolve/resolved-dns-transaction.c \
	src/resolve/resolved-dns-scope.h \
	src/resolve/resolved-dns-scope.c \
	src/resolve/resolved-dns-server.h \
	src/resolve/resolved-dns-server.c \
	src/resolve/resolved-dns-search-domain.h \
	src/resolve/resolved-dns-search-domain.c \
	src/resolve/resolved-dns-cache.h \
	src/resolve/resolved-dns-cache.c \
	src/resolve/resolved-dns-zone.h \
	src/resolve/resolved-dns-zone.c \
	src/resolve/resolved-dns-stream.h \
	src/resolve/resolved-dns-stream.c \
	src/resolve/resolved-dns-trust-anchor.h \
	src/resolve/resolved-dns-trust-anchor.c \
	src/resolve/resolved-dns-stub.h \
	src/resolve/resolved-dns-stub.c \
	src/resolve/resolved-etc-hosts.h \
	src/resolve/resolved-etc-hosts.c \
	src/shared/gcrypt-util.c \
	src/shared/gcrypt-util.h

nodist_systemd_resolved_SOURCES = \
	src/resolve/dns_type-from-name.h \
	src/resolve/dns_type-to-name.h \
	src/resolve/resolved-gperf.c

systemd_resolved_CFLAGS = \
	$(GCRYPT_CFLAGS)

systemd_resolved_LDADD = \
	libsystemd-network.la \
	libsystemd-shared.la \
	$(GCRYPT_LIBS) \
	-lm

rootlibexec_PROGRAMS += \
	systemd-resolved

nodist_systemunit_DATA += \
	units/systemd-resolved.service

dist_systemunit_DATA_busnames += \
	units/org.freedesktop.resolve1.busname

dist_dbuspolicy_DATA += \
	src/resolve/org.freedesktop.resolve1.conf

dist_dbussystemservice_DATA += \
	src/resolve/org.freedesktop.resolve1.service

SYSTEM_UNIT_ALIASES += \
	systemd-resolved.service dbus-org.freedesktop.resolve1.service

BUSNAMES_TARGET_WANTS += \
	org.freedesktop.resolve1.busname

GENERAL_ALIASES += \
	$(systemunitdir)/systemd-resolved.service $(pkgsysconfdir)/system/multi-user.target.wants/systemd-resolved.service

nodist_pkgsysconf_DATA += \
	src/resolve/resolved.conf

endif # ENABLE_RESOLVED
gperf_gperf_sources += \
	src/resolve/resolved-gperf.gperf

EXTRA_DIST += \
	units/systemd-resolved.service.m4.in \
	src/resolve/resolved.conf.in

dist_rootlibexec_DATA += \
	src/resolve/resolv.conf

include $(topsrcdir)/build-aux/Makefile.tail.mk
