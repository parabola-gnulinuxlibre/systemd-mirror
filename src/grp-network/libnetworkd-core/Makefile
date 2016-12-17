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

noinst_LTLIBRARIES += \
	libnetworkd-core.la

libnetworkd_core_la_CFLAGS = \

libnetworkd_core_la_SOURCES = \
	src/libsystemd-network/network-internal.h \
	src/network/networkd.h \
	src/network/networkd-conf.h \
	src/network/networkd-conf.c \
	src/network/networkd-link.h \
	src/network/networkd-link.c \
	src/network/networkd-netdev.h \
	src/network/networkd-netdev.c \
	src/network/networkd-netdev-vrf.h \
	src/network/networkd-netdev-vrf.c \
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
	src/network/networkd-netdev-vcan.h \
	src/network/networkd-netdev-vcan.c \
	src/network/networkd-link-bus.c \
	src/network/networkd-ipv4ll.c \
	src/network/networkd-dhcp4.c \
	src/network/networkd-dhcp6.c \
	src/network/networkd-ndisc.h \
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
	src/network/networkd-brvlan.h \
	src/network/networkd-brvlan.c \
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
	libsystemd-shared.la

include $(topsrcdir)/build-aux/Makefile.tail.mk
