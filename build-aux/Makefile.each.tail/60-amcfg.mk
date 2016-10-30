$(DESTDIR)$(includedir)/%.h: $(srcdir)/include/%.h
	@$(NORMAL_INSTALL)
	$(am.INSTALL)

$(DESTDIR)$(sysusersdir)/%.conf: $(srcdir)/%.sysusers
	@$(NORMAL_INSTALL)
	$(am.INSTALL)
$(DESTDIR)$(sysusersdir)/%.conf: $(outdir)/%.sysusers
	@$(NORMAL_INSTALL)
	$(am.INSTALL)

$(DESTDIR)$(sysctldir)/%.conf: $(srcdir)/%.sysctl
	@$(NORMAL_INSTALL)
	$(am.INSTALL)
$(DESTDIR)$(sysctldir)/%.conf: $(outdir)/%.sysctl
	@$(NORMAL_INSTALL)
	$(am.INSTALL)

$(DESTDIR)$(tmpfilesdir)/%.conf: $(srcdir)/%.tmpfiles
	@$(NORMAL_INSTALL)
	$(am.INSTALL)
$(DESTDIR)$(tmpfilesdir)/%.conf: $(outdir)/%.tmpfiles
	@$(NORMAL_INSTALL)
	$(am.INSTALL)

$(DESTDIR)$(pamconfdir)/%: $(srcdir)/%.pam
	@$(NORMAL_INSTALL)
	$(am.INSTALL)
$(DESTDIR)$(pamconfdir)/%: $(outdir)/%.pam
	@$(NORMAL_INSTALL)
	$(am.INSTALL)

$(DESTDIR)$(bashcompletiondir)/%: $(srcdir)/%.completion.bash
	@$(NORMAL_INSTALL)
	$(am.INSTALL)
$(DESTDIR)$(bashcompletiondir)/%: $(outdir)/%.completion.bash
	@$(NORMAL_INSTALL)
	$(am.INSTALL)

$(DESTDIR)$(zshcompletiondir)/_%: $(srcdir)/%.completion.zsh
	@$(NORMAL_INSTALL)
	$(am.INSTALL)
$(DESTDIR)$(zshcompletiondir)/_%: $(outdir)/%.completion.zsh
	@$(NORMAL_INSTALL)
	$(am.INSTALL)
