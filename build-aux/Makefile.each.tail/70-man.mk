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

mod.man.description = (systemd) manpages
mod.man.depends += am files write-atomic

_man.man_xml = $(foreach _man.tmp,$(filter %.xml,$(files.src.src)),$(if $(findstring /,$(_man.tmp)),,$(_man.tmp)))

ifneq ($(_man.man_xml),)
#$(info $(outdir)/_man.man_xml: «$(_man.man_xml)»)

$(srcdir)/Makefile-man.mk: $(topsrcdir)/tools/make-man-rules.py $(topsrcdir)/tools/xml_helper.py $(topsrcdir)/man/custom-entities.ent.in $(outdir)/.var._man.man_xml $(call at.addprefix,$(srcdir),$(_man.man_xml))
	$(AM_V_GEN)$(PYTHON) $< $(filter %.xml,$^) | $(WRITE_ATOMIC) $@
files.src.gen += Makefile-man.mk

man.MANPAGES =
man.MANPAGES_ALIAS =
-include $(srcdir)/Makefile-man.mk

_man.XML_FILES = \
	${patsubst %.1,%.xml,${patsubst %.3,%.xml,${patsubst %.5,%.xml,${patsubst %.7,%.xml,${patsubst %.8,%.xml,$(man.MANPAGES)}}}}}
man.HTML_FILES = \
	${_man.XML_FILES:.xml=.html}
man.HTML_ALIAS = \
	${patsubst %.1,%.html,${patsubst %.3,%.html,${patsubst %.5,%.html,${patsubst %.7,%.html,${patsubst %.8,%.html,$(man.MANPAGES_ALIAS)}}}}}

ifneq ($(ENABLE_MANPAGES),)
man_MANS = \
	$(man.MANPAGES) \
	$(man.MANPAGES_ALIAS)

noinst_DATA += \
	$(man.HTML_FILES) \
	$(man.HTML_ALIAS)
endif # ENABLE_MANPAGES

at.subdirs += $(abspath $(topoutdir)/man)

endif # _man.man_xml
