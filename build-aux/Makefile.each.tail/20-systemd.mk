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

-include $(wildcard $(outdir)/$(DEPDIR)/*.P*)

std.clean_files += *.o *.lo *.so .deps/ .libs/
std.clean_files += *-list.txt
std.clean_files += *-from-name.gperf
std.clean_files += *-from-name.h
std.clean_files += *-to-name.h
std.clean_files += *-gperf.c

$(outdir)/%.o : $(srcdir)/%.c $(topoutdir)/config.h | $(outdir)/.deps; $(AM_V_CC)$(COMPILE)   -c -o $@ $<
$(outdir)/%.o : $(outdir)/%.c $(topoutdir)/config.h | $(outdir)/.deps; $(AM_V_CC)$(COMPILE)   -c -o $@ $<
$(outdir)/%.lo: $(srcdir)/%.c $(topoutdir)/config.h | $(outdir)/.deps; $(AM_V_CC)$(LTCOMPILE) -c -o $@ $<
$(outdir)/%.lo: $(outdir)/%.c $(topoutdir)/config.h | $(outdir)/.deps; $(AM_V_CC)$(LTCOMPILE) -c -o $@ $<

$(outdir)/.deps:
	$(AM_V_at)$(MKDIR_P) $@

_systemd.dups = $(sort $(foreach l,$1,$(if $(filter-out 1,$(words $(filter $l,$1))),$l)))
_systemd.patsubst-all = $(if $1,$(call _systemd.patsubst-all,$(wordlist 2,$(words $1),$1),$2,$(patsubst $(firstword $1),$2,$3)),$3)
_systemd.lt_libs = $(foreach l,$(filter %.la,$1), $l $(call _systemd.lt_libs,$($(notdir $l).DEPENDS)))
_systemd.lt_filter = $(filter-out $(call _systemd.dups,$(call _systemd.lt_libs,$1)),$1)

_systemd.rpath = $(dir $(patsubst $(DESTDIR)%,%,$(filter %/$(@F),$(std.sys_files/$(@D)))))
_systemd.link_files = $(call _systemd.lt_filter,$(filter %.o %.lo %.la,$^)) $(call _systemd.patsubst-all,$(.LIBPATTERNS),-l%,$(filter $(.LIBPATTERNS),$(notdir $^)))
$(outdir)/%.la:
	@if test $(words $^) = 0; then echo 'Cannot link library with no dependencies: $@' >&2; exit 1; fi
	$(AM_V_CCLD)$(LINK) $(if $(_systemd.rpath),-rpath $(_systemd.rpath)) $(_systemd.link_files)
$(addprefix $(outdir)/,$(foreach d,$(am.bindirs),$($d_PROGRAMS))): $(outdir)/%:
	@if test $(words $^) = 0; then echo 'Cannot link executable with no dependencies: $@' >&2; exit 1; fi
	$(AM_V_CCLD)$(LINK) $(_systemd.link_files)

_systemd.in_destdir = $(foreach f,$(std.sys_files),$(if $(filter $1,$(patsubst %/,%,$(dir $f))),$(DESTDIR)$f))

define install_bindir
$(call _systemd.in_destdir,$(bindir)): $(DESTDIR)$(bindir)/%: $(outdir)/%
	@$(NORMAL_INSTALL)
	$(AM_V_PROG)$(LIBTOOL) $(AM_V_lt) --tag=CC $(SYS_LIBTOOLFLAGS) --mode=install $(INSTALL_PROGRAM) $< $@
endef
$(foreach bindir,$(sort $(foreach d,$(am.bindirs),$($ddir))),$(eval $(value install_bindir)))

define install_libdir
$(call _systemd.in_destdir,$(libdir)): $(DESTDIR)$(libdir)/%.la: $(outdir)/%.la
	@$(NORMAL_INSTALL)
	$(AM_V_LIB)$(LIBTOOL) $(AM_V_lt) --tag=CC $(SYS_LIBTOOLFLAGS) --mode=install $(INSTALL) $< $@
endef
$(foreach libdir,$(sort $(foreach d,lib rootlib,$($ddir))),$(eval $(value install_libdir)))

define install_datadir
$(call _systemd.in_destdir,$(datadir)): $(DESTDIR)$(datadir)/%: $(outdir)/%
	@$(NORMAL_INSTALL)
	$(AM_V_DATA)$(INSTALL_DATA) $< $@
endef
$(foreach datadir,$(sort $(foreach d,pkgconfigdata pkgconfiglib bootlib,$($ddir))),$(eval $(value install_datadir)))

$(outdir)/%-from-name.gperf: $(outdir)/%-list.txt
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "struct $(notdir $*)_name { const char* name; int id; };"; print "%null-strings"; print "%%";} { printf "%s, %s\n", $$1, $$1 }' <$< >$@

$(outdir)/%-from-name.h: $(outdir)/%-from-name.gperf
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GPERF)$(GPERF) -L ANSI-C -t --ignore-case -N lookup_$(notdir $*) -H hash_$(notdir $*)_name -p -C <$< >$@

$(addprefix $(outdir)/,$(systemd.sed_files)): $(outdir)/%: $(srcdir)/%.in
	$(SED_PROCESS)

#$(outdir)/%.sh: $(srcdir)/%.sh.in
#	$(SED_PROCESS)
#	$(AM_V_GEN)chmod +x $@

$(outdir)/%.c: $(srcdir)/%.gperf
	$(AM_V_GPERF)$(GPERF) < $< > $@
$(outdir)/%.c: $(outdir)/%.gperf
	$(AM_V_GPERF)$(GPERF) < $< > $@

$(outdir)/%: $(srcdir)/%.m4 $(top_builddir)/config.status
	$(AM_V_M4)$(M4) -P $(M4_DEFINES) < $< > $@
