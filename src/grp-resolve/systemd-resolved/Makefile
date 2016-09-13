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

$(outdir)/dns_type-list.txt: src/resolve/dns-type.h
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)$(SED) -n -r 's/.* DNS_TYPE_(\w+).*/\1/p' <$< >$@

$(outdir)/dns_type-to-name.h: src/resolve/dns_type-list.txt
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "const char *dns_type_to_string(int type) {\n\tswitch(type) {" } {printf "        case DNS_TYPE_%s: return ", $$1; sub(/_/, "-"); printf "\"%s\";\n", $$1 } END{ print "        default: return NULL;\n\t}\n}\n" }' <$< >$@

$(outdir)/dns_type-from-name.gperf: src/resolve/dns_type-list.txt
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "struct dns_type_name { const char* name; int id; };"; print "%null-strings"; print "%%";} { s=$$1; sub(/_/, "-", s); printf "%s, ", $$s; printf "DNS_TYPE_%s\n", $$1 }' <$< >$@

ifneq ($(ENABLE_RESOLVED),)

basic_dns_sources = \
	src/resolve/resolved-dns-dnssec.c \
	src/resolve/resolved-dns-dnssec.h \
	src/resolve/resolved-dns-packet.c \
	src/resolve/resolved-dns-packet.h \
	src/resolve/resolved-dns-rr.c \
	src/resolve/resolved-dns-rr.h \
	src/resolve/resolved-dns-answer.c \
	src/resolve/resolved-dns-answer.h \
	src/resolve/resolved-dns-question.c \
	src/resolve/resolved-dns-question.h \
	src/resolve/dns-type.c \
	src/resolve/dns-type.h

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
	src/resolve/resolved-etc-hosts.h \
	src/resolve/resolved-etc-hosts.c \
	src/shared/gcrypt-util.c \
	src/shared/gcrypt-util.h

nodist_systemd_resolved_SOURCES = \
	src/resolve/dns_type-from-name.h \
	src/resolve/dns_type-to-name.h \
	src/resolve/resolved-gperf.c

systemd_resolved_LDADD = \
	libsystemd-network.la \
	libshared.la

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
