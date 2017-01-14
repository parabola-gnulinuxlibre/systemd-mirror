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

# This bit only gets evaluated once, at the very beginning
ifeq ($(_at.NO_ONCE),)

ifeq ($(topsrcdir),)
$(error topsrcdir must be set before including Makefile.head.mk)
endif
ifeq ($(topoutdir),)
$(error topoutdir must be set before including Makefile.head.mk)
endif

_at.noslash = $(patsubst %/.,%,$(patsubst %/,%,$1))
# These are all $(call _at.func,parent,child)
#at.relto = $(if $2,$(shell realpath -sm --relative-to='$1' $2))
_at.is_subdir = $(filter $(abspath $1)/%,$(abspath $2)/.)
_at.relto_helper = $(if $(call _at.is_subdir,$1,$2),$(patsubst $1/%,%,$(addsuffix /.,$2)),$(addprefix ../,$(call _at.relto_helper,$(patsubst %/,%,$(dir $1)),$2)))
_at.relto = $(call _at.noslash,$(call _at.relto_helper,$(call _at.noslash,$(abspath $1)),$(call _at.noslash,$(abspath $2))))
at.relto = $(foreach p,$2,$(call _at.relto,$1,$p))
# Note that _at.is_subdir says that a directory is a subdirectory of
# itself.
at.path = $(call at.relto,.,$1)

define at.nl


endef

_at.rest = $(wordlist 2,$(words $1),$1)
_at.reverse = $(if $1,$(call _at.reverse,$(_at.rest))) $(firstword $1)

at.dirlocal += at.subdirs
at.dirlocal += at.depdirs

include $(sort $(wildcard $(topsrcdir)/build-aux/Makefile.once.head/*.mk))

endif # _at.NO_ONCE

# This bit gets evaluated for each Makefile

## Set outdir and srcdir (assumes that topoutdir and topsrcdir are
## already set)
outdir := $(call at.path,$(dir $(lastword $(filter-out %.mk,$(MAKEFILE_LIST)))))
srcdir := $(call at.path,$(topsrcdir)/$(call _at.relto,$(topoutdir),$(outdir)))

_at.included_makefiles := $(_at.included_makefiles) $(call at.path,$(outdir)/Makefile)

$(foreach v,$(at.dirlocal),$(eval $v=))

include $(sort $(wildcard $(topsrcdir)/build-aux/Makefile.each.head/*.mk))
