# This file is based on §7.2 "Makefile Conventions" of the release of
# the GNU Coding Standards dated April 13, 2016.

# 7.2.1: General Conventions for Makefiles
# ----------------------------------------

# The standards claim that every Makefile should contain
#
#    SHELL = /bin/sh`
#
# but note that this is unnescesary with GNU Make.

# 7.2.2: Utilities in Makefiles
# -----------------------------

# It's ok to hard-code these commands in rules, but who wants to
# memorize the list of what's ok?
AWK ?= awk
CAT ?= cat
CMP ?= cmp
CP ?= cp
DIFF ?= diff
ECHO ?= echo
EGREP ?= egrep
EXPR ?= expr
FALSE ?= false
GREP ?= grep
INSTALL_INFO ?= install-info
LN ?= ln
LS ?= ls
MKDIR ?= mkdir
MV ?= mv
PRINTF ?= printf
PWD ?= pwd
RM ?= rm
RMDIR ?= rmdir
SED ?= sed
SLEEP ?= sleep
SORT ?= sort
TAR ?= tar
TEST ?= test
TOUCH ?= touch
TR ?= tr
TRUE ?= true

# Beyond those, define them yourself.

# 7.2.3 Variables for Specifying Commands
# ---------------------------------------

INSTALL ?= install
INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA ?= ${INSTALL} -m 644

# 7.2.4 DESTDIR: Support for Staged Installs
# ------------------------------------------

# This is done for us by the std module.

# 7.2.5 Variables for Installation Directories
# --------------------------------------------

# Root for the installation
prefix ?= /usr/local
exec_prefix ?= $(prefix)
# Executable programs
bindir     ?= $(exec_prefix)/bin
sbindir    ?= $(exec_prefix)/sbin
libexecdir ?= $(exec_prefix)/libexec
gnu.program_dirs += $(bindir) $(sbindir) $(libexecdir)
# Data files
datarootdir    ?= $(prefix)/share
datadir        ?= $(datarootdir)
sysconfdir     ?= $(prefix)/etc
sharedstatedir ?= $(prefix)/com
localstatedir  ?= $(prefix)/var
runstatedir    ?= $(localstatedir)/run
gnu.data_dirs += $(datarootdir) $(datadir) $(sysconfdir) $(sharedstatedir) $(localstatedir) $(runstatedir)
# Specific types of files
includedir ?= $(prefix)/include
oldincludedir ?= /usr/include
docdir ?= $(datarootdir)/doc/$(PACKAGE)
infodir ?= $(datarootdir)/info
htmldir ?= $(docdir)
dvidir ?= $(docdir)
pdfdir ?= $(docdir)
psdir ?= $(docdir)
libdir ?= $(exec_prefix)/lib
lispdir ?= $(datarootdir)/emacs/site-lisp
localedir ?= $(datarootdir)/locale
gnu.data_dirs += $(includedir) $(oldincludedir) $(docdir) $(infodir) $(htmldir) $(dvidir) $(pdfdir) $(psdir) $(libdir) $(lispdir) $(localedir)

mandir ?= $(datarootdir)/man
man1dir ?= $(mandir)/man1
man2dir ?= $(mandir)/man2
man3dir ?= $(mandir)/man3
man4dir ?= $(mandir)/man4
man5dir ?= $(mandir)/man5
man6dir ?= $(mandir)/man6
man7dir ?= $(mandir)/man7
man8dir ?= $(mandir)/man8
gnu.data_dirs += $(mandir) $(man1dir) $(man2dir) $(man3dir) $(man4dir) $(man5dir) $(man6dir) $(man7dir) $(man8dir)

manext ?= .1
man1ext ?= .1
man2ext ?= .2
man3ext ?= .3
man4ext ?= .4
man5ext ?= .5
man6ext ?= .6
man7ext ?= .7
man8ext ?= .8

# srcdir is handled for us by the core

define _gnu.install_program
$$($1)/%: $$(outdir)/$$($1)
	$$(NORMAL_INSTALL)
	$$(INSTALL_PROGRAM)
$$($1)/%: $$(srcdir)/$$($1)
	$$(NORMAL_INSTALL)
	$$(INSTALL_PROGRAM)
endef
$(foreach d,$(gnu.program_dirs),$(eval $(call _gnu.install_program,$d)))

define _gnu.install_data
$$($1)/%: $$(outdir)/$$($1)
	$$(NORMAL_INSTALL)
	$$(INSTALL_DATA)
$$($1)/%: $$(srcdir)/$$($1)
	$$(NORMAL_INSTALL)
	$$(INSTALL_DATA)
endef
$(foreach d,$(gnu.data_dirs),$(eval $(call _gnu.install_data,$d)))

gnu.dirs += $(gnu.program_dirs) $(gnu.data_dirs)
$(gnu.dirs):
	$(MKDIR_P) $@

# 7.2.6: Standard Targets for Users
# ---------------------------------

gnu.info_docs ?=
std.sys_files += $(foreach f,$(gnu.info_docs), $(infodir)/$f.info )

#all: std
#install: std
$(outdir)/install-html: $(foreach f,$(gnu.info_docs), $(DESTDIR)$(htmldir)/$f.html )
$(outdir)/install-dvi : $(foreach f,$(gnu.info_docs), $(DESTDIR)$(dvidir)/$f.dvi  )
$(outdir)/install-pdf : $(foreach f,$(gnu.info_docs), $(DESTDIR)$(pdfdir)/$f.pdf  )
$(outdir)/install-ps  : $(foreach f,$(gnu.info_docs), $(DESTDIR)$(psdir)/$f.ps   )
#uninstall: std
$(outdir)/install-strip: install
	$(STRIP $(filter $(bindir)/% $(sbindir)/% $(libexecdir)/%,$(std.sys_files/$(@D)))
#clean: std
#distclean: std
#mostlyclean: std
#maintainer-clean: std
TAGS: TODO
$(outdir)/info: $(addsuffix .info,$(gnu.info_docs))
$(outdir)/dvi : $(addsuffix .dvi ,$(gnu.info_docs))
$(outdir)/html: $(addsuffix .html,$(gnu.info_docs))
$(outdir)/pdf : $(addsuffix .pdf ,$(gnu.info_docs))
$(outdir)/ps  : $(addsuffix .ps  ,$(gnu.info_docs))
#dist:dist
check: TODO
installcheck: TODO
#installdirs: std

$(outdir)/%.info: $(srcdir)/%.texi; $(MAKEINFO)  -o $(@D) $<
$(outdir)/%.dvi : $(srcdir)/%.texi; $(TEXI2DVI)  -o $(@D) $<
$(outdir)/%.html: $(srcdir)/%.texi; $(TEXI2HTML) -o $(@D) $<
$(outdir)/%.pdf : $(srcdir)/%.texi; $(TEXI2PDF)  -o $(@D) $<
$(outdir)/%.ps  : $(srcdir)/%.texi; $(TEXI2PS)   -o $(@D) $<



#installdirs: std

# 7.2.7: Standard Targets for Users
# ---------------------------------
