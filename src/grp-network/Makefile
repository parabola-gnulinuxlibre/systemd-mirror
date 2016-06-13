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
rootlibexec_PROGRAMS += \
	systemd-networkd

systemd_networkd_SOURCES = \
	src/network/networkd.c

systemd_networkd_LDADD = \
	libnetworkd-core.la

ifneq ($(HAVE_LIBIPTC),)
systemd_networkd_LDADD += \
	libfirewall.la
endif # HAVE_LIBIPTC

noinst_LTLIBRARIES += \
	libnetworkd-core.la

libnetworkd_core_la_CFLAGS = \
	$(AM_CFLAGS)

libnetworkd_core_la_SOURCES = \
	src/libsystemd-network/network-internal.h \
	src/network/networkd.h \
	src/network/networkd-conf.h \
	src/network/networkd-conf.c \
	src/network/networkd-link.h \
	src/network/networkd-link.c \
	src/network/networkd-netdev.h \
	src/network/networkd-netdev.c \
	src/network/networkd-netdev-tunnel.h \
	src/network/networkd-netdev-tunnel.c \
	src/network/networkd-netdev-veth.h \
	src/network/networkd-netdev-veth.c \
	src/network/networkd-netdev-vxlan.h \
	src/network/networkd-netdev-vxlan.c \
	src/network/networkd-netdev-vlan.h \
	src/network/networkd-netdev-vlan.c \
	src/network/networkd-netdev-macvlan.h \
	src/network/networkd-netdev-macvlan.c \
	src/network/networkd-netdev-ipvlan.h \
	src/network/networkd-netdev-ipvlan.c \
	src/network/networkd-netdev-dummy.h \
	src/network/networkd-netdev-dummy.c \
	src/network/networkd-netdev-tuntap.h \
	src/network/networkd-netdev-tuntap.c \
	src/network/networkd-netdev-bond.h \
	src/network/networkd-netdev-bond.c \
	src/network/networkd-netdev-bridge.h \
	src/network/networkd-netdev-bridge.c \
	src/network/networkd-link-bus.c \
	src/network/networkd-ipv4ll.c \
	src/network/networkd-dhcp4.c \
	src/network/networkd-dhcp6.c \
	src/network/networkd-ndisc.c \
	src/network/networkd-network.h \
	src/network/networkd-network.c \
	src/network/networkd-network-bus.c \
	src/network/networkd-address.h \
	src/network/networkd-address.c \
	src/network/networkd-route.h \
	src/network/networkd-route.c \
	src/network/networkd-manager.c \
	src/network/networkd-manager-bus.c \
	src/network/networkd-fdb.h \
	src/network/networkd-fdb.c \
	src/network/networkd-address-pool.h \
	src/network/networkd-address-pool.c \
	src/network/networkd-util.h \
	src/network/networkd-util.c \
	src/network/networkd-lldp-tx.h \
	src/network/networkd-lldp-tx.c

nodist_libnetworkd_core_la_SOURCES = \
	src/network/networkd-gperf.c \
	src/network/networkd-network-gperf.c \
	src/network/networkd-netdev-gperf.c

libnetworkd_core_la_LIBADD = \
	libsystemd-network.la \
	libshared.la

rootlibexec_PROGRAMS += \
	systemd-networkd-wait-online

systemd_networkd_wait_online_CFLAGS = \
	$(AM_CFLAGS)

systemd_networkd_wait_online_SOURCES = \
	src/libsystemd-network/network-internal.h \
	src/network/networkd-wait-online.h \
	src/network/networkd-wait-online-link.h \
	src/network/networkd-wait-online.c \
	src/network/networkd-wait-online-manager.c \
	src/network/networkd-wait-online-link.c

systemd_networkd_wait_online_LDADD = \
	libsystemd-network.la \
	libshared.la

rootbin_PROGRAMS += \
	networkctl

networkctl_SOURCES = \
	src/network/networkctl.c

networkctl_LDADD = \
	libshared.la \
	libsystemd-network.la

dist_bashcompletion_data += \
	shell-completion/bash/networkctl

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
	libfirewall.la
endif # HAVE_LIBIPTC

test_network_tables_SOURCES = \
	src/network/test-network-tables.c \
	src/shared/test-tables.h

test_network_tables_LDADD = \
	libnetworkd-core.la \
	libudev-core.la

ifneq ($(HAVE_LIBIPTC),)
test_network_tables_LDADD += \
	libfirewall.la
endif # HAVE_LIBIPTC

tests += \
	test-networkd-conf \
	test-network \
	test-network-tables

dist_systemunit_DATA += \
	units/systemd-networkd.socket

nodist_systemunit_DATA += \
	units/systemd-networkd.service \
	units/systemd-networkd-wait-online.service

dist_systemunit_DATA_busnames += \
	units/org.freedesktop.network1.busname

dist_dbussystemservice_DATA += \
	src/network/org.freedesktop.network1.service

dist_dbuspolicy_DATA += \
	src/network/org.freedesktop.network1.conf

GENERAL_ALIASES += \
	$(systemunitdir)/systemd-networkd.socket $(pkgsysconfdir)/system/sockets.target.wants/systemd-networkd.socket \
	$(systemunitdir)/systemd-networkd.service $(pkgsysconfdir)/system/multi-user.target.wants/systemd-networkd.service \
	$(systemunitdir)/systemd-networkd-wait-online.service $(pkgsysconfdir)/system/network-online.target.wants/systemd-networkd-wait-online.service

SYSTEM_UNIT_ALIASES += \
	systemd-networkd.service dbus-org.freedesktop.network1.service

BUSNAMES_TARGET_WANTS += \
	org.freedesktop.network1.busname

endif # ENABLE_NETWORKD

gperf_gperf_sources += \
	src/network/networkd-gperf.gperf \
	src/network/networkd-network-gperf.gperf \
	src/network/networkd-netdev-gperf.gperf

EXTRA_DIST += \
	units/systemd-networkd.service.m4.in \
	units/systemd-networkd-wait-online.service.in \
	test/networkd-test.py

include $(topsrcdir)/build-aux/Makefile.tail.mk
