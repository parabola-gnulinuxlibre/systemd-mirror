mod.amcfg.description = (systemd) Automake-to-Autothing configuration
mod.amcfg.depends += am
define mod.amcfg.doc
# User variables:
#   - `V`
#   - `AM_V_*`
#   - `pamconfdir`
#   - `tmpfilesdir`
#   - `sysusersdir`
#   - `sysctldir`
#   - `bashcompletiondir`
#   - `zshcompletiondir`
#   - `LIBTOOL`
#   - `INSTALL_PROGRAM`
#   - `INSTALL_SCRIPT`
#   - `INSTALL_DATA`
# Inputs:
#   - Global variable: `sd.ALL_LIBTOOLFLAGS`
# Outputs:
#   - Global variable: `am.sys2out_*`
#   - Global variable: `am.INSTALL_*`
endef
mod.amcfg.doc := $(value mod.amcfg.doc)

am.outpat_pamconf_DATA = %.pam

am.outpat_tmpfiles_DATA = %.tmpfiles
am.syspat_tmpfiles_DATA = %.conf

am.outpat_sysusers_DATA = %.sysusers
am.syspat_sysusers_DATA = %.conf

am.outpat_sysctl_DATA = %.sysctl
am.syspat_sysctl_DATA = %.conf

am.outpat_bashcompletion_DATA = %.completion.bash

am.outpat_zshcompletion_DATA = %.completion.zsh
am.syspat_zshcompletion_DATA = _%

am.outpat_xinitrc_SCRIPTS = %.xinitrc
am.syspat_xinitrc_SCRIPTS = %.sh

am.outpat_include_HEADERS = $(abspath $(srcdir)/include)/%.h
am.syspat_include_HEADERS = %.h

V ?=

AM_V_PROG ?= $(AM_V_PROG_$(V))
AM_V_PROG_ ?= $(AM_V_PROG_$(AM_DEFAULT_VERBOSITY))
AM_V_PROG_0 ?= @echo "  PROG    " $@;
AM_V_PROG_1 ?=

AM_V_SCRIPT ?= $(AM_V_SCRIPT_$(V))
AM_V_SCRIPT_ ?= $(AM_V_SCRIPT_$(AM_DEFAULT_VERBOSITY))
AM_V_SCRIPT_0 ?= @echo "  SCRIPT    " $@;
AM_V_SCRIPT_1 ?=

AM_V_LIB ?= $(AM_V_LIB_$(V))
AM_V_LIB_ ?= $(AM_V_LIB_$(AM_DEFAULT_VERBOSITY))
AM_V_LIB_0 ?= @echo "  LIB     " $@;
AM_V_LIB_1 ?=

AM_V_DATA ?= $(AM_V_DATA_$(V))
AM_V_DATA_ ?= $(AM_V_DATA_$(AM_DEFAULT_VERBOSITY))
AM_V_DATA_0 ?= @echo "  DATA    " $@;
AM_V_DATA_1 ?=

AM_V_HEADER ?= $(AM_V_HEADER_$(V))
AM_V_HEADER_ ?= $(AM_V_HEADER_$(AM_DEFAULT_VERBOSITY))
AM_V_HEADER_0 ?= @echo "  HEADER  " $@;
AM_V_HEADER_1 ?=

AM_V_MAN ?= $(AM_V_MAN_$(V))
AM_V_MAN_ ?= $(AM_V_MAN_$(AM_DEFAULT_VERBOSITY))
AM_V_MAN_0 ?= @echo "  MAN     " $@;
AM_V_MAN_1 ?=

am.INSTALL_PROGRAMS    = $(AM_V_PROG)$(LIBTOOL) $(AM_V_lt) --tag=CC $(sd.ALL_LIBTOOLFLAGS) --mode=install $(INSTALL_PROGRAM) $< $@
am.INSTALL_SCRIPTS     = $(AM_V_SCRIPT)$(INSTALL_SCRIPT) $< $@
am.INSTALL_LTLIBRARIES = $(AM_V_LIB)$(LIBTOOL) $(AM_V_lt) --tag=CC $(sd.ALL_LIBTOOLFLAGS) --mode=install $(INSTALL) $< $@
am.INSTALL_DATA        = $(AM_V_DATA)$(INSTALL_DATA) $< $@
am.INSTALL_HEADERS     = $(AM_V_HEADER)$(INSTALL_DATA) $< $@
am.INSTALL_MANS        = $(AM_V_MAN)$(INSTALL_DATA) $< $@
