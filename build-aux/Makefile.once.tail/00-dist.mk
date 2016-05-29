# Copyright (C) 2015-2016  Luke Shumaker
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

# Add the `dist` target
.PHONY: dist
dist: $(addprefix $(topoutdir)/$(PACKAGE)-$(VERSION),$($(_am)distexts)

_am_copyfile = $(MKDIR_P) $(dir $2) && $(CP) -T $1 $2
_am_addfile = $(call _am_copyfile,$3,$2/$(call _am_relto,$1,$3))
$(topoutdir)/$(PACKAGE)-$(VERSION): $(_am_src_files/$(topoutdir)) $(_am_gen_files/$(topoutdir))
	$(RM) -r $@
	@PS4='' && set -x && \
	$(MKDIR) $(@D)/tmp.$(@F).$$$$ && \
	$(foreach f,$^,$(call _am_addfile,$(topsrcdir),$(@D)/tmp.$(@F).$$$$,$f) &&) \
	$(MV) $(@D)/tmp.$(@F).$$$$ $@ || $(RM) -r $(@D)/tmp.$(@F).$$$$

$(topoutdir)/$(PACKAGE)-$(VERSION).tar: $(topoutdir)/$(PACKAGE)-$(VERSION)
	$(TAR) cf $@ -C $(<D) $(<F)
$(topoutdir)/$(PACKAGE)-$(VERSION).tar.gz: $(topoutdir)/$(PACKAGE)-$(VERSION).tar
	$(GZIP) $(GZIP_ENV) < $< > $@

CP      ?= cp
GZIP    ?= gzip
MKDIR   ?= mkdir
MKDIR_P ?= mkdir -p
MV      ?= mv
RM      ?= rm -f
RMDIR_P ?= rmdir -p
TAR     ?= tar
TRUE    ?= true

GZIP_ENV ?= --best
