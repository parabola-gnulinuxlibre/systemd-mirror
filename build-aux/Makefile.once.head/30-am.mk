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

am.distmode_PROGRAMS    = nodist
am.distmode_LTLIBRARIES = nodist
am.distmode_SCRIPTS     = dist
am.distmode_DATA        = nodist
am.distmode_HEADERS     = dist
am.distmode_MANS        = nodist

# Used by the PROGRAMS and LTLIBRARIES passes
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
_am.pass0 = $(value _am.pass0_)
define _am.pass0_
# == Pass 0: man_MANS ==
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
# == Pass 1: initialize variables ==
# Inputs: (used to detect a list of $(dirname)s)
#  - Directory variable :        `$(dirname)_$(primary)`
#  - Directory variable : `nodist_$(dirname)_$(primary)`
#  - Directory variable :   `dist_$(dirname)_$(primary)`
# Outputs:
#  - Directory variable : `noinst_$(primary) ?=`
#  - Directory variable : `check_$(primary)  ?=`
#  - Directory variable : `       $(dirname)_$(primary) ?=`
#  - Directory variable : `  dist_$(dirname)_$(primary) ?=`
#  - Directory variable : `nodist_$(dirname)_$(primary) ?=`
#  - Global variable    : `am.outpat_$(dirname)_$(primary) ?= %`
#  - Global variable    : `am.syspat_$(dirname)_$(primary) ?= %`
#
# Make sure that several variabes are initialized
endef

# Utility (reused in pass 2)
_am.primary2dirs = $(sort $(filter $(patsubst %dir,%,$(filter %dir,$(.VARIABLES))),\
                                   $(patsubst nodist_%,%,$(patsubst dist_%,%,$(patsubst %_$1,%,$(filter %_$1,$(.VARIABLES)))))))

define _am.pass1
# == Pass 1: initialize variables ==
$(foreach _am.primary,$(_am.primaries),
  $(foreach _am.dirname,$(call _am.primary2dirs,$(_am.primary)),
    am.outpat_$(_am.dirname)_$(_am.primary) ?= %
    am.syspat_$(_am.dirname)_$(_am.primary) ?= %
  )
)
endef

define _am.pass2.doc
# == Pass 2: Uniform Naming Scheme ==
# Inputs:
#   - Global variable    : `NORMAL_INSTALL`
#   - Global variable    : `MKDIR_P`
#   - Global variable    : `am.INSTALL_$(primary)`
#   - Global variable    : `am.outpat_$(dirname)_$(primary)`
#   - Global variable    : `am.syspat_$(dirname)_$(primary)`
#   - Global variable    : `am.distmode_$(primary)`
#   - Global variable    : `$(dirname)dir`
# Erased inputs:
#   - Directory variable : `noinst_$(primary)`
#   - Directory variable :  `check_$(primary)`
#
#   - Directory variable :        `$(dirname)_$(primary)`
#   - Directory variable :   `dist_$(dirname)_$(primary)`
#   - Directory variable : `nodist_$(dirname)_$(primary)`
# Outputs:
#   - Directory variable : `am.chk_$(primary)`
#   - Directory variable : `am.out_$(primary)`
#   - Directory variable : `am.sys_$(primary)`
#   - Targets            : `$(addprefix $(DESTDIR)$(dirname)/,$(notdir $({,dist_,nodist_}$(dirname)_$(primary))))`
#
# TODO
endef

# Default values
am.INSTALL_PROGRAMS    ?= $(INSTALL_PROGRAM) $< $@
am.INSTALL_SCRIPTS     ?= $(INSTALL) $< $@
am.INSTALL_LTLIBRARIES ?= $(INSTALL) $< $@
am.INSTALL_DATA        ?= $(INSTALL_DATA) $< $@
am.INSTALL_HEADERS     ?= $(INSTALL_DATA) $< $@
am.INSTALL_MANS        ?= $(INSTALL_DATA) $< $@

# Utility
_am.var_out  =        $(_am.dirname)_$(_am.primary)
_am.var_out += nodist_$(_am.dirname)_$(_am.primary)
#_am.var_out += noinst_$(_am.primary)
#_am.var_out +=  check_$(_am.primary)

_am.var_sys  =        $(_am.dirname)_$(_am.primary)
_am.var_sys += nodist_$(_am.dirname)_$(_am.primary)
_am.var_sys +=   dist_$(_am.dirname)_$(_am.primary)

define _am.pass2
# == Pass 2: Uniform Naming Scheme ==
$(foreach _am.primary,$(_am.primaries),
  ## primary: $(_am.primary)
  noinst_$(_am.primary) ?=
  check_$(_am.primary)  ?=

  am.chk_$(_am.primary) := $$(check_$(_am.primary))
  am.out_$(_am.primary) := $$(noinst_$(_am.primary))
  am.sys_$(_am.primary) :=

  $(foreach _am.dirname,$(call _am.primary2dirs,$(_am.primary)),
    ## dirname: $(_am.dirname) ($(_am.primary))
    $(_am.dirname)_$(_am.primary)           ?=
    dist_$(_am.dirname)_$(_am.primary)      ?=
    nodist_$(_am.dirname)_$(_am.primary)    ?=

    $(am.distmode_$(_am.primary))_$(_am.dirname)_$(_am.primary) += $$($(_am.dirname)_$(_am.primary))

    # nodist
    $$(addprefix $$(DESTDIR)$$($(_am.dirname)dir)/,$$(notdir $$(nodist_$(_am.dirname)_$(_am.primary)) )): \
    $$(DESTDIR)$$($(_am.dirname)dir)/$(am.syspat_$(_am.dirname)_$(_am.primary)): $(call at.addprefix,$(outdir),$(am.outpat_$(_am.dirname)_$(_am.primary)))
	@$$(NORMAL_INSTALL)
	@$$(MKDIR_P) $$(@D)
	$$(am.INSTALL_$(_am.primary))

    # dist
    $$(addprefix $$(DESTDIR)$$($(_am.dirname)dir)/,$$(notdir $$(dist_$(_am.dirname)_$(_am.primary)) )): \
    $$(DESTDIR)$$($(_am.dirname)dir)/$(am.syspat_$(_am.dirname)_$(_am.primary)): $(call at.addprefix,$(srcdir),$(am.outpat_$(_am.dirname)_$(_am.primary)))
	@$$(NORMAL_INSTALL)
	@$$(MKDIR_P) $$(@D)
	$$(am.INSTALL_$(_am.primary))

    am.out_$(_am.primary) := $$(am.out_$(_am.primary)) $$(patsubst $(am.syspat_$(_am.dirname)_$(_am.primary)),$(am.outpat_$(_am.dirname)_$(_am.primary)),$$(notdir $$(nodist_$(_am.dirname)_$(_am.primary)) ))
    am.sys_$(_am.primary) := $$(am.sys_$(_am.primary)) $$(addprefix $$($(_am.dirname)dir)/,$$(notdir                                                               $$(nodist_$(_am.dirname)_$(_am.primary)) $$(dist_$(_am.dirname)_$(_am.primary)) ))

    ## (end dirname)
  )

  ## (end primary)
)
endef

define _am.pass3.doc
# == Pass 3: PROGRAMS ==
# Inputs:
#   - Directory variable : `am.out_PROGRAMS`
# Erased inputs:
#   - Directory variable :        `$(program)_SOURCES`
#   - Directory variable : `nodist_$(program)_SOURCES`
#   - Directory variable :        `$(program)_CFLAGS`
#   - Directory variable :        `$(program)_CPPFLAGS`
#   - Directory variable :        `$(program)_LDFLAGS`
#   - Directory variable :        `$(program)_LDADD`
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

# Utility
_am.var_PROGRAMS  =        $(_am.var)_SOURCES
_am.var_PROGRAMS += nodist_$(_am.var)_SOURCES
_am.var_PROGRAMS +=        $(_am.var)_CFLAGS
_am.var_PROGRAMS +=        $(_am.var)_CPPFLAGS
_am.var_PROGRAMS +=        $(_am.var)_LDFLAGS
_am.var_PROGRAMS +=        $(_am.var)_LDADD

define _am.pass3
# == Pass 3: PROGRAMS ==
$(foreach _am.file,$(am.out_PROGRAMS),
  $(eval _am.var = $(call _am.file2var,$(_am.file)))
  ## PROGRAM: $(_am.file) ($(_am.var))
  $(foreach var,_am.depends $(_am.var_PROGRAMS),
    $(var) ?=
  )

  _am.depends += $$(call at.path,$$(call _am.file2.o,$(_am.file))  $$(call _am.file2lib,$(_am.file),LDADD))
  am.CPPFLAGS +=                 $$($(_am.var)_CPPFLAGS)           $$(call _am.file2cpp,$(_am.file),LDADD)
  am.CFLAGS   +=                 $$($(_am.var)_CFLAGS)
  $$(outdir)/$(_am.file): private am.LDFLAGS := $$($(_am.var)_LDFLAGS)
  $$(outdir)/$(_am.file): $$(_am.depends)
  $$(outdir)/install: $$(addsuffix install,$$(dir $$(filter %.la,$$(_am.depends))))
  am.subdirs := $$(sort $$(am.subdirs)\
                        $$(filter-out $$(abspath $$(srcdir)),\
                                      $$(abspath $$(dir $$(filter-out -l% /%,$$(_am.depends))))))

  am.CPPFLAGS := $$(am.CPPFLAGS)
  am.CFLAGS := $$(am.CFLAGS)

  undefine _am.depends
)
endef

define _am.pass4.doc
# == Pass 4: LTLIBRARIES ==
# Inputs:
#   - Directory variable : `am.out_LTLIBRARIES`
# Erased inputs:
#   - Directory variable :        `$(library)_SOURCES`
#   - Directory variable : `nodist_$(library)_SOURCES`
#   - Directory variable :        `$(library)_CFLAGS`
#   - Directory variable :        `$(library)_CPPFLAGS`
#   - Directory variable :        `$(library)_LDFLAGS`
#   - Directory variable :        `$(library)_LIBADD`
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

# Utility
_am.var_LTLIBRARIES  =        $(_am.var)_SOURCES
_am.var_LTLIBRARIES += nodist_$(_am.var)_SOURCES
_am.var_LTLIBRARIES +=        $(_am.var)_CFLAGS
_am.var_LTLIBRARIES +=        $(_am.var)_CPPFLAGS
_am.var_LTLIBRARIES +=        $(_am.var)_LDFLAGS
_am.var_LTLIBRARIES +=        $(_am.var)_LIBADD

define _am.pass4
# == Pass 4: LTLIBRARIES ==
$(foreach _am.file,$(am.out_LTLIBRARIES),
  $(eval _am.var = $(call _am.file2var,$(_am.file)))
  ## LTLIBRARY: $(_am.file) ($(_am.var))
  $(foreach var,_am.depends $(_am.var_LTLIBRARIES),
    $(var) ?=
  )

  _am.depends += $$(call at.path,$$(call _am.file2.lo,$(_am.file)) $$(call _am.file2lib,$(_am.file),LIBADD))
  am.CPPFLAGS +=                 $$($(_am.var)_CPPFLAGS)           $$(call _am.file2cpp,$(_am.file),LIBADD)
  am.CFLAGS   +=                 $$($(_am.var)_CFLAGS)
  $$(outdir)/$(_am.file): private am.LDFLAGS := $$($(_am.var)_LDFLAGS)
  $$(outdir)/$(_am.file): $$(_am.depends)
  $$(outdir)/install: $$(addsuffix install,$$(dir $$(filter %.la,$$(_am.depends))))
  am.subdirs := $$(sort $$(am.subdirs)\
                        $$(filter-out $$(abspath $$(srcdir)),\
                                      $$(abspath $$(dir $$(filter-out -l% /%,$$(_am.depends))))))

  am.CPPFLAGS := $$(am.CPPFLAGS)
  am.CFLAGS := $$(am.CFLAGS)

  undefine _am.depends
)
endef

mod.am.depends += files
define _am.pass5.doc
# == Pass 5: export ==
# Inputs:
#   - Directory variable : `am.subdirs`
#   - Directory variable : `am.sys_$(primary)`
#   - Directory variable : `am.out_$(primary)`
#   - Directory variable : `am.chk_$(primary)`
# Outputs:
#   - Directory variable : `at.subdirs`
#   - Directory variable : `files.sys.all`
#   - Directory variable : `files.out.all`
#   - Directory variable : `files.out.check`
endef
_am.pass5 = $(value _am.pass5_)
define _am.pass5_
# == Pass 4: export ==
at.subdirs += $(am.subdirs)
files.out.check += $(foreach p,$(_am.primaries),$(am.chk_$p))
files.out.all += $(foreach p,$(_am.primaries),$(am.out_$p))
files.sys.all += $(foreach p,$(_am.primaries),$(am.sys_$p))
endef
