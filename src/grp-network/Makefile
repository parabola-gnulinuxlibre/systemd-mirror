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

dist_network_DATA = \
	network/99-default.link \
	network/80-container-host0.network \
	network/80-container-ve.network \
	network/80-container-vz.network

ifneq ($(ENABLE_NETWORKD),)
test_networkd_conf_SOURCES = \
	src/network/test-networkd-conf.c

test_networkd_conf_LDADD = \
	libnetworkd-core.la

test_network_SOURCES = \
	src/network/test-network.c

test_network_LDADD = \
	libnetworkd-core.la

ifneq ($(HAVE_LIBIPTC),)
test_network_LDADD += \
	libsystemd-firewall.la
endif # HAVE_LIBIPTC

test_network_tables_SOURCES = \
	src/network/test-network-tables.c \
	src/shared/test-tables.h

test_network_tables_LDADD = \
	libnetworkd-core.la \
	libudev-core.la

ifneq ($(HAVE_LIBIPTC),)
test_network_tables_LDADD += \
	libsystemd-firewall.la
endif # HAVE_LIBIPTC

tests += \
	test-networkd-conf \
	test-network \
	test-network-tables

endif # ENABLE_NETWORKD

gperf_gperf_sources += \
	src/network/networkd-gperf.gperf \
	src/network/networkd-network-gperf.gperf \
	src/network/networkd-netdev-gperf.gperf

EXTRA_DIST += \
	units/systemd-networkd.service.m4.in \
	units/systemd-networkd-wait-online.service.in \
	test/networkd-test.py

nested.subdirs += libnetworkd-core
nested.subdirs += networkctl
nested.subdirs += systemd-networkd
nested.subdirs += systemd-networkd-wait-online

include $(topsrcdir)/build-aux/Makefile.tail.mk
