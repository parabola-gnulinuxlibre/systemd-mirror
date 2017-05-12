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
#   - Target             : `$(outdir)/%.1`
#   - Target             : `$(outdir)/%.3`
#   - Target             : `$(outdir)/%.5`
#   - Target             : `$(outdir)/%.7`
#   - Target             : `$(outdir)/%.8`
#   - Target             : `$(outdir)/%.html`
#
# sdman -> Makefile-man.mk:
#   - Global variable    : `sdman.html-alias`
# Makefile-man.mk -> sdman:
#   - Directory variable : `sdman.MANPAGES`
#   - Directory variable : `sdman.MANPAGES_ALIAS`
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

AM_V_SOELIM ?= $(AM_V_SOELIM_$(V))
AM_V_SOELIM_ ?= $(AM_V_SOELIM_$(AM_DEFAULT_VERBOSITY))
AM_V_SOELIM_0 ?= @echo "  SOELIM  " $@;
AM_V_SOELIM_1 ?=

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
_sdman.XSLTPROC_PROCESS_MAN = \
	$(AM_V_XSLT)$(_sdman.XSLT) -o $@ $(_sdman.XSLTPROC_FLAGS) $(srcdir)/man/custom-man.xsl $<

_sdman.XSLTPROC_PROCESS_HTML = \
	$(AM_V_XSLT)$(_sdman.XSLT) -o $@ $(_sdman.XSLTPROC_FLAGS) $(srcdir)/man/custom-html.xsl $<

# Because the docbooc-xsl authors are assholes, they ignore everything but the
# directory of '-o' and instead choose filenames in it based on <refname>
# elements, with no option to override that from the command line.  This is a
# nice feature, until we have to rectify it with Make's poor support for
# commands with multiple outputs.  So, we'll let it do its thing, but have a
# rule for manually re-creating an alias without re-running xsltproc in case it
# gets removed.
sdman.man-alias = \
	$(AM_V_LN)$(PRINTF) '.so %s\n' $(<F) > $@
sdman.html-alias = \
	$(AM_V_LN)$(LN_S) -f $(<F) $@
