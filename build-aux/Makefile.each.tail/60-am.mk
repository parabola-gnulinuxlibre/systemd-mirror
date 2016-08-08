mod.am.depends += files

########################################################################
_am.primary2dirs = $(filter $(patsubst %dir,%,$(filter %dir,$(.VARIABLES))),\
                            $(patsubst %_$1,%,$(filter %_$1,$(.VARIABLES))))
define _am.per_primary
noinst_$p ?=
check_$p ?=

am.inst_$p := $$(foreach d,$$(call _am.primary2dirs,$p),$$($$d_$p))
am.noinst_$p := $$(noinst_$p)
am.check_$p := $$(check_$p)
$(foreach d,$(call _am.primary2dirs,$p) nointt check,undefine $d_$p$(at.nl))
ifneq ($$(am.inst_$p),)
$$(am.inst_$p): private am.INSTALL = $$(am.INSTALL_$p)
endif
am.$p = $$(am.check_$p) $$(am.noinst_$p) $$(call am.inst2noinst_$p,$$(am.inst_$p))
endef
$(foreach p,$(am.primaries),$(eval $(_am.per_primary)))
########################################################################
files.sys.all += $(foreach p,$(am.primaries),$(am.inst_$p))
files.out.all += $(foreach p,$(am.primaries),$(am.noinst_$p))
files.out.all += $(foreach p,$(am.primaries),$(call am.inst2noinst_$p,$(am.inst_$p)))
files.out.check += $(foreach p,$(am.primaries),$(am.check_$p))
########################################################################
# TODO: I'm not in love with how _am.per_PROGRAM figures out at.subdirs
define _am.per_PROGRAM
$$(foreach var,_am.depends $$(call am.var_PROGRAMS,$v),$$(eval $$(var) ?=))
_am.depends += $$(call at.path,$$(call am.file2.o,$f)  $$(call am.file2lib,$f,LDADD))
am.CPPFLAGS +=                 $$($v_CPPFLAGS)         $$(call am.file2cpp,$f,LDADD)
am.CFLAGS   +=                 $$($v_CFLAGS)
$$(outdir)/$f: private ALL_LDFLAGS += $$($v_LDFLAGS)
$$(outdir)/$f: $$(_am.depends)
am.subdirs := $$(sort $$(am.subdirs)\
                      $$(filter-out $$(abspath $$(srcdir)),\
                                    $$(abspath $$(dir $$(filter-out -l% /%,$$(_am.depends))))))
$$(foreach var,_am.depends $$(call am.var_PROGRAMS,$v),$$(eval undefine $$(var)))
endef
$(foreach f,$(am.PROGRAMS),$(foreach v,$(call am.file2var,$f),$(eval $(_am.per_PROGRAM))))
########################################################################
# TODO: I'm not in love with how _am.per_LTLIBRARY figures out at.subdirs
define _am.per_LTLIBRARY
$$(foreach var,_am.depends $$(call am.var_LTLIBRARIES,$v),$$(eval $$(var) ?=))
_am.depends += $$(call at.path,$$(call am.file2.lo,$f) $$(call am.file2lib,$f,LIBADD))
am.CPPFLAGS +=                 $$($v_CPPFLAGS)         $$(call am.file2cpp,$f,LIBADD)
am.CFLAGS   +=                 $$($v_CFLAGS)
$$(outdir)/$f: private ALL_LDFLAGS += $$($v_LDFLAGS)
$$(outdir)/$f: $$(_am.depends)
am.subdirs := $$(sort $$(am.subdirs)\
                      $$(filter-out $$(abspath $$(srcdir)),\
                                    $$(abspath $$(dir $$(filter-out -l% /%,$$(_am.depends))))))
$$(foreach var,_am.depends $$(call am.var_LTLIBRARIES,$v),$$(eval undefine $$(var)))
endef
$(foreach f,$(am.LTLIBRARIES),$(foreach v,$(call am.file2var,$f),$(eval $(_am.per_LTLIBRARY))))
########################################################################
define _am.per_directory
$$(DESTDIR)$d/%: $$(outdir)/%
	@$$(NORMAL_INSTALL)
	$$(am.INSTALL)
$$(DESTDIR)$d/%: $$(srcdir)/%
	@$$(NORMAL_INSTALL)
	$$(am.INSTALL)
endef
$(foreach d,$(sort $(dir $(foreach p,$(am.primaries),$(am.inst_$p)))),$(eval $(_am.per_directory)))
