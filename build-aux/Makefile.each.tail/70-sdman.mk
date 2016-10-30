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
# Inputs:
#   - Global variable    : `ENABLE_MANPAGES`
#   - Directory variable : `files.src.src`
# Inputs (from `$(srcdir)/Makefile-man.mk`):
#   - Directory variable : `sdman.MANPAGES`
#   - Directory variable : `sdman.MANPAGES_ALIAS`
# Outputs:
#   - File               : `$(srcdir)/Makefile-man.mk`
#   - Directory variable : `at.subdirs`
#   - Directory variable : `files.src.gen`
#   - Directory variable : `man_MANS`
#   - Directory variable : `noinst_DATA`
#
# The `sdman.*` variables are the interface by which Makefile-man.mk may
# communicate up.  They should not be used outside of the `sdman` module.
endef
mod.sdman.doc := $(value mod.sdman.doc)

_sdman.man_xml = $(foreach _sdman.tmp,$(filter %.xml,$(files.src.src)),$(if $(findstring /,$(_sdman.tmp)),,$(_sdman.tmp)))

ifneq ($(_sdman.man_xml),)

$(srcdir)/Makefile-man.mk: $(topsrcdir)/tools/make-man-rules.py $(topsrcdir)/tools/xml_helper.py $(topsrcdir)/man/custom-entities.ent.in $(outdir)/.var._sdman.man_xml $(call at.addprefix,$(srcdir),$(_sdman.man_xml))
	$(AM_V_GEN)$(PYTHON) $< $(filter %.xml,$^) | $(WRITE_ATOMIC) $@
files.src.gen += Makefile-man.mk

sdman.MANPAGES =
sdman.MANPAGES_ALIAS =
#-include $(srcdir)/Makefile-man.mk

_sdman.XML_FILES = \
	${patsubst %.1,%.xml,${patsubst %.3,%.xml,${patsubst %.5,%.xml,${patsubst %.7,%.xml,${patsubst %.8,%.xml,$(sdman.MANPAGES)}}}}}
_sdman.HTML_FILES = \
	${_sdman.XML_FILES:.xml=.html}
_sdman.HTML_ALIAS = \
	${patsubst %.1,%.html,${patsubst %.3,%.html,${patsubst %.5,%.html,${patsubst %.7,%.html,${patsubst %.8,%.html,$(sdman.MANPAGES_ALIAS)}}}}}

ifneq ($(ENABLE_MANPAGES),)
man_MANS = \
	$(sdman.MANPAGES) \
	$(sdman.MANPAGES_ALIAS)

noinst_DATA += \
	$(_sdman.HTML_FILES) \
	$(_sdman.HTML_ALIAS)
endif # ENABLE_MANPAGES

at.subdirs += $(abspath $(topoutdir)/man)

endif # _sdman.man_xml
