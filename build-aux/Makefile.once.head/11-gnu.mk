# Copyright (C) 2016  Luke Shumaker
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

# 7.2.3 Variables for Specifying Commands
# ---------------------------------------

INSTALL ?= install
INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA ?= ${INSTALL} -m 644

# These aren't specified in the standards, but we use them
STRIP ?= strip
MAKEINFO ?= makeinfo
TEXI2DVI ?= texi2dvi
TEXI2HTML ?= makeinfo --html
TEXI2PDF ?= texi2pdf
TEXI2PS ?= makeinfo --ps
MKDIR_P ?= mkdir -p

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

# Other initialization
gnu.info_docs ?=
std.dirlocal += gnu.info_docs

define _gnu.install_program
$$($1)/%: $$(outdir)/$$($1)
	$$(NORMAL_INSTALL)
	$$(INSTALL_PROGRAM)
$$($1)/%: $$(srcdir)/$$($1)
	$$(NORMAL_INSTALL)
	$$(INSTALL_PROGRAM)
endef

define _gnu.install_data
$$($1)/%: $$(outdir)/$$($1)
	$$(NORMAL_INSTALL)
	$$(INSTALL_DATA)
$$($1)/%: $$(srcdir)/$$($1)
	$$(NORMAL_INSTALL)
	$$(INSTALL_DATA)
endef

gnu.dirs += $(gnu.program_dirs) $(gnu.data_dirs)
