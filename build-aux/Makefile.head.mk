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
ifeq ($(_am_NO_ONCE),)

_am = am_

_am_noslash = $(patsubst %/.,%,$(patsubst %/,%,$1))
# These are all $(call _am_func,parent,child)
#_am_relto = $(if $2,$(shell realpath -sm --relative-to='$1' $2))
_am_is_subdir = $(filter $(abspath $1)/%,$(abspath $2)/.)
_am_relto_helper = $(if $(call _am_is_subdir,$1,$2),$(patsubst $1/%,%,$(addsuffix /.,$2)),$(addprefix ../,$(call _am_relto_helper,$(patsubst %/,%,$(dir $1)),$2)))
_am_relto = $(call _am_noslash,$(call _am_relto_helper,$(call _am_noslash,$(abspath $1)),$(call _am_noslash,$(abspath $2))))
# Note that _am_is_subdir says that a directory is a subdirectory of
# itself.
am_path = $(foreach p,$1,$(call _am_relto,.,$p))

$(_am)dirlocal += $(_am)subdirs
$(_am)dirlocal += $(_am)depdirs

include $(topsrcdir)/common.once.head.mk

endif # _am_NO_ONCE

# This bit gets evaluated for each Makefile

## Set outdir and srcdir (assumes that topoutdir and topsrcdir are
## already set)
outdir := $(call am_path,$(dir $(lastword $(filter-out %.mk,$(MAKEFILE_LIST)))))
srcdir := $(call am_path,$(topsrcdir)/$(call _am_relto,$(topoutdir),$(outdir)))

_am_included_makefiles := $(_am_included_makefiles) $(call am_path,$(outdir)/Makefile)

$(foreach v,$($(_am)dirlocal),$(eval $v=))

include $(topsrcdir)/common.each.head.mk
