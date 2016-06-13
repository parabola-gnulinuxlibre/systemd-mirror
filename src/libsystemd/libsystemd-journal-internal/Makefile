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

audit_list_includes = -include linux/audit.h -include missing.h
ifneq ($(HAVE_AUDIT),)
audit_list_includes += -include libaudit.h
endif # HAVE_AUDIT

$(outdir)/audit_type-list.txt:
	$(AM_V_GEN)$(CPP) $(ALL_CPPFLAGS) -dM $(audit_list_includes) - </dev/null | grep -vE 'AUDIT_.*(FIRST|LAST)_' | $(SED) -r -n 's/^#define\s+AUDIT_(\w+)\s+([0-9]{4})\s*$$/\1\t\2/p' | sort -k2 >$@

$(outdir)/audit_type-to-name.h: $(outdir)/audit_type-list.txt
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "const char *audit_type_to_string(int type) {\n\tswitch(type) {" } {printf "        case AUDIT_%s: return \"%s\";\n", $$1, $$1 } END{ print "        default: return NULL;\n\t}\n}\n" }' <$< >$@

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
