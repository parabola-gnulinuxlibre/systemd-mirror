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
	libsystemd-network.la

libsystemd_network_la_CFLAGS = \
	$(KMOD_CFLAGS)

libsystemd_network_la_SOURCES = \
	src/systemd/sd-dhcp-client.h \
	src/systemd/sd-dhcp-server.h \
	src/systemd/sd-dhcp-lease.h \
	src/systemd/sd-ipv4ll.h \
	src/systemd/sd-ipv4acd.h \
	src/systemd/sd-ndisc.h \
	src/systemd/sd-dhcp6-client.h \
	src/systemd/sd-dhcp6-lease.h \
	src/systemd/sd-lldp.h \
	src/libsystemd-network/sd-dhcp-client.c \
	src/libsystemd-network/sd-dhcp-server.c \
	src/libsystemd-network/dhcp-network.c \
	src/libsystemd-network/dhcp-option.c \
	src/libsystemd-network/dhcp-packet.c \
	src/libsystemd-network/dhcp-internal.h \
	src/libsystemd-network/dhcp-server-internal.h \
	src/libsystemd-network/dhcp-protocol.h \
	src/libsystemd-network/dhcp-lease-internal.h \
	src/libsystemd-network/sd-dhcp-lease.c \
	src/libsystemd-network/sd-ipv4ll.c \
	src/libsystemd-network/sd-ipv4acd.c \
	src/libsystemd-network/arp-util.h \
	src/libsystemd-network/arp-util.c \
	src/libsystemd-network/network-internal.c \
	src/libsystemd-network/network-internal.h \
	src/libsystemd-network/sd-ndisc.c \
	src/libsystemd-network/ndisc-internal.h \
	src/libsystemd-network/ndisc-router.h \
	src/libsystemd-network/ndisc-router.c \
	src/libsystemd-network/icmp6-util.h \
	src/libsystemd-network/icmp6-util.c \
	src/libsystemd-network/sd-dhcp6-client.c \
	src/libsystemd-network/dhcp6-internal.h \
	src/libsystemd-network/dhcp6-protocol.h \
	src/libsystemd-network/dhcp6-network.c \
	src/libsystemd-network/dhcp6-option.c \
	src/libsystemd-network/dhcp6-lease-internal.h \
	src/libsystemd-network/sd-dhcp6-lease.c \
	src/libsystemd-network/dhcp-identifier.h \
	src/libsystemd-network/dhcp-identifier.c \
	src/libsystemd-network/lldp-internal.h \
	src/libsystemd-network/lldp-network.h \
	src/libsystemd-network/lldp-network.c \
	src/libsystemd-network/lldp-neighbor.h \
	src/libsystemd-network/lldp-neighbor.c \
	src/libsystemd-network/sd-lldp.c

libsystemd_network_la_LIBADD = \
	$(KMOD_LIBS)

include $(topsrcdir)/build-aux/Makefile.tail.mk
