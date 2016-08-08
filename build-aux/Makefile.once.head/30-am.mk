mod.am.description = Support for Automake variables (systemd specific)
mod.am.depends += gnuconf

am.inst2noinst_DATA = $(notdir \
        $(patsubst $(sysusersdir)/%.conf,%.sysusers,\
        $(patsubst $(sysctldir)/%.conf,%.sysctl,\
        $1)))
am.var_PROGRAMS    = $1_SOURCES nodist_$1_SOURCES $1_CFLAGS $1_CPPFLAGS $1_LDFLAGS $1_LDADD
am.var_LTLIBRARIES = $1_SOURCES nodist_$1_SOURCES $1_CFLAGS $1_CPPFLAGS $1_LDFLAGS $1_LIBADD

am.INSTALL_PROGRAM   ?= $(INSTALL_PROGRAM) $< $@
am.INSTALL_SCRIPT    ?= $(INSTALL) $< $@
am.INSTALL_LTLIBRARY ?= $(INSTALL) $< $@
am.INSTALL_DATA      ?= $(INSTALL_DATA) $< $@

################################################################################

# this list of primaries is based on the Automake 1.15 manual
am.primaries ?= PROGRAMS LIBRARIES LTLIBRARIES LISP PYTHON JAVA SCRIPTS DATA HEADERS MANS TEXINFOS
$(foreach p,$(am.primaries),$(eval am.inst2noinst_$p ?= $$(notdir $$1)))

am.file2var = $(subst -,_,$(subst .,_,$1))
am.file2sources  = $(addprefix $(srcdir)/,$(notdir $($(am.file2var)_SOURCES)))
am.file2sources += $(addprefix $(outdir)/,$(notdir $(nodist_$(am.file2var)_SOURCES)))
am.file2.o  = $(patsubst $(srcdir)/%,$(outdir)/%,$(patsubst %.c,%.o ,$(filter %.c,$(am.file2sources))))
am.file2.lo = $(patsubst %.o,%.lo,$(am.file2.o))
am.file2lib = $(foreach l,   $($(am.file2var)_$2),$(if $(filter lib%.la,$l), $($(l:.la=).DEPENDS)  , $l ))
am.file2cpp = $(foreach l,$1 $($(am.file2var)_$2),$(if $(filter lib%.la,$l), $($(l:.la=).CPPFLAGS) ,    ))
