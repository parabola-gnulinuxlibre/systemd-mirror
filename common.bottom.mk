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
clean_files += $(obj_files)
conf_files += Makefile $(topobjdir)/config.mk
# Now namespace the *_files variables
$(module)_src_files   := $(addprefix $(srcdir)/,$(src_files))
$(module)_obj_files   := $(addprefix $(objdir)/,$(obj_files))
$(module)_sys_files   := $(addprefix $(DESTDIR)/,$(sys_files))
$(module)_clean_files := $(addprefix $(objdir)/,$(clean_files))
$(module)_slow_files  := $(addprefix $(objdir)/,$(slow_files))
$(module)_conf_files  := $(addprefix $(objdir)/,$(conf_files))
$(module)_dist_files  := $(addprefix $(srcdir)/,$(dist_files))

# And add them to the $(parent)_*_files variables (if applicable)
ifneq ($(parent),)
$(parent)_src_files   := $($(parent)_src_files) $($(module)_src_files)
$(parent)_obj_files   := $($(parent)_obj_files) $($(module)_obj_files)
$(parent)_sys_files   := $($(parent)_sys_files) $($(module)_sys_files)
$(parent)_clean_files := $($(parent)_clean_files) $($(module)_clean_files)
$(parent)_slow_files  := $($(parent)_slow_files) $($(module)_slow_files)
$(parent)_conf_files  := $($(parent)_conf_files) $($(module)_conf_files)
$(parent)_dist_files  := $($(parent)_dist_files) $($(module)_dist_files)
endif

modules := $(modules) $(module)


# Include Makefiles from other directories

define _nl


endef
define _include_makefile
ifeq ($(filter $(abspath $1),$(included_makefiles)),)
include $(if $(call _is_subdir,.,$1),$(call _relto,.,$1),$(topobjdir)/$(call _relto,$(topobjdir),$1))
endif
endef
$(eval \
  _COMMON_MK_NOONCE = n$(_nl)\
  $(foreach dir,$(subdirs),parent=$(module)$(_nl)$(call _include_makefile,$(objdir)/$(dir)/Makefile)$(_nl))\
  parent=dep$(_nl)\
  $(call _include_makefile,$(topobjdir)/$(dir)/Makefile)$(_nl)\
  _COMMON_MK_NOONCE = $(_COMMON_MK_NOONCE))


# This only gets evaluated once, after all of the other Makefiles are read
ifeq ($(_COMMON_MK_NOONCE),)
# Empty module-level variables
objdir = /bogus
srcdir = /bogus
subdirs =
depdirs =
src_files =
obj_files =
sys_files =
clean_files =
slow_files =
conf_files =
dist_files =

# Declare phony targets
.phony = build install uninstall mostlyclean clean distclean maintainer-clean check
define module_rules
.PHONY: $(addsuffix -%(module),$(.phony))
# Constructive phony targets
build-%(module): $(%(module)_obj_files)
install-%(module): $(%(module)_sys_files)
# Destructive phony targets
_%(module)_uninstall        = $(%(module)_sys_files))
_%(module)_mostlyclean      = $(filter-out $(%(module)_slow_files) $(%(module)_conf_files) $(%(module)_dist_files),$(%(module)_clean_files))
_%(module)_clean            = $(filter-out                         $(%(module)_conf_files) $(%(module)_dist_files),$(%(module)_clean_files))
_%(module)_distclean        = $(filter-out                                                 $(%(module)_dist_files),$(%(module)_clean_files))
_%(module)_maintainer-clean =                                                                                      $(%(module)_clean_files)
uninstall-%(module) mostlyclean-%(module) clean-%(module) distclean-%(module) maintainer-clean-%(module): %-%(module):
	$(RM) -- $(sort $(_%(module)_$*))
	$(RMDIRS) $(sort $(dir $(_%(module)_$*))) 2>/dev/null || $(TRUE)
endef
$(foreach module,$(modules),$(eval $(subst %(module),$(module),$(value module_rules))))

# Alias each bare phony target to itself with the `-all` suffix
$(foreach t,$(.phony),$(eval $t: $t-all))

# Add the `dist` target
.PHONY: dist
dist: $(topobjdir)/$(PACKAGE)-$(VERSION).tar.gz
$(topobjdir)/$(PACKAGE)-$(VERSION).tar.gz: $(topobjdir)/$(PACKAGE)-$(VERSION)
	$(TAR) czf $@ -C $(<D) $(<F)
_copyfile = $(MKDIRS) $(dir $2) && $(CP) $1 $2
_addfile = $(call _copyfile,$3,$2/$(call _relto,$1,$3))
$(topobjdir)/$(PACKAGE)-$(VERSION): $(all_src_files) $(dep_src_files) $(all_dist_files) $(dep_dist_files)
	$(RM) -r $@
	$(MKDIR) $(@D)/tmp.$(@F).$$$$ && \
	$(foreach f,$^,$(call _addfile,$(topsrcdir),$(@D)/tmp.$(@F).$$$$,$f) &&) \
	$(MV) $(@D)/tmp.$(@F).$$$$ $@ || $(RM) -r $(@D)/tmp.$(@F).$$$$

include $(topsrcdir)/common.once.mk

endif
