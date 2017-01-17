# This Makefile is a minimal stub that exists to allow the
# `at-modules` set of Make targets to print documentation for the
# present Autothing modules.
#
# This file is part of the documentation for Autothing.
#
# Copyright (C) 2017  Luke Shumaker
#
# This documentation file is placed into the public domain.  If that
# is not possible in your legal system, I grant you permission to use
# it in absolutely every way that I can legally grant to you.

dist.pkgname = autothing
dist.version = 1.0
gnuconf.pkgname = autothing

topoutdir ?= .
topsrcdir ?= .
include $(topsrcdir)/build-aux/Makefile.head.mk
include $(topsrcdir)/build-aux/Makefile.tail.mk
