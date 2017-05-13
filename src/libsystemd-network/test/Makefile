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

test_dhcp_option_SOURCES = \
	src/libsystemd-network/dhcp-protocol.h \
	src/libsystemd-network/dhcp-internal.h \
	src/libsystemd-network/test-dhcp-option.c

test_dhcp_option_LDADD = \
	libsystemd-network.la \
	libsystemd-shared.la

test_dhcp_client_SOURCES = \
	src/systemd/sd-dhcp-client.h \
	src/libsystemd-network/dhcp-protocol.h \
	src/libsystemd-network/dhcp-internal.h \
	src/libsystemd-network/test-dhcp-client.c

test_dhcp_client_LDADD = \
	libsystemd-network.la \
	libsystemd-shared.la

test_dhcp_server_SOURCES = \
	src/libsystemd-network/test-dhcp-server.c

test_dhcp_server_LDADD = \
	libsystemd-network.la \
	libsystemd-shared.la

test_ipv4ll_SOURCES = \
	src/systemd/sd-ipv4ll.h \
	src/libsystemd-network/arp-util.h \
	src/libsystemd-network/test-ipv4ll.c

test_ipv4ll_LDADD = \
	libsystemd-network.la \
	libsystemd-shared.la

test_ipv4ll_manual_SOURCES = \
	src/systemd/sd-ipv4ll.h \
	src/libsystemd-network/test-ipv4ll-manual.c

test_ipv4ll_manual_LDADD = \
	libsystemd-network.la \
	libsystemd-shared.la

test_acd_SOURCES = \
	src/systemd/sd-ipv4acd.h \
	src/libsystemd-network/test-acd.c

test_acd_LDADD = \
	libsystemd-network.la \
	libsystemd-shared.la

test_ndisc_rs_SOURCES = \
	src/systemd/sd-dhcp6-client.h \
	src/systemd/sd-ndisc.h \
	src/libsystemd-network/icmp6-util.h \
	src/libsystemd-network/test-ndisc-rs.c \
	src/libsystemd-network/dhcp-identifier.h \
	src/libsystemd-network/dhcp-identifier.c

test_ndisc_rs_LDADD = \
	libsystemd-network.la \
	libudev.la \
	libsystemd-shared.la

test_dhcp6_client_SOURCES = \
	src/systemd/sd-dhcp6-client.h \
	src/libsystemd-network/dhcp6-internal.h \
	src/libsystemd-network/test-dhcp6-client.c \
	src/libsystemd-network/dhcp-identifier.h \
	src/libsystemd-network/dhcp-identifier.c

test_dhcp6_client_LDADD = \
	libsystemd-network.la \
	libudev.la \
	libsystemd-shared.la

test_lldp_SOURCES = \
	src/libsystemd-network/test-lldp.c

test_lldp_LDADD = \
	libsystemd-network.la \
	libsystemd-shared.la

tests += \
	test-dhcp-option \
	test-dhcp-client \
	test-dhcp-server \
	test-ipv4ll \
	test-ndisc-rs \
	test-dhcp6-client \
	test-lldp

include $(topsrcdir)/build-aux/Makefile.tail.mk
