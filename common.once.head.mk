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
AM_CPPFLAGS     = @OUR_CPPFLAGS@
AM_CFLAGS       = @OUR_CFLAGS@
AM_LDFLAGS      = @OUR_LDFLAGS@
AM_LIBTOOLFLAGS = 

ALL_CPPFLAGS     = $(CPPFLAGS) $(AM_CPPFLAGS) $(CPPFLAGS/$(@D))
ALL_CFLAGS       = $(CFLAGS) $(AM_CFLAGS) $(CFLAGS/$(@D))
ALL_LDFLAGS      = $(LDFLAGS) $(AM_LDFLAGS) $(LDFLAGS/$(@D))
ALL_LIBTOOLFLAGS = $(LIBTOOLFLAGS) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS/$(@D))

AM_CPPFLAGS += -include $(topoutdir)/config.h

GCC_COLORS ?= 'ooh, shiny!'
export GCC_COLORS

# remove targets if the command fails
.DELETE_ON_ERROR:

# keep intermediate files
.SECONDARY:

# Keep the test-suite.log
.PRECIOUS: $(TEST_SUITE_LOG) Makefile

%-from-name.gperf: %-list.txt
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "struct $(notdir $*)_name { const char* name; int id; };"; print "%null-strings"; print "%%";} { printf "%s, %s\n", $$1, $$1 }' <$< >$@

%-from-name.h: %-from-name.gperf
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GPERF)$(GPERF) -L ANSI-C -t --ignore-case -N lookup_$(notdir $*) -H hash_$(notdir $*)_name -p -C <$< >$@

# Stupid test that everything purported to be exported really is
define generate-sym-test
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_at)printf '#include <stdio.h>\n' > $@
	$(AM_V_at)printf '#include "%s"\n' $(notdir $(filter %.h, $^)) >> $@
	$(AM_V_at)printf 'void* functions[] = {\n' >> $@
	$(AM_V_GEN)sed -r -n 's/^ +([a-zA-Z0-9_]+);/\1,/p' $< >> $@
	$(AM_V_at)printf '};\nint main(void) {\n' >> $@
	$(AM_V_at)printf 'unsigned i; for (i=0;i<sizeof(functions)/sizeof(void*);i++) printf("%%p\\n", functions[i]);\n' >> $@
	$(AM_V_at)printf 'return 0; }\n' >> $@
endef


lib_LTLIBRARIES
pkginclude_HEADERS

o = $(if $(filter lib%,$(notdir $2)),lo,o)
m = $(subst -,_,$(subst .,_,$2))
define amtarget2dir
am_out_files += $(notdir $2)
LDFLAGS/$(outdir) += $($m_LDFLAGS)
CFLAGS/$(outdir) += $($m_CFLAGS)
LDFLAGS/$(outdir) += $($m_LDFLAGS)
$(outdir)/$(notdir $2): $(filter %.$o,$(patsubst $(addsuffix /%.c,$1),%.$o,$($m_SOURCES))) $($m_LIBADD)

am_out_files += $(lib_LTLIBRARIES) $(noinst_LTLIBRARIES)
am_sys_files += $(addprefix $(libdir)/,$(lib_LTLIBRARIES))
endef

$(topoutdir)/config.mk: $(topoutdir)/config.status $(topsrcdir)/config.mk.in
	cd $(topoutdir) && ./config.status

include $(topsrcdir)/am-pretty.mk
include $(topsrcdir)/am-tools.mk
