ifeq ($(topsrcdir),)
topsrcdir := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
endif

PACKAGE = rvs
VERSION = 0.10
pkgtextdomain = $(PACKAGE)

DESTDIR        =
prefix         = /usr/local
exec_prefix    = $(prefix)

bindir         = $(exec_prefix)/bin
sbindir        = $(exec_prefix)/sbin
libexecdir     = $(exec_prefix)/libexec
datarootdir    = $(prefix)/share
datadir        = $(datarootdir)
sysconfdir     = $(prefix)/etc
sharedstatedir = $(prefix)/com
localstatedir  = $(prefix)/var
runstatedir    = $(localstatedir)/run
localedir      = $(datarootdir)/locale

pkgdatadir     = $(datadir)/$(PACKAGE)
pkglibexecdir  = $(libexecdir)/$(PACKAGE)

CFLAGS   = -std=c99 -Werror -Wall -Wextra -pedantic -O2
CPPFLAGS = -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE

CC              = cc
M4              = m4
MKDIR           = mkdir
MKDIRS          = mkdir -p
RMDIRS          = rmdir -p
INSTALL_DATA    = install -m644
INSTALL_PROGRAM = install -m755
CP              = cp
MV              = mv
RM              = rm -f
SED             = sed
SORT            = sort
TAR             = tar
TRUE            = true
PRINTF          = printf

AUTODEPS = t
