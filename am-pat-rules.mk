$(outdir)/%.o: $(srcdir)/%.c | $(outdir)/.deps
	$(AM_V_CC)depbase=`echo $@ | sed 's|[^/]*$$|$(DEPDIR)/&|;s|\.o$$||'`;\
	$(COMPILE) -MT $@ -MD -MP -MF $$depbase.Tpo -c -o $@ $< &&\
	$(am__mv) $$depbase.Tpo $$depbase.Po

$(outdir)/%.lo: $(srcdir)/%.c | $(outdir)/.deps
	$(AM_V_CC)depbase=`echo $@ | sed 's|[^/]*$$|$(DEPDIR)/&|;s|\.lo$$||'`;\
	$(LTCOMPILE) -MT $@ -MD -MP -MF $$depbase.Tpo -c -o $@ $< &&\
	$(am__mv) $$depbase.Tpo $$depbase.Plo

$(outdir)/.deps:
	$(AM_V_at)$(MKDIR_P) $@

$(outdir)/%.la:
	$(AM_V_CCLD)$(LINK) $^
