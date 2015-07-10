# Copyright (C) 2015  Luke Shumaker
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

include $(topsrcdir)/common.each.mk


# Aggregate variables

# Add some more defaults to the *_files variables
clean_files += $(out_files)
conf_files += Makefile $(topoutdir)/config.mk
# Now namespace the *_files variables
define _am_add_to_module
_am_$(module)_src_files   = $(addprefix $(srcdir)/,$(src_files))
_am_$(module)_out_files   = $(addprefix $(outdir)/,$(out_files))
_am_$(module)_sys_files   = $(addprefix $(DESTDIR),$(sys_files))
_am_$(module)_clean_files = $(addprefix $(outdir)/,$(clean_files))
_am_$(module)_slow_files  = $(addprefix $(outdir)/,$(slow_files))
_am_$(module)_conf_files  = $(addprefix $(outdir)/,$(conf_files))
_am_$(module)_dist_files  = $(addprefix $(srcdir)/,$(dist_files))
endef
$(eval $(_am_add_to_module))

# And add them to the $(parent)_*_files variables (if applicable)
define _am_add_to_parent
_am_%(parent)_src_files   += $(_am_%(module)_src_files)
_am_%(parent)_out_files   += $(_am_%(module)_out_files)
_am_%(parent)_sys_files   += $(_am_%(module)_sys_files)
_am_%(parent)_clean_files += $(_am_%(module)_clean_files)
_am_%(parent)_slow_files  += $(_am_%(module)_slow_files)
_am_%(parent)_conf_files  += $(_am_%(module)_conf_files)
_am_%(parent)_dist_files  += $(_am_%(module)_dist_files)
endef
ifneq ($(parent),)
$(eval $(subst %(parent),$(parent),$(subst %(module),$(module),$(value _am_add_to_parent))))
endif

modules := $(modules) $(module)


# Do some per-module magic

_am_phony = build install uninstall mostlyclean clean distclean maintainer-clean check

.PHONY: $(addsuffix -%(module),$(_am_phony))

$(addsuffix -$(module),uninstall mostlyclean clean distclean maintainer-clean) ::
	$(RM) -- $(sort $(_am_$@))
	$(RMDIRS) $(sort $(dir $(_am_$@))) 2>/dev/null || $(TRUE)


# Include Makefiles from other directories

define _am_nl


endef
define _am_include_makefile
ifeq ($(filter $(abspath $1),$(included_makefiles)),)
include $(if $(call _am_is_subdir,.,$1),$(call _am_relto,.,$1),$(topoutdir)/$(call _am_relto,$(topoutdir),$1))
endif
endef
$(eval \
  _am_NO_ONCE = y$(_am_nl)\
  $(foreach dir,$(subdirs),parent=$(module)$(_am_nl)$(call _am_include_makefile,$(outdir)/$(dir)/Makefile)$(_am_nl))\
  parent=dep$(_am_nl)\
  $(call _am_include_makefile,$(topoutdir)/$(dir)/Makefile)$(_am_nl)\
  _am_NO_ONCE = $(_am_NO_ONCE))


# This only gets evaluated once, after all of the other Makefiles are read
ifeq ($(_am_NO_ONCE),)
# Empty module-level variables
outdir = /bogus
srcdir = /bogus
subdirs =
depdirs =
src_files =
out_files =
sys_files =
clean_files =
slow_files =
conf_files =
dist_files =

ifeq ($(abspath .),$(abspath $(topoutdir)))
_am_all_clean_files += $(topoutdir)/$(PACKAGE)-$(VERSION).tar.gz
$(addsuffix -all,mostlyclean clean distclean maintainer-clean) ::
	$(RM) -r -- $(topoutdir)/$(PACKAGE)-$(VERSION)
endif

define _am_module_rules
# Constructive phony targets
build-%(module): $(_am_%(module)_out_files)
install-%(module): $(_am_%(module)_sys_files)
# Destructive phony targets
_am_uninstall-%(module)        = $(_am_%(module)_sys_files))
_am_mostlyclean-%(module)      = $(filter-out $(_am_%(module)_slow_files) $(_am_%(module)_conf_files) $(_am_%(module)_dist_files),$(_am_%(module)_clean_files))
_am_clean-%(module)            = $(filter-out                             $(_am_%(module)_conf_files) $(_am_%(module)_dist_files),$(_am_%(module)_clean_files))
_am_distclean-%(module)        = $(filter-out                                                         $(_am_%(module)_dist_files),$(_am_%(module)_clean_files))
_am_maintainer-clean-%(module) =                                                                                                  $(_am_%(module)_clean_files)
endef
$(foreach module,$(modules),$(eval $(subst %(module),$(module),$(value _am_module_rules))))

# Alias each bare phony target to itself with the `-all` suffix
$(foreach t,$(_am_phony),$(eval $t: $t-all))

# Add the `dist` target
.PHONY: dist
dist: $(topoutdir)/$(PACKAGE)-$(VERSION).tar.gz
$(topoutdir)/$(PACKAGE)-$(VERSION).tar.gz: $(topoutdir)/$(PACKAGE)-$(VERSION)
	$(TAR) czf $@ -C $(<D) $(<F)
_am_copyfile = $(MKDIRS) $(dir $2) && $(CP) $1 $2
_am_addfile = $(call _am_copyfile,$3,$2/$(call _am_relto,$1,$3))
$(topoutdir)/$(PACKAGE)-$(VERSION): $(_am_all_src_files) $(_am_dep_src_files) $(_am_all_dist_files) $(_am_dep_dist_files)
	$(RM) -r $@
	@PS4='' && set -x && \
	$(MKDIR) $(@D)/tmp.$(@F).$$$$ && \
	$(foreach f,$^,$(call _am_addfile,$(topsrcdir),$(@D)/tmp.$(@F).$$$$,$f) &&) \
	$(MV) $(@D)/tmp.$(@F).$$$$ $@ || $(RM) -r $(@D)/tmp.$(@F).$$$$

include $(topsrcdir)/common.once.mk

endif
