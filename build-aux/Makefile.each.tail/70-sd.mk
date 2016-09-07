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

mod.sd.depends += files write-atomic

_sd.man_xml = $(foreach _sd.tmp,$(filter %.xml,$(files.src.src)),$(if $(findstring /,$(_sd.tmp)),,$(_sd.tmp)))

ifneq ($(_sd.man_xml),)
#$(info $(outdir)/_sd.man_xml: «$(_sd.man_xml)»)

$(srcdir)/Makefile-man.mk: $(topsrcdir)/tools/make-man-rules.py $(topsrcdir)/tools/xml_helper.py $(topsrcdir)/man/custom-entities.ent.in $(outdir)/.var._sd.man_xml $(call at.addprefix,$(srcdir),$(_sd.man_xml))
	$(AM_V_GEN)$(PYTHON) $< $(filter %.xml,$^) | $(WRITE_ATOMIC) $@
files.src.gen += Makefile-man.mk

sd.MANPAGES =
sd.MANPAGES_ALIAS =
-include $(srcdir)/Makefile-man.mk

_sd.XML_FILES = \
	${patsubst %.1,%.xml,${patsubst %.3,%.xml,${patsubst %.5,%.xml,${patsubst %.7,%.xml,${patsubst %.8,%.xml,$(sd.MANPAGES)}}}}}
sd.HTML_FILES = \
	${_sd.XML_FILES:.xml=.html}
sd.HTML_ALIAS = \
	${patsubst %.1,%.html,${patsubst %.3,%.html,${patsubst %.5,%.html,${patsubst %.7,%.html,${patsubst %.8,%.html,$(sd.MANPAGES_ALIAS)}}}}}

ifneq ($(ENABLE_MANPAGES),)
man_MANS = \
	$(sd.MANPAGES) \
	$(sd.MANPAGES_ALIAS)

noinst_DATA += \
	$(sd.HTML_FILES) \
	$(sd.HTML_ALIAS)
endif # ENABLE_MANPAGES

at.subdirs += $(abspath $(topoutdir)/man)

endif # _sd.man_xml
