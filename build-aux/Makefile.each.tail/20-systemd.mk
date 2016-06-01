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
