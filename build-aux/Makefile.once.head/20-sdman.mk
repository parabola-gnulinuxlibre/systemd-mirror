#  -*- Mode: makefile; indent-tabs-mode: t -*-
#
#  This file is part of systemd.
#
#  Copyright 2010-2012 Lennart Poettering
#  Copyright 2010-2012 Kay Sievers
#  Copyright 2013 Zbigniew Jędrzejewski-Szmek
#  Copyright 2013 David Strauss
#  Copyright 2016 Luke Shumaker
#
#  systemd is free software; you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation; either version 2.1 of the License, or
#  (at your option) any later version.
#
#  systemd is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with systemd; If not, see <http://www.gnu.org/licenses/>.

mod.sdman.description = (systemd) manpages
mod.sdman.depends += am files write-atomic
define mod.sdman.doc
# User variables:
#   - `V`
#   - `AM_V_LN*`
#   - `AM_V_XSLT*`
#   - `LN_S`
#   - `XSLTPROC`
# Inputs:
#   - Global variable    : `ENABLE_MANPAGES`
#   - Global variable    : `VERSION`
#   - Directory variable : `files.src.src`
# Outputs:
#   - File               : `$(srcdir)/Makefile-man.mk`
#   - Directory variable : `at.subdirs`
#   - Directory variable : `files.src.gen`
#   - Directory variable : `man_MANS`
#   - Directory variable : `noinst_DATA` (HTML)
#
# sdman -> Makefile-man.mk:
#   - Global variable    : `sdman.man-xslt`
#   - Global variable    : `sdman.man-alias`
#   - Global variable    : `sdman.html-xslt`
#   - Global variable    : `sdman.html-alias`
# Makefile-man.mk -> sdman:
#   - Directory variable : `sdman.MANPAGES`
#   - Directory variable : `sdman.MANPAGES_ALIAS`
#   - Targets            : ...
#
# The `sdman.*` variables are the interface by which the module
# communicates with the genrated Makefile-man.mk file.  They should not
# be used outside of the `sdman` module.
endef
mod.sdman.doc := $(value mod.sdman.doc)

V ?=
LN_S ?= ln -s

AM_V_LN ?= $(AM_V_LN_$(V))
AM_V_LN_ ?= $(AM_V_LN_$(AM_DEFAULT_VERBOSITY))
AM_V_LN_0 ?= @echo "  LN      " $@;
AM_V_LN_1 ?=

AM_V_SOALIAS ?= $(AM_V_SOALIAS_$(V))
AM_V_SOALIAS_ ?= $(AM_V_SOALIAS_$(AM_DEFAULT_VERBOSITY))
AM_V_SOALIAS_0 ?= @echo "  SOALIAS " $@;
AM_V_SOALIAS_1 ?=

AM_V_XSLT ?= $(AM_V_XSLT_$(V))
AM_V_XSLT_ ?= $(AM_V_XSLT_$(AM_DEFAULT_VERBOSITY))
AM_V_XSLT_0 ?= @echo "  XSLT    " $@;
AM_V_XSLT_1 ?=

_sdman.XSLTPROC_FLAGS = \
	--nonet \
	--xinclude \
	--stringparam man.output.quietly 1 \
	--stringparam funcsynopsis.style ansi \
	--stringparam man.authors.section.enabled 0 \
	--stringparam man.copyright.section.enabled 0 \
	--stringparam systemd.version $(VERSION) \
	--path '$(outdir):$(srcdir):$(topoutdir)/man:$(topsrcdir)/man'

_sdman.XSLT = $(if $(XSLTPROC), $(XSLTPROC), xsltproc)

sdman.man-xslt   = $(AM_V_XSLT)$(_sdman.XSLT) -o $@ $(_sdman.XSLTPROC_FLAGS) $(srcdir)/man/custom-man.xsl $<
sdman.man-alias  = $(AM_V_SOALIAS)$(PRINTF) '.so %s\n' $(<F) > $@
sdman.html-xslt  = $(AM_V_XSLT)$(_sdman.XSLT) -o $@ $(_sdman.XSLTPROC_FLAGS) $(srcdir)/man/custom-html.xsl $<
sdman.html-alias = $(AM_V_LN)$(LN_S) -f $(<F) $@
