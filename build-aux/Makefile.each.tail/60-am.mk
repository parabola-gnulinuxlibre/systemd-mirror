mod.am.depends += files

rootbin_PROGRAMS ?=
bin_PROGRAMS ?=
bin_SCRIPTS ?=
bashcompletion_DATA ?=
zshcompletion_DATA ?=
dist_bashcompletion_DATA := $(sort $(bashcompletion_DATA) $(rootbin_PROGRAMS) $(bin_PROGRAMS) $(bin_SCRIPTS))
dist_zshcompletion_DATA := $(sort $(zshcompletion_DATA) $(addprefix _,$(rootbin_PROGRAMS) $(bin_PROGRAMS) $(bin_SCRIPTS)))

man_MANS ?=
_am.man_MANS := $(man_MANS)
undefine man_MANS
man0_MANS += $(foreach _am.tmp,$(_am.man_MANS),$(if $(findstring .0,$(suffix $(_am.tmp))),$(_am.tmp)))
man1_MANS += $(foreach _am.tmp,$(_am.man_MANS),$(if $(findstring .1,$(suffix $(_am.tmp))),$(_am.tmp)))
man2_MANS += $(foreach _am.tmp,$(_am.man_MANS),$(if $(findstring .2,$(suffix $(_am.tmp))),$(_am.tmp)))
man3_MANS += $(foreach _am.tmp,$(_am.man_MANS),$(if $(findstring .3,$(suffix $(_am.tmp))),$(_am.tmp)))
man4_MANS += $(foreach _am.tmp,$(_am.man_MANS),$(if $(findstring .4,$(suffix $(_am.tmp))),$(_am.tmp)))
man5_MANS += $(foreach _am.tmp,$(_am.man_MANS),$(if $(findstring .5,$(suffix $(_am.tmp))),$(_am.tmp)))
man6_MANS += $(foreach _am.tmp,$(_am.man_MANS),$(if $(findstring .6,$(suffix $(_am.tmp))),$(_am.tmp)))
man7_MANS += $(foreach _am.tmp,$(_am.man_MANS),$(if $(findstring .7,$(suffix $(_am.tmp))),$(_am.tmp)))
man8_MANS += $(foreach _am.tmp,$(_am.man_MANS),$(if $(findstring .8,$(suffix $(_am.tmp))),$(_am.tmp)))
man9_MANS += $(foreach _am.tmp,$(_am.man_MANS),$(if $(findstring .9,$(suffix $(_am.tmp))),$(_am.tmp)))
manl_MANS += $(foreach _am.tmp,$(_am.man_MANS),$(if $(findstring .l,$(suffix $(_am.tmp))),$(_am.tmp)))
mann_MANS += $(foreach _am.tmp,$(_am.man_MANS),$(if $(findstring .n,$(suffix $(_am.tmp))),$(_am.tmp)))

$(eval \
  $(foreach p,$(am.primaries)  ,$(call _am.per_primary,$p)$(at.nl)))
$(eval \
  $(foreach f,$(am.out_PROGRAMS)   ,$(call _am.per_PROGRAM,$f,$(call am.file2var,$f))$(at.nl))\
  $(foreach f,$(am.out_LTLIBRARIES),$(call _am.per_LTLIBRARY,$f,$(call am.file2var,$f))$(at.nl))\
  $(foreach d,$(am.sys2dirs)  ,$(call _am.per_directory,$d)$(at.nl)))

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

at.subdirs += $(am.subdirs)
files.sys.all += $(foreach p,$(am.primaries),$(am.sys_$p))
files.out.all += $(foreach p,$(am.primaries),$(am.out_$p))
files.out.check += $(foreach p,$(am.primaries),$(am.check_$p))
