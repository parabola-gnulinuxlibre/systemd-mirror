mod.am.description = (systemd) Automake-to-Autothing magic
mod.am.depends += gnuconf
define mod.am.doc
# Because many of the inputs/outpus are repetative, they are defined
# here in terms of $$(primary), which may be any of the supported
# Automake primaries (see below for a table).
#
#    | Automake 1.15 primaries |
#    +-------------------------+
#    | name        | supported |
#    +-------------+-----------+
#    | PROGRAMS    | yes       |
#    | LIBRARIES   | no        |
#    | LTLIBRARIES | yes       |
#    | LISP        | no        |
#    | PYTHON      | no        |
#    | JAVA        | no        |
#    | SCRIPTS     | yes       |
#    | DATA        | yes       |
#    | HEADERS     | yes       |
#    | MANS        | yes       |
#    | TEXINFOS    | no        |
#
# Further, there is also $$(dirname), which could be anything; and is
# detected at runtime by inspecting $$(.VARIABLES) to find every possible
# matching value.  See `_am.primary2dirs` for how this is done.
#
# Some inputs are "erased" after the pass they are used in.  This means
# that they are `undefine`ed.
#
$(value _am.pass0.doc)
#
$(value _am.pass1.doc)
#
$(value _am.pass2.doc)
#
$(value _am.pass3.doc)
#
$(value _am.pass4.doc)
#
$(value _am.pass5.doc)
endef

_am.primaries =
_am.primaries += PROGRAMS
#_am.primaries += LIBRARIES
_am.primaries += LTLIBRARIES
#_am.primaries += LISP
#_am.primaries += PYTHON
#_am.primaries += JAVA
_am.primaries += SCRIPTS
_am.primaries += DATA
_am.primaries += HEADERS
_am.primaries += MANS
#_am.primaries += TEXINFOS

# Used by the per_PROGRAM and per_LTLIBRARY passes
_am.file2var = $(subst -,_,$(subst .,_,$1))
_am.file2sources  = $(addprefix $(srcdir)/,$(notdir $($(_am.file2var)_SOURCES)))
_am.file2sources += $(addprefix $(outdir)/,$(notdir $(nodist_$(_am.file2var)_SOURCES)))
_am.file2.o  = $(patsubst $(srcdir)/%,$(outdir)/%,$(patsubst %.c,%.o ,$(filter %.c,$(_am.file2sources))))
_am.file2.lo = $(patsubst %.o,%.lo,$(_am.file2.o))
_am.file2lib = $(foreach l,   $($(_am.file2var)_$2),$(if $(filter lib%.la,$l), $($(l:.la=).DEPENDS)  , $l ))
_am.file2cpp = $(foreach l,$1 $($(_am.file2var)_$2),$(if $(filter lib%.la,$l), $($(l:.la=).CPPFLAGS) ,    ))

define _am.pass0.doc
# == Pass 0: man_MANS ==
# Erased inputs:
#   - Directory variable: `man_MANS`
# Outputs:
#   - Directory variable: `man$n_MANS` for $n in `{0..9} n l`
#
# Split man_MANS into man$n_MANS 
endef
define _am.pass0
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
endef

define _am.pass1.doc
# == Pass 1: _am.per_primary ==
# Inputs:
#   - Global variable    : `am.INSTALL_$(primary)`
#   - Global variable    : `am.sys2out_$(primary)`
#   - Global variable    : `$(dirname)dir`
# Erased inputs:
#   - Directory variable : `$(dirname)_$(primary)` [1] [2]
#   - Directory variable : `dist_$(dirname)_$(primary)` [1] [2]
#   - Directory variable : `nodist_$(dirname)_$(primary)` [1] [2]
#   - Directory variable : `noinst_$(primary)` [2]
#   - Directory variable : `check_$(primary)` [2]
# Outputs:
#   - Directory variable : `am.sys_$(primary)`
#   - Directory variable : `am.out_$(primary)`
#   - Directory variable : `am.check_$(primary)`
#   - Target variable    : `am.INSTALL`
#
# [1]: HACK: Each of these is first passed through `$(dir ...)`.
#
# [2]: HACK: For `am.out_*` each of these are turned into
#      $(DESTDIR)-relative paths (ie, as if for `am.sys_*`), then turned
#      back into $(outdir)-relative paths with `$(call
#      am.sys2out_$(primary),...)`.
endef

# Default values
am.INSTALL_PROGRAMS    ?= $(INSTALL_PROGRAM) $< $@
am.INSTALL_SCRIPTS     ?= $(INSTALL) $< $@
am.INSTALL_LTLIBRARIES ?= $(INSTALL) $< $@
am.INSTALL_DATA        ?= $(INSTALL_DATA) $< $@
am.INSTALL_HEADERS     ?= $(INSTALL_DATA) $< $@
am.INSTALL_MANS        ?= $(INSTALL_DATA) $< $@
$(eval $(foreach p,$(_am.primaries),am.sys2out_$p ?= $$(notdir $$1)$(at.nl)))

# Utility functions
_am.primary2dirs = $(filter $(patsubst %dir,%,$(filter %dir,$(.VARIABLES))),\
                            $(patsubst nodist_%,%,$(patsubst dist_%,%,$(patsubst %_$1,%,$(filter %_$1,$(.VARIABLES))))))

_am.pass1 = $(eval $(foreach p,$(_am.primaries)  ,$(call _am.per_primary,$p)$(at.nl)))
define _am.per_primary
# Initialize input variables
$(foreach d,$(call _am.primary2dirs,$1),\
  $d_$1        ?=$(at.nl)\
  dist_$d_$1   ?=$(at.nl)\
  nodist_$d_$1 ?=$(at.nl))
noinst_$1 ?=
check_$1  ?=

# Directory variable outputs
am.check_$1 := $$(check_$1)
am.sys_$1 :=                       $(foreach d,$(call _am.primary2dirs,$1),$$(addprefix $$($ddir)/,$$(notdir $$($d_$1) $$(dist_$d_$1) $$(nodist_$d_$1))))
am.out_$1 := $$(call am.sys2out_$1,$(foreach d,$(call _am.primary2dirs,$1),$$(addprefix $$($ddir)/,$$(notdir $$($d_$1)                $$(nodist_$d_$1)))) $$(noinst_$1))
#                                                                                                                                                     ^^^              ^
#                                                                                                                                              notdir-'||              |
#                                                                                                                                           addprefix--'|              |
#                                                                                                                                           foreach d---'              |
#                                                                                                                                          am.sys2out------------------'

# Erase appropriate inputs
$(foreach d,$(call _am.primary2dirs,$1),undefine $d_$1$(at.nl)undefine dist_$d_$1$(at.nl)undefine nodist_$d_$1$(at.nl))
undefine noinst_$1
undefine check_$1

# Target variable outputs
$$(addprefix $$(DESTDIR),$$(am.sys_$1)): private am.INSTALL = $$(am.INSTALL_$1)
endef

define _am.pass2.doc
# == Pass 2: _am.per_PROGRAM ==
# Inputs:
#   - Directory variable : `am.out_PROGRAMS`
# Erased inputs:
#   - Directory variable : `$(program)_SOURCES`
#   - Directory variable : `nodist_$(program)_SOURCES`
#   - Directory variable : `$(program)_CFLAGS`
#   - Directory variable : `$(program)_CPPFLAGS`
#   - Directory variable : `$(program)_LDFLAGS`
#   - Directory variable : `$(program)_LDADD`
# Outputs:
#   - Directory variable : `am.CPPFLAGS`
#   - Directory variable : `am.CFLAGS`
#   - Target dependencies: `$(outdir)/$(program)`
#   - Target variable    : `$(outdir)/$(program): am.LDFLAGS`
#   - Directory variable : `am.subdirs`
#
# TODO: I'm not in love with how it figures out `am.subdirs`.
# TODO: I'm not in love with how it does the `install` dependencies.
endef
_am.pass2 = $(eval $(foreach f,$(am.out_PROGRAMS)   ,$(call _am.per_PROGRAM,$f,$(call _am.file2var,$f))$(at.nl)))
_am.var_PROGRAMS    = $1_SOURCES nodist_$1_SOURCES $1_CFLAGS $1_CPPFLAGS $1_LDFLAGS $1_LDADD
# $1 = filename
# $2 = varname
define _am.per_PROGRAM
$(foreach var,_am.depends $(call _am.var_PROGRAMS,$2),$(var) ?=$(at.nl))
_am.depends += $$(call at.path,$$(call _am.file2.o,$1)  $$(call _am.file2lib,$1,LDADD))
am.CPPFLAGS +=                 $$($2_CPPFLAGS)          $$(call _am.file2cpp,$1,LDADD)
am.CFLAGS   +=                 $$($2_CFLAGS)
$$(outdir)/$1: private am.LDFLAGS := $$($2_LDFLAGS)
$$(outdir)/$1: $$(_am.depends)
$$(outdir)/install: $$(addsuffix install,$$(dir $$(filter %.la,$$(_am.depends))))
am.subdirs := $$(sort $$(am.subdirs)\
                      $$(filter-out $$(abspath $$(srcdir)),\
                                    $$(abspath $$(dir $$(filter-out -l% /%,$$(_am.depends))))))
am.CPPFLAGS := $$(am.CPPFLAGS)
am.CFLAGS := $$(am.CFLAGS)
$(foreach var,_am.depends $(call _am.var_PROGRAMS,$2),undefine $(var)$(at.nl))
endef

define _am.pass3.doc
# == Pass 3: _am.per_LTLIBRARY ==
# Inputs:
#   - Directory variable : `am.out_LTLIBRARIES`
# Erased inputs:
#   - Directory variable : `$(library)_SOURCES`
#   - Directory variable : `nodist_$(library)_SOURCES`
#   - Directory variable : `$(library)_CFLAGS`
#   - Directory variable : `$(library)_CPPFLAGS`
#   - Directory variable : `$(library)_LDFLAGS`
#   - Directory variable : `$(library)_LIBADD`
# Outputs:
#   - Directory variable : `am.CPPFLAGS`
#   - Directory variable : `am.CFLAGS`
#   - Target dependencies: `$(outdir)/$(library)`
#   - Target variable    : `$(outdir)/$(library): am.LDFLAGS`
#   - Directory variable : `am.subdirs`
#
# TODO: I'm not in love with how it figures out `am.subdirs`.
# TODO: I'm not in love with how it does the `install` dependencies.
endef
_am.pass3 = $(eval $(foreach f,$(am.out_LTLIBRARIES),$(call _am.per_LTLIBRARY,$f,$(call _am.file2var,$f))$(at.nl)))
_am.var_LTLIBRARIES = $1_SOURCES nodist_$1_SOURCES $1_CFLAGS $1_CPPFLAGS $1_LDFLAGS $1_LIBADD
# $1 = filename
# $2 = varname
define _am.per_LTLIBRARY
$(foreach var,_am.depends $(call _am.var_LTLIBRARIES,$2),$(var) ?=$(at.nl))
_am.depends += $$(call at.path,$$(call _am.file2.lo,$1) $$(call _am.file2lib,$1,LIBADD))
am.CPPFLAGS +=                 $$($2_CPPFLAGS)          $$(call _am.file2cpp,$1,LIBADD)
am.CFLAGS   +=                 $$($2_CFLAGS)
$$(outdir)/$1: private am.LDFLAGS := $$($2_LDFLAGS)
$$(outdir)/$1: $$(_am.depends)
$$(outdir)/install: $$(addsuffix install,$$(dir $$(filter %.la,$$(_am.depends))))
am.subdirs := $$(sort $$(am.subdirs)\
                      $$(filter-out $$(abspath $$(srcdir)),\
                                    $$(abspath $$(dir $$(filter-out -l% /%,$$(_am.depends))))))
am.CPPFLAGS := $$(am.CPPFLAGS)
am.CFLAGS := $$(am.CFLAGS)
$(foreach var,_am.depends $(call _am.var_LTLIBRARIES,$2),undefine $(var)$(at.nl))
endef

define _am.pass4.doc
# == Pass 4: Install rules / _am.per_directory ==
# Inputs:
#   - Directory variable : `am.sys_$(primary)`
# Outputs:
#   - Target : `$(DESTDIR)/$($(dirname)dir)/%`
#
# Creates simple `install` rules.  You will need to define your own rules if
# `am.sys2out_$(primary)` changed the notdir part of the filename.
endef

# Utility functions
_am.sys2dirs = $(sort $(patsubst %/,%,$(dir $(foreach p,$(_am.primaries),$(am.sys_$p)))))

_am.pass4 = $(eval $(foreach d,$(_am.sys2dirs)  ,$(call _am.per_directory,$d)$(at.nl)))
define _am.per_directory
$$(DESTDIR)$1/%: $$(outdir)/%
	@$$(NORMAL_INSTALL)
	$$(am.INSTALL)
$$(DESTDIR)$1/%: $$(srcdir)/%
	@$$(NORMAL_INSTALL)
	$$(am.INSTALL)
endef

mod.am.depends += files
define _am.pass5.doc
# == Pass 5: export ==
# Inputs:
#   - Directory variable : `am.subdirs`
#   - Directory variable : `am.sys_$(primary)`
#   - Directory variable : `am.out_$(primary)`
#   - Directory variable : `am.check_$(primary)`
# Outputs:
#   - Directory variable : `at.subdirs`
#   - Directory variable : `files.sys.all`
#   - Directory variable : `files.out.all`
#   - Directory variable : `files.out.check`
endef
define _am.pass5
at.subdirs += $(am.subdirs)
files.sys.all += $(foreach p,$(_am.primaries),$(am.sys_$p))
files.out.all += $(foreach p,$(_am.primaries),$(am.out_$p))
files.out.check += $(foreach p,$(_am.primaries),$(am.check_$p))
endef
