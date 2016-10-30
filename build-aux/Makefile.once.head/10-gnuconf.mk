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

mod.gnuconf.description = GNU standard configuration variables
define mod.gnuconf.doc
# Inputs:
#   - Global variable: `gnuconf.pkgname`
#      (Default: `$(firstword $(PACKAGE_TARNAME) $(PACKAGE) $(PACKAGE_NAME))`)
# Outputs:
#   (see below)
#
# This module defines default values (using `?=`) a huge list of
# variables specified in the GNU Coding Standards that installing-users
# expect to be able to set.
#
# This is based on §7.2 "Makefile Conventions" of the July 25, 2016
# release of the GNU Coding Standards.
endef
mod.gnuconf.doc := $(value mod.gnuconf.doc)

gnuconf.pkgname ?= $(firstword $(PACKAGE_TARNAME) $(PACKAGE) $(PACKAGE_NAME))
ifeq ($(gnuconf.pkgname),)
$(error Autothing module: gnuconf: gnuconf.pkgname must be set)
endif

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

# 7.2.2: Utilities in Makefiles/7.2.3: Variables for Specifying Commands
# ----------------------------------------------------------------------

# Standard user-configurable programs.
#
# The list of programs here is specified in §7.2.2, but the associated FLAGS
# variables are specified in §7.2.3.  I found it cleaner to list them together.
AR ?= ar
ARFLAGS ?=
BISON ?= bison
BISONFLAGS ?=
CC ?= cc
CFLAGS ?= # CFLAGS instead of CCFLAGS
FLEX ?= flex
FLEXFLAGS ?=
INSTALL ?= install
# There is no INSTALLFLAGS[0]
LD ?= ld
LDFLAGS ?=
LDCONFIG ?= ldconfig # TODO[1]
LDCONFIGFLAGS ?=
LEX ?= lex
LFLAGS ?= # LFLAGS instead of LEXFLAGS
#MAKE
MAKEINFO ?= makeinfo
MAKEINFOFLAGS ?=
RANLIB ?= ranlib # TODO[1]
RANLIBFLAGS ?=
TEXI2DVI ?= texi2dvi
TEXI2DVIFLAGS ?=
YACC ?= yacc
YFLAGS ?= # YFLAGS instead of YACCFLAGS

LN_S ?= ln -s # TODO[2]

CHGRP ?= chgrp
CHGRPFLAGS ?=
CHMOD ?= chmod
CHMODFLAGS ?=
CHOWN ?= chown
CHOWNFLAGS ?=
MKNOD ?= mknod
MKNODFLAGS ?=

# [0]: There is no INSTALLFLAGS because it would be inconsistent with how the
#      standards otherwise recommend using $(INSTALL); with INSTALL_PROGRAM and
#      INSTALL_DATA; which are specified in a way precluding the use of
#      INSTALLFLAGS.  To have the variable, but to ignore it in the common case
#      would be confusing.
#
# [1]: The RANLIB and LDCONFIG variables need some extra smarts; §7.2.2 says:
#
#       > When you use ranlib or ldconfig, you should make sure nothing bad
#       > happens if the system does not have the program in question. Arrange
#       > to ignore an error from that command, and print a message before the
#       > command to tell the user that failure of this command does not mean a
#       > problem. (The Autoconf ‘AC_PROG_RANLIB’ macro can help with this.)
#
# [2]: The LN_S variable isn't standard, but we have it here as an (incomplete)
#      stub to help support this bit of §7.2.2:
#
#       > If you use symbolic links, you should implement a fallback for
#       > systems that don’t have symbolic links.

# 7.2.3: Variables for Specifying Commands
# ----------------------------------------

INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA ?= ${INSTALL} -m 644

# 7.2.5: Variables for Installation Directories
# ---------------------------------------------

# Root for the installation
prefix ?= /usr/local
exec_prefix ?= $(prefix)
# Executable programs
bindir     ?= $(exec_prefix)/bin
sbindir    ?= $(exec_prefix)/sbin
libexecdir ?= $(exec_prefix)/libexec
# Data files
datarootdir    ?= $(prefix)/share
datadir        ?= $(datarootdir)
sysconfdir     ?= $(prefix)/etc
sharedstatedir ?= $(prefix)/com
localstatedir  ?= $(prefix)/var
runstatedir    ?= $(localstatedir)/run
# Specific types of files
includedir ?= $(prefix)/include
oldincludedir ?= /usr/include
docdir ?= $(datarootdir)/doc/$(gnuconf.pkgname)
infodir ?= $(datarootdir)/info
htmldir ?= $(docdir)
dvidir  ?= $(docdir)
pdfdir  ?= $(docdir)
psdir   ?= $(docdir)
libdir ?= $(exec_prefix)/lib
lispdir ?= $(datarootdir)/emacs/site-lisp
localedir ?= $(datarootdir)/locale

mandir ?= $(datarootdir)/man
man1dir ?= $(mandir)/man1
man2dir ?= $(mandir)/man2
man3dir ?= $(mandir)/man3
man4dir ?= $(mandir)/man4
man5dir ?= $(mandir)/man5
man6dir ?= $(mandir)/man6
man7dir ?= $(mandir)/man7
man8dir ?= $(mandir)/man8

manext ?= .1
man1ext ?= .1
man2ext ?= .2
man3ext ?= .3
man4ext ?= .4
man5ext ?= .5
man6ext ?= .6
man7ext ?= .7
man8ext ?= .8

# 7.2.7: Install Command Categories
# ---------------------------------

PRE_INSTALL ?=
POST_INSTALL ?=
NORMAL_INSTALL ?=

PRE_UNINSTALL ?=
POST_UNINSTALL ?=
NORMAL_UNINSTALL ?=
