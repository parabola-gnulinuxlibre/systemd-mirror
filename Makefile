dist.pkgname = autothing
dist.version = 1.0
gnuconf.pkgname = autothing

topoutdir ?= .
topsrcdir ?= .
include $(topsrcdir)/build-aux/Makefile.head.mk
include $(topsrcdir)/build-aux/Makefile.tail.mk
