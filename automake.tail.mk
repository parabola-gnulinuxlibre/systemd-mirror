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

include $(topsrcdir)/common.each.tail.mk


# Aggregate variables

# Add some more defaults to the *_files variables
$(_am)clean_files += $($(_am)gen_files) $($(_am)out_files)
$(_am)cfg_files += Makefile

# Now namespace the *_files variables
define _am_save_variables
_am_src_files/$(outdir)   = $(addprefix $(srcdir)/,$($(_am)src_files))
_am_gen_files/$(outdir)   = $(addprefix $(srcdir)/,$($(_am)gen_files))
_am_cfg_files/$(outdir)   = $(addprefix $(outdir)/,$($(_am)cfg_files))
_am_out_files/$(outdir)   = $(addprefix $(outdir)/,$($(_am)out_files))
_am_sys_files/$(outdir)   = $(addprefix $(DESTDIR),$($(_am)sys_files))
_am_clean_files/$(outdir) = $(addprefix $(outdir)/,$($(_am)clean_files))
_am_slow_files/$(outdir)  = $(addprefix $(outdir)/,$($(_am)slow_files))
_am_subdirs/$(outdir)     =                        $($(_am)subdirs)
endef
$(eval $(_am_save_variables))

# And add them to the $(parent)_*_files variables (if applicable)
define _am_add_subdir
_am_src_files/%(outdir)   += $(_am_src_files/%(subdir))
_am_gen_files/%(outdir)   += $(_am_gen_files/%(subdir))
_am_cfg_files/%(outdir)   += $(_am_cfg_files/%(subdir))
_am_out_files/%(outdir)   += $(_am_out_files/%(subdir))
_am_sys_files/%(outdir)   += $(_am_sys_files/%(subdir))
_am_clean_files/%(outdir) += $(_am_clean_files/%(subdir))
_am_slow_files/%(outdir)  += $(_am_slow_files/%(subdir))
endef
$(foreach subdir,$(call _am_path,$(addprefix $(outdir)/,$($(_am)subdirs))),$(eval $(subst %(outdir),$(outdir),$(subst %(subdir),$(subdir),$(value _am_add_subdir)))))

_am_outdirs := $(_am_outdirs) $(outdir)


# Do some per-directory magic

_am_phony = build install uninstall mostlyclean clean distclean maintainer-clean check

.PHONY: $(addprefix $(outdir)/,$(_am_phony))

$(addprefix $(outdir)/,uninstall mostlyclean clean distclean maintainer-clean)::
	$(RM) -- $(sort $(_am_$(@F)/$(@D)))
	$(RMDIRS) $(sort $(dir $(_am_$(@F)/$(@D)))) 2>/dev/null || $(TRUE)

# 'build' and 'install' must be defined later, because the
# am_*_files/* variables might not be complete yet.


# Include Makefiles from other directories

define _am_nl


endef

$(foreach _am_NO_ONCE,y,\
	$(foreach makefile,$(foreach dir,$($(_am)subdirs) $($(_am)depdirs),$(call _am_path,$(outdir)/$(dir)/Makefile)),\
		$(eval include $(filter-out $(_am_included_makefiles),$(makefile)))))


# This only gets evaluated once, after all of the other Makefiles are read
ifeq ($(_am_NO_ONCE),)
# Empty directory-level variables
outdir = /bogus
srcdir = /bogus

$(_am)subdirs =
$(_am)depdirs =

$(_am)src_files =
$(_am)gen_files =
$(_am)cfg_files =
$(_am)out_files =
$(_am)sys_files =
$(_am)clean_files =
$(_am)slow_files =

_am_clean_files/$(topoutdir) += $(topoutdir)/$(PACKAGE)-$(VERSION).tar.gz
$(addprefix $(topoutdir)/,mostlyclean clean distclean maintainer-clean) ::
	$(RM) -r -- $(topoutdir)/$(PACKAGE)-$(VERSION)

define _am_directory_rules
# Constructive phony targets
$(outdir)/build  : $(_am_out_files/%(outdir))
$(outdir)/install: $(_am_sys_files/%(outdir))
# Destructive phony targets
_am_uninstall/%(outdir)        = $(_am_sys_files/%(outdir))
_am_mostlyclean/%(outdir)      = $(filter-out $(_am_slow_files/%(outdir)) $(_am_cfg_files/%(outdir)) $(_am_gen_files/%(outdir)),$(_am_clean_files/%(outdir)))
_am_clean/%(outdir)            = $(filter-out                             $(_am_cfg_files/%(outdir)) $(_am_gen_files/%(outdir)),$(_am_clean_files/%(outdir)))
_am_distclean/%(outdir)        = $(filter-out                                                        $(_am_gen_files/%(outdir)),$(_am_clean_files/%(outdir)))
_am_maintainer-clean/%(outdir) =                                                                                                $(_am_clean_files/%(outdir))
endef
$(foreach outdir,$(_am_outdirs),$(eval $(subst %(outdir),$(outdir),$(value _am_directory_rules))))

# Add the `dist` target
.PHONY: dist
dist: $(topoutdir)/$(PACKAGE)-$(VERSION).tar.gz
$(topoutdir)/$(PACKAGE)-$(VERSION).tar.gz: $(topoutdir)/$(PACKAGE)-$(VERSION)
	$(TAR) czf $@ -C $(<D) $(<F)
_am_copyfile = $(MKDIRS) $(dir $2) && $(CP) -T $1 $2
_am_addfile = $(call _am_copyfile,$3,$2/$(call _am_relto,$1,$3))
$(topoutdir)/$(PACKAGE)-$(VERSION): $(_am_src_files/$(topoutdir)) $(_am_gen_files/$(topoutdir))
	$(RM) -r $@
	@PS4='' && set -x && \
	$(MKDIR) $(@D)/tmp.$(@F).$$$$ && \
	$(foreach f,$^,$(call _am_addfile,$(topsrcdir),$(@D)/tmp.$(@F).$$$$,$f) &&) \
	$(MV) $(@D)/tmp.$(@F).$$$$ $@ || $(RM) -r $(@D)/tmp.$(@F).$$$$

include $(topsrcdir)/common.once.tail.mk

# For some reason I can't explain, RM doesn't really get set with ?=
CP     ?= cp
MKDIR  ?= mkdir
MKDIRS ?= mkdir -p
MV     ?= mv
RM      = rm -f
RMDIRS ?= rmdir -p
TAR    ?= tar
TRUE   ?= true

.PHONY: noop
endif
