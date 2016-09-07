mod.am.description = (systemd) Automake-to-Autothing magic
mod.am.depends += gnuconf

am.sys2out_DATA = \
        $(notdir \
        $(patsubst $(pamconfdir)/%,%.pam,\
        $(patsubst $(tmpfilesdir)/%.conf,%.tmpfiles,\
        $(patsubst $(sysusersdir)/%.conf,%.sysusers,\
        $(patsubst $(sysctldir)/%.conf,%.sysctl,\
        $(patsubst $(bashcompletiondir)/%,%.completion.bash,\
        $(patsubst $(zshcompletiondir)/_%,%.completion.zsh,\
        $1)))))))
am.sys2out_HEADERS = $(abspath $(addprefix $(srcdir)/include/,$(notdir $1)))

am.var_PROGRAMS    = $1_SOURCES nodist_$1_SOURCES $1_CFLAGS $1_CPPFLAGS $1_LDFLAGS $1_LDADD
am.var_LTLIBRARIES = $1_SOURCES nodist_$1_SOURCES $1_CFLAGS $1_CPPFLAGS $1_LDFLAGS $1_LIBADD

# So these are reasonable defaults, to keep my sanity.  They get overridden by
# `libtool`/`AM_V_*`-aware versions in `*-sd.mk`
am.INSTALL_PROGRAMS    ?= $(INSTALL_PROGRAM) $< $@
am.INSTALL_SCRIPTS     ?= $(INSTALL) $< $@
am.INSTALL_LTLIBRARIES ?= $(INSTALL) $< $@
am.INSTALL_DATA        ?= $(INSTALL_DATA) $< $@
am.INSTALL_HEADERS     ?= $(INSTALL_DATA) $< $@
am.INSTALL_MANS        ?= $(INSTALL_DATA) $< $@

am.LDFLAGS =

########################################################################

# this list of primaries is based on the Automake 1.15 manual
am.primaries ?= PROGRAMS LIBRARIES LTLIBRARIES LISP PYTHON JAVA SCRIPTS DATA HEADERS MANS TEXINFOS
$(eval $(foreach p,$(am.primaries),am.sys2out_$p ?= $$(notdir $$1)$(at.nl)))

am.primary2dirs = $(filter $(patsubst %dir,%,$(filter %dir,$(.VARIABLES))),\
                           $(patsubst nodist_%,%,$(patsubst dist_%,%,$(patsubst %_$1,%,$(filter %_$1,$(.VARIABLES))))))
am.sys2dirs = $(sort $(patsubst %/,%,$(dir $(foreach p,$(am.primaries),$(am.sys_$p)))))

am.file2var = $(subst -,_,$(subst .,_,$1))
am.file2sources  = $(addprefix $(srcdir)/,$(notdir $($(am.file2var)_SOURCES)))
am.file2sources += $(addprefix $(outdir)/,$(notdir $(nodist_$(am.file2var)_SOURCES)))
am.file2.o  = $(patsubst $(srcdir)/%,$(outdir)/%,$(patsubst %.c,%.o ,$(filter %.c,$(am.file2sources))))
am.file2.lo = $(patsubst %.o,%.lo,$(am.file2.o))
am.file2lib = $(foreach l,   $($(am.file2var)_$2),$(if $(filter lib%.la,$l), $($(l:.la=).DEPENDS)  , $l ))
am.file2cpp = $(foreach l,$1 $($(am.file2var)_$2),$(if $(filter lib%.la,$l), $($(l:.la=).CPPFLAGS) ,    ))

define _am.per_primary
$(foreach d,$(call am.primary2dirs,$1),$d_$1 ?=$(at.nl)dist_$d_$1 ?=$(at.nl)nodist_$d_$1 ?=$(at.nl))
noinst_$1 ?=
check_$1 ?=

am.sys_$1 := $(foreach d,$(call am.primary2dirs,$1),$$(addprefix $$($ddir)/,$$(notdir $$($d_$1) $$(dist_$d_$1) $$(nodist_$d_$1))))
am.out_$1 := $$(call am.sys2out_$1,$(foreach d,$(call am.primary2dirs,$1),$$(addprefix $$($ddir)/,$$(notdir $$($d_$1) $$(nodist_$d_$1) ))) $$(noinst_$1))
am.check_$1 := $$(check_$1)
$(foreach d,$(call am.primary2dirs,$1),undefine $d_$1$(at.nl)undefine dist_$d_$1$(at.nl)undefine nodist_$d_$1$(at.nl))
undefine noinst_$1
undefine check_$1
$$(addprefix $$(DESTDIR),$$(am.sys_$1)): private am.INSTALL = $$(am.INSTALL_$1)
endef
########################################################################
# TODO: I'm not in love with how _am.per_PROGRAM figures out am.subdirs
# $1 = filename
# $2 = varname
define _am.per_PROGRAM
$(foreach var,_am.depends $(call am.var_PROGRAMS,$2),$(var) ?=$(at.nl))
_am.depends += $$(call at.path,$$(call am.file2.o,$1)  $$(call am.file2lib,$1,LDADD))
am.CPPFLAGS +=                 $$($2_CPPFLAGS)         $$(call am.file2cpp,$1,LDADD)
am.CFLAGS   +=                 $$($2_CFLAGS)
$$(outdir)/$1: private am.LDFLAGS := $$($2_LDFLAGS)
$$(outdir)/$1: $$(_am.depends)
am.subdirs := $$(sort $$(am.subdirs)\
                      $$(filter-out $$(abspath $$(srcdir)),\
                                    $$(abspath $$(dir $$(filter-out -l% /%,$$(_am.depends))))))
am.CPPFLAGS := $$(am.CPPFLAGS)
am.CFLAGS := $$(am.CFLAGS)
$(foreach var,_am.depends $(call am.var_LTLIBRARIES,$2),undefine $(var)$(at.nl))
endef
########################################################################
# TODO: I'm not in love with how _am.per_LTLIBRARY figures out am.subdirs
# $1 = filename
# $2 = varname
define _am.per_LTLIBRARY
$(foreach var,_am.depends $(call am.var_LTLIBRARIES,$2),$(var) ?=$(at.nl))
_am.depends += $$(call at.path,$$(call am.file2.lo,$1) $$(call am.file2lib,$1,LIBADD))
am.CPPFLAGS +=                 $$($2_CPPFLAGS)         $$(call am.file2cpp,$1,LIBADD)
am.CFLAGS   +=                 $$($2_CFLAGS)
$$(outdir)/$1: private am.LDFLAGS := $$($2_LDFLAGS)
$$(outdir)/$1: $$(_am.depends)
am.subdirs := $$(sort $$(am.subdirs)\
                      $$(filter-out $$(abspath $$(srcdir)),\
                                    $$(abspath $$(dir $$(filter-out -l% /%,$$(_am.depends))))))
am.CPPFLAGS := $$(am.CPPFLAGS)
am.CFLAGS := $$(am.CFLAGS)
$(foreach var,_am.depends $(call am.var_LTLIBRARIES,$2),undefine $(var)$(at.nl))
endef
########################################################################
define _am.per_directory
$$(DESTDIR)$1/%: $$(outdir)/%
	@$$(NORMAL_INSTALL)
	$$(am.INSTALL)
$$(DESTDIR)$1/%: $$(srcdir)/%
	@$$(NORMAL_INSTALL)
	$$(am.INSTALL)
endef
########################################################################
define _am.per_include_directory
$$(DESTDIR)$1/%: $$(outdir)/include/%
	@$$(NORMAL_INSTALL)
	$$(am.INSTALL)
$$(DESTDIR)$1/%: $$(srcdir)/include/%
	@$$(NORMAL_INSTALL)
	$$(am.INSTALL)
endef
