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

_am = am_

_am_noslash = $(patsubst %/.,%,$(patsubst %/,%,$1))
# These are all $(call _am_func,parent,child)
#_am_relto = $(if $2,$(shell realpath -sm --relative-to='$1' $2))
_am_is_subdir = $(filter $(abspath $1)/%,$(abspath $2)/.)
_am_relto_helper = $(if $(call _am_is_subdir,$1,$2),$(patsubst $1/%,%,$(addsuffix /.,$2)),$(addprefix ../,$(call _am_relto_helper,$(patsubst %/,%,$(dir $1)),$2)))
_am_relto = $(call _am_noslash,$(call _am_relto_helper,$(call _am_noslash,$(abspath $1)),$(call _am_noslash,$(abspath $2))))
# Note that _am_is_subdir says that a directory is a subdirectory of
# itself.
_am_path = $(call _am_relto,.,$1)
am_path = $(foreach p,$1,$(call _am_relto,.,$p))

## Declare the default target
all: build
.PHONY: all

## Set outdir and srcdir (assumes that topoutdir and topsrcdir are
## already set)
outdir := $(call _am_path,$(dir $(lastword $(filter-out %.mk,$(MAKEFILE_LIST)))))
srcdir := $(call _am_path,$(topsrcdir)/$(call _am_relto,$(topoutdir),$(outdir)))

_am_included_makefiles := $(_am_included_makefiles) $(call _am_path,$(outdir)/Makefile)

## Empty variables for use by each Makefile
$(_am)subdirs =
$(_am)depdirs =

$(_am)src_files =
$(_am)gen_files =
$(_am)cfg_files =
$(_am)out_files =
$(_am)sys_files =

$(_am)clean_files =
$(_am)slow_files =

ifeq ($(_am_NO_ONCE),)
include $(topsrcdir)/common.once.head.mk
endif
include $(topsrcdir)/common.each.head.mk
