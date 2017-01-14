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

# This file is based on §7.2 "Makefile Conventions" of the release of
# the GNU Coding Standards dated April 13, 2016.

gnuconf.pkgname ?= $(firstword $(PACKAGE_TARNAME) $(PACKAGE) $(PACKAGE_NAME))
ifeq ($(gnuconf.pkgname),)
$(error gnuconf.pkgname must be set)
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

# These must be user-configurable
AR ?= ar
ARFLAGS ?=
BISON ?= bison
BISONFLAGS ?=
CC ?= cc
CCFLAGS ?= $(CFLAGS)
FLEX ?= flex
FLEXFLAGS ?=
INSTALL ?= install
#INSTALLFLAGS ?=
LD ?= ld
LDFLAGS ?=
LDCONFIG ?= ldconfig #TODO
LDCONFIGFLAGS ?=
LEX ?= lex
LEXFLAGS ?= $(LFLAGS)
#MAKE
MAKEINFO ?= makeinfo
MAKEINFOFLAGS ?=
RANLIB ?= ranlib #TODO
RANLIBFLAGS ?=
TEXI2DVI ?= texi2dvi
TEXI2DVIFLAGS ?=
YACC ?= yacc
YACCFLAGS ?= $(YFLAGS)

CFLAGS ?=
LFLAGS ?=
YFLAGS ?=

LN_S ?= ln -s #TODO

CHGRP ?= chgrp
CHMOD ?= chmod
CHOWN ?= chown
MKNOD ?= mknod

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
