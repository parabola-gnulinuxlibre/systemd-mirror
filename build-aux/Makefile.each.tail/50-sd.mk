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

mod.sd.depends += files am lt

-include $(wildcard $(outdir)/$(DEPDIR)/*.P*)

files.out.int += *.o *.lo *.so .deps/ .libs/
files.out.int += *-list.txt
files.out.int += *-from-name.gperf
files.out.int += *-from-name.h
files.out.int += *-to-name.h
files.out.int += *-gperf.c

$(outdir)/%.o : $(srcdir)/%.c $(topoutdir)/config.h | $(outdir)/$(DEPDIR); $(AM_V_CC)$(sd.COMPILE)   -c -o $@ $<
$(outdir)/%.o : $(outdir)/%.c $(topoutdir)/config.h | $(outdir)/$(DEPDIR); $(AM_V_CC)$(sd.COMPILE)   -c -o $@ $<
$(outdir)/%.lo: $(srcdir)/%.c $(topoutdir)/config.h | $(outdir)/$(DEPDIR); $(AM_V_CC)$(sd.LTCOMPILE) -c -o $@ $<
$(outdir)/%.lo: $(outdir)/%.c $(topoutdir)/config.h | $(outdir)/$(DEPDIR); $(AM_V_CC)$(sd.LTCOMPILE) -c -o $@ $<

$(outdir)/$(DEPDIR):
	$(AM_V_at)$(MKDIR_P) $@

$(outdir)/%.la:
	@if test $(words $(lt.lib.files.all)) = 0; then echo 'Cannot link library with no dependencies: $@' >&2; exit 1; fi
	@if test $(origin am.LDFLAGS) = undefined; then echo 'Cannot link library with undefined am.LDFLAGS: $@' >&2; exit 1; fi
	$(AM_V_CCLD)$(sd.LINK) $(if $(lt.lib.rpath),-rpath $(lt.lib.rpath)) $(lt.lib.files.ld)
	$(AM_V_at)$(lt.lib.post)
$(addprefix $(outdir)/,$(am.out_PROGRAMS)): $(outdir)/%:
	@if test $(words $(lt.exe.files.all)) = 0; then echo 'Cannot link executable with no dependencies: $@' >&2; exit 1; fi
	@if test $(origin am.LDFLAGS) = undefined; then echo 'Cannot link executable with undefined am.LDFLAGS: $@' >&2; exit 1; fi
	$(AM_V_CCLD)$(sd.LINK) $(lt.exe.files.ld)

# Stupid test that everything purported to be exported really is
$(outdir)/test-lib%-sym.c: $(srcdir)/lib%.sym
	$(AM_V_GEN){\
		printf '#include <stdio.h>\n' && \
		printf '#include "%s"\n' $(notdir $(filter %.h, $^)) && \
		printf 'void* functions[] = {\n' && \
		sed -r -n 's/^ +([a-zA-Z0-9_]+);/\1,/p' $< && \
		printf '};\nint main(void) {\n' && \
		printf 'unsigned i; for (i=0;i<sizeof(functions)/sizeof(void*);i++) printf("%%p\\n", functions[i]);\n' && \
		printf 'return 0; }\n' && \
	:; } > $@

_sd.files_in = $(foreach f,$(files.sys),$(if $(filter $1,$(patsubst %/,%,$(dir $f))),$(DESTDIR)$f))

$(outdir)/%-from-name.gperf: $(outdir)/%-list.txt
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "struct $(notdir $*)_name { const char* name; int id; };"; print "%null-strings"; print "%%";} { printf "%s, %s\n", $$1, $$1 }' <$< >$@

$(outdir)/%-from-name.h: $(outdir)/%-from-name.gperf
	$(AM_V_GPERF)$(GPERF) -L ANSI-C -t --ignore-case -N lookup_$(notdir $*) -H hash_$(notdir $*)_name -p -C <$< >$@

ifeq ($(sd.sed_files),)
EXTRA_DIST ?=
sd.sed_files += $(notdir $(patsubst %.in,%,$(filter %.in,$(EXTRA_DIST))))
endif
ifneq ($(sd.sed_files),)
$(addprefix $(outdir)/,$(sd.sed_files)): $(outdir)/%: $(srcdir)/%.in
	$(sd.SED_PROCESS)
endif

#$(outdir)/%.sh: $(srcdir)/%.sh.in
#	$(SED_PROCESS)
#	$(AM_V_GEN)chmod +x $@

$(addprefix $(DESTDIR),$(INSTALL_DIRS)): %:
	mkdir -p -- $@

$(outdir)/%.c: $(srcdir)/%.gperf
	$(AM_V_GPERF)$(GPERF) < $< > $@
$(outdir)/%.c: $(outdir)/%.gperf
	$(AM_V_GPERF)$(GPERF) < $< > $@

$(addprefix $(outdir)/,$(patsubst %.m4,%,$(filter %.m4,$(files.src)))): $(outdir)/%: $(srcdir)/%.m4 $(top_builddir)/config.status
	$(AM_V_M4)$(M4) -P $(M4_DEFINES) < $< > $@
$(addprefix $(outdir)/,$(patsubst %.m4.in,%,$(filter %.m4.in,$(files.src)))): $(outdir)/%: $(outdir)/%.m4 $(top_builddir)/config.status
	$(AM_V_M4)$(M4) -P $(M4_DEFINES) < $< > $@
