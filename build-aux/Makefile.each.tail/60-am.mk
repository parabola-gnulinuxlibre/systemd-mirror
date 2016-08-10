mod.am.depends += files

bin_PROGRAMS ?=
bin_SCRIPTS ?=
bashcompletion_DATA ?=
zshcompletion_DATA ?=
bashcompletion_DATA := $(sort $(bashcompletion_DATA) $(bin_PROGRAMS) $(bin_SCRIPTS))
zshcompletion_DATA := $(sort $(zshcompletion_DATA) $(addprefix _,$(bin_PROGRAMS) $(bin_SCRIPTS)))

$(eval \
  $(foreach p,$(am.primaries)  ,$(call _am.per_primary,$p)$(at.nl)))
$(eval \
  $(foreach f,$(am.PROGRAMS)   ,$(call _am.per_PROGRAM,$f,$(call am.file2var,$f))$(at.nl))\
  $(foreach f,$(am.LTLIBRARIES),$(call _am.per_LTLIBRARY,$f,$(call am.file2var,$f))$(at.nl))\
  $(foreach d,$(am.inst2dirs)  ,$(call _am.per_directory,$d)$(at.nl)))

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

at.subdirs += $(am.subdirs)
files.sys.all += $(foreach p,$(am.primaries),$(am.inst_$p))
files.out.all += $(foreach p,$(am.primaries),$(am.noinst_$p))
files.out.all += $(foreach p,$(am.primaries),$(call am.inst2noinst_$p,$(am.inst_$p)))
files.out.check += $(foreach p,$(am.primaries),$(am.check_$p))
