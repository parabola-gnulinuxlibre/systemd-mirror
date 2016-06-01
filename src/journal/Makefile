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

audit_list_includes = -include linux/audit.h -include missing.h
ifneq ($(HAVE_AUDIT),)
audit_list_includes += -include libaudit.h
endif

$(outdir)/audit_type-list.txt:
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)$(CPP) $(CFLAGS) $(AM_CPPFLAGS) $(CPPFLAGS) -dM $(audit_list_includes) - </dev/null | grep -vE 'AUDIT_.*(FIRST|LAST)_' | $(SED) -r -n 's/^#define\s+AUDIT_(\w+)\s+([0-9]{4})\s*$$/\1\t\2/p' | sort -k2 >$@

$(outdir)/audit_type-to-name.h: src/journal/audit_type-list.txt
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "const char *audit_type_to_string(int type) {\n\tswitch(type) {" } {printf "        case AUDIT_%s: return \"%s\";\n", $$1, $$1 } END{ print "        default: return NULL;\n\t}\n}\n" }' <$< >$@


$(outdir)/dns_type-list.txt: src/resolve/dns-type.h
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)$(SED) -n -r 's/.* DNS_TYPE_(\w+).*/\1/p' <$< >$@

$(outdir)/dns_type-to-name.h: src/resolve/dns_type-list.txt
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "const char *dns_type_to_string(int type) {\n\tswitch(type) {" } {printf "        case DNS_TYPE_%s: return ", $$1; sub(/_/, "-"); printf "\"%s\";\n", $$1 } END{ print "        default: return NULL;\n\t}\n}\n" }' <$< >$@

$(outdir)/dns_type-from-name.gperf: src/resolve/dns_type-list.txt
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "struct dns_type_name { const char* name; int id; };"; print "%null-strings"; print "%%";} { s=$$1; sub(/_/, "-", s); printf "%s, ", $$s; printf "DNS_TYPE_%s\n", $$1 }' <$< >$@

systemd_journald_SOURCES = \
	src/journal/journald.c \
	src/journal/journald-server.h

systemd_journald_LDADD = \
	libjournal-core.la \
	libshared.la

systemd_cat_SOURCES = \
	src/journal/cat.c

systemd_cat_LDADD = \
	libjournal-core.la

# using _CFLAGS = in the conditional below would suppress AM_CFLAGS
journalctl_CFLAGS = \
	$(AM_CFLAGS)

journalctl_SOURCES = \
	src/journal/journalctl.c

journalctl_LDADD = \
	libshared.la \
	libudev-core.la

ifneq ($(HAVE_QRENCODE),)
journalctl_SOURCES += \
	src/journal/journal-qrcode.c \
	src/journal/journal-qrcode.h

journalctl_CFLAGS += \
	$(QRENCODE_CFLAGS)

journalctl_LDADD += \
	$(QRENCODE_LIBS)
endif

test_journal_SOURCES = \
	src/journal/test-journal.c

test_journal_LDADD = \
	libjournal-core.la

test_journal_send_SOURCES = \
	src/journal/test-journal-send.c

test_journal_send_LDADD = \
	libjournal-core.la

test_journal_syslog_SOURCES = \
	src/journal/test-journal-syslog.c

test_journal_syslog_LDADD = \
	libjournal-core.la

test_journal_match_SOURCES = \
	src/journal/test-journal-match.c

test_journal_match_LDADD = \
	libjournal-core.la

test_journal_enum_SOURCES = \
	src/journal/test-journal-enum.c

test_journal_enum_LDADD = \
	libjournal-core.la

test_journal_stream_SOURCES = \
	src/journal/test-journal-stream.c

test_journal_stream_LDADD = \
	libjournal-core.la

test_journal_flush_SOURCES = \
	src/journal/test-journal-flush.c

test_journal_flush_LDADD = \
	libjournal-core.la

test_journal_init_SOURCES = \
	src/journal/test-journal-init.c

test_journal_init_LDADD = \
	libjournal-core.la

test_journal_verify_SOURCES = \
	src/journal/test-journal-verify.c

test_journal_verify_LDADD = \
	libjournal-core.la

test_journal_interleaving_SOURCES = \
	src/journal/test-journal-interleaving.c

test_journal_interleaving_LDADD = \
	libjournal-core.la

test_mmap_cache_SOURCES = \
	src/journal/test-mmap-cache.c

test_mmap_cache_LDADD = \
	libjournal-core.la

test_catalog_SOURCES = \
	src/journal/test-catalog.c

test_catalog_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DCATALOG_DIR=\"$(abs_top_srcdir)/catalog\"

test_catalog_LDADD = \
	libjournal-core.la

test_compress_SOURCES = \
	src/journal/test-compress.c

test_compress_LDADD = \
	libshared.la

test_compress_benchmark_SOURCES = \
	src/journal/test-compress-benchmark.c

test_compress_benchmark_LDADD = \
	libshared.la

test_audit_type_SOURCES = \
	src/journal/test-audit-type.c

test_audit_type_LDADD = \
	libjournal-core.la

libjournal_core_la_SOURCES = \
	src/journal/journald-kmsg.c \
	src/journal/journald-kmsg.h \
	src/journal/journald-syslog.c \
	src/journal/journald-syslog.h \
	src/journal/journald-stream.c \
	src/journal/journald-stream.h \
	src/journal/journald-server.c \
	src/journal/journald-server.h \
	src/journal/journald-console.c \
	src/journal/journald-console.h \
	src/journal/journald-wall.c \
	src/journal/journald-wall.h \
	src/journal/journald-native.c \
	src/journal/journald-native.h \
	src/journal/journald-audit.c \
	src/journal/journald-audit.h \
	src/journal/journald-rate-limit.c \
	src/journal/journald-rate-limit.h \
	src/journal/journal-internal.h

nodist_libjournal_core_la_SOURCES = \
	src/journal/journald-gperf.c

libjournal_core_la_LIBADD = \
	libshared.la

noinst_LTLIBRARIES += \
	libjournal-core.la

journal-install-hook:
	-$(MKDIR_P) $(DESTDIR)/var/log/journal
	-chown 0:0 $(DESTDIR)/var/log/journal
	-chmod 755 $(DESTDIR)/var/log/journal
	-setfacl -nm g:adm:rx,d:g:adm:rx $(DESTDIR)/var/log/journal/
	-setfacl -nm g:wheel:rx,d:g:wheel:rx $(DESTDIR)/var/log/journal/

journal-uninstall-hook:
	-rmdir $(DESTDIR)/var/log/journal/remote
	-rmdir $(DESTDIR)/var/log/journal/

INSTALL_EXEC_HOOKS += journal-install-hook
UNINSTALL_EXEC_HOOKS += journal-uninstall-hook

# ------------------------------------------------------------------------------
# Update catalog on installation. Do not bother if installing
# in DESTDIR, since this is likely for packaging purposes.
catalog-update-hook:
	-test -n "$(DESTDIR)" || $(rootbindir)/journalctl --update-catalog

INSTALL_DATA_HOOKS += \
	catalog-update-hook

catalog-remove-hook:
	-test -n "$(DESTDIR)" || rm -f $(catalogstatedir)/database

UNINSTALL_DATA_HOOKS += \
	catalog-remove-hook

manual_tests += \
	test-journal-enum

tests += \
	test-journal \
	test-journal-send \
	test-journal-syslog \
	test-journal-match \
	test-journal-stream \
	test-journal-init \
	test-journal-verify \
	test-journal-interleaving \
	test-journal-flush \
	test-mmap-cache \
	test-catalog \
	test-audit-type

ifneq ($(HAVE_COMPRESSION),)
tests += \
	test-compress \
	test-compress-benchmark
endif


rootlibexec_PROGRAMS += \
	systemd-journald

rootbin_PROGRAMS += \
	journalctl

bin_PROGRAMS += \
	systemd-cat

dist_systemunit_DATA += \
	units/systemd-journald.socket \
	units/systemd-journald-dev-log.socket \
	units/systemd-journald-audit.socket

nodist_systemunit_DATA += \
	units/systemd-journald.service \
	units/systemd-journal-flush.service \
	units/systemd-journal-catalog-update.service

dist_pkgsysconf_DATA += \
	src/journal/journald.conf

dist_catalog_DATA = \
	catalog/systemd.be.catalog \
	catalog/systemd.be@latin.catalog \
	catalog/systemd.fr.catalog \
	catalog/systemd.it.catalog \
	catalog/systemd.pl.catalog \
	catalog/systemd.pt_BR.catalog \
	catalog/systemd.ru.catalog \
	catalog/systemd.zh_CN.catalog \
	catalog/systemd.zh_TW.catalog \
	catalog/systemd.catalog

SOCKETS_TARGET_WANTS += \
	systemd-journald.socket \
	systemd-journald-dev-log.socket \
	systemd-journald-audit.socket

SYSINIT_TARGET_WANTS += \
	systemd-journald.service \
	systemd-journal-flush.service \
	systemd-journal-catalog-update.service

EXTRA_DIST += \
	units/systemd-journald.service.in \
	units/systemd-journal-flush.service.in \
	units/systemd-journal-catalog-update.service.in

gperf_gperf_sources += \
	src/journal/journald-gperf.gperf

# ------------------------------------------------------------------------------
ifneq ($(HAVE_MICROHTTPD),)
gatewayddocumentrootdir=$(pkgdatadir)/gatewayd

rootlibexec_PROGRAMS += \
	systemd-journal-gatewayd

systemd_journal_gatewayd_SOURCES = \
	src/journal-remote/journal-gatewayd.c \
	src/journal-remote/microhttpd-util.h \
	src/journal-remote/microhttpd-util.c

systemd_journal_gatewayd_LDADD = \
	libshared.la \
	$(MICROHTTPD_LIBS)

ifneq ($(HAVE_GNUTLS),)
systemd_journal_gatewayd_LDADD += \
	$(GNUTLS_LIBS)
endif

systemd_journal_gatewayd_CFLAGS = \
	$(AM_CFLAGS) \
	$(MICROHTTPD_CFLAGS)

systemd_journal_gatewayd_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DDOCUMENT_ROOT=\"$(gatewayddocumentrootdir)\"

dist_systemunit_DATA += \
	units/systemd-journal-gatewayd.socket

nodist_systemunit_DATA += \
	units/systemd-journal-gatewayd.service

dist_gatewayddocumentroot_DATA = \
	src/journal-remote/browse.html

endif

EXTRA_DIST += \
	units/systemd-journal-gatewayd.service.in

$(eval $(value automake2autothing))
include $(topsrcdir)/build-aux/Makefile.tail.mk
