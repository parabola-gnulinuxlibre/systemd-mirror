$(outdir)/%.o: $(srcdir)/%.c | $(outdir)/.deps
	$(AM_V_CC)$(COMPILE)   -c -o $@ $<

$(outdir)/%.lo: $(srcdir)/%.c | $(outdir)/.deps
	$(AM_V_CC)$(LTCOMPILE) -c -o $@ $<

$(outdir)/.deps:
	$(AM_V_at)$(MKDIR_P) $@

$(outdir)/%.la:
	$(AM_V_CCLD)$(LINK) $(filter-out .var%,$^)
