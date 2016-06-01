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

-include $(outdir)/$(DEPDIR)/*.P*

std.clean_files += *.o *.lo .deps/ .libs/

$(outdir)/%.o: $(srcdir)/%.c $(topoutdir)/config.h | $(outdir)/.deps
	$(AM_V_CC)$(COMPILE)   -c -o $@ $<

$(outdir)/%.lo: $(srcdir)/%.c $(topoutdir)/config.h | $(outdir)/.deps
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $<

$(outdir)/.deps:
	$(AM_V_at)$(MKDIR_P) $@

$(outdir)/%.la:
	$(AM_V_CCLD)$(LINK) $(filter-out .var%,$^)

$(DESTDIR)$(libdir)/%.so: $(outdir)/%.la
	$(LIBTOOL) $(ALL_LIBTOOLFLAGS) --mode=install $(INSTALL) $(INSTALL_STRIP_FLAG) $< $(@D)

$(outdir)/%-from-name.gperf: $(srcdir)/%-list.txt
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "struct $(notdir $*)_name { const char* name; int id; };"; print "%null-strings"; print "%%";} { printf "%s, %s\n", $$1, $$1 }' <$< >$@

$(outdir)/%-from-name.h: $(srcdir)/%-from-name.gperf
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GPERF)$(GPERF) -L ANSI-C -t --ignore-case -N lookup_$(notdir $*) -H hash_$(notdir $*)_name -p -C <$< >$@

$(outdir)/%: $(srcdir)/%.in
	$(SED_PROCESS)

$(outdir)/%.sh: $(srcdir)/%.sh.in
	$(SED_PROCESS)
	$(AM_V_GEN)chmod +x $@

$(outdir)/%.c: $(srcdir)/%.gperf
	$(AM_V_GPERF)$(GPERF) < $< > $@

$(outdir)/%: $(srcdir)/%.m4 $(top_builddir)/config.status
	$(AM_V_M4)$(M4) -P $(M4_DEFINES) < $< > $@
