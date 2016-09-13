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

systemd_resolve_SOURCES = \
	src/resolve/resolve-tool.c \
	$(basic_dns_sources) \
	src/shared/gcrypt-util.c \
	src/shared/gcrypt-util.h

nodist_systemd_resolve_SOURCES = \
	src/resolve/dns_type-from-name.h \
	src/resolve/dns_type-to-name.h

systemd_resolve_LDADD = \
	libshared.la

bin_PROGRAMS += \
	systemd-resolve

dist_bashcompletion_data += \
	shell-completion/bash/systemd-resolve

dist_zshcompletion_data += \
	shell-completion/zsh/_systemd-resolve

tests += \
	test-dns-packet \
	test-resolve-tables \
	test-dnssec

manual_tests += \
	test-dnssec-complex

test_resolve_tables_SOURCES = \
	src/resolve/test-resolve-tables.c \
	src/resolve/dns_type-from-name.h \
	src/resolve/dns_type-to-name.h \
	$(basic_dns_sources) \
	src/shared/test-tables.h

test_resolve_tables_LDADD = \
	libshared.la

test_dns_packet_SOURCES = \
	src/resolve/test-dns-packet.c \
	$(basic_dns_sources)

test_dns_packet_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DRESOLVE_TEST_DIR=\"$(abs_top_srcdir)/src/resolve/test-data\"

test_dns_packet_LDADD = \
	libshared.la

EXTRA_DIST += \
	src/resolve/test-data/_openpgpkey.fedoraproject.org.pkts \
	src/resolve/test-data/fedoraproject.org.pkts \
	src/resolve/test-data/gandi.net.pkts \
	src/resolve/test-data/google.com.pkts \
	src/resolve/test-data/root.pkts \
	src/resolve/test-data/sw1a1aa-sw1a2aa-sw1a2ab-sw1a2ac.find.me.uk.pkts \
	src/resolve/test-data/teamits.com.pkts \
	src/resolve/test-data/zbyszek@fedoraproject.org.pkts \
	src/resolve/test-data/_443._tcp.fedoraproject.org.pkts \
	src/resolve/test-data/kyhwana.org.pkts \
	src/resolve/test-data/fake-caa.pkts

test_dnssec_SOURCES = \
	src/resolve/test-dnssec.c \
	$(basic_dns_sources)

test_dnssec_LDADD = \
	libshared.la

test_dnssec_complex_SOURCES = \
	src/resolve/test-dnssec-complex.c \
	src/resolve/dns-type.c \
	src/resolve/dns-type.h

test_dnssec_complex_LDADD = \
	libshared.la

endif # ENABLE_RESOLVED

gperf_txt_sources += \
	src/resolve/dns_type-list.txt

gperf_gperf_sources += \
	src/resolve/resolved-gperf.gperf

EXTRA_DIST += \
	units/systemd-resolved.service.m4.in \
	src/resolve/resolved.conf.in

include $(topsrcdir)/build-aux/Makefile.tail.mk
