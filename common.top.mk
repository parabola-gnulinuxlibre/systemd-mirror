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

# Both of these have the argument order "parent,child"
_noslash = $(patsubst %/,%,$1)
_relto = $(call _noslash,$(patsubst $(abspath $1)/%,%,$(abspath $2)/))
_is_subdir = $(filter $(abspath $1)/%,$(abspath $2)/)

## Declare the standard targets
all: build
.PHONY: all

## Set topobjdir, objdir, and srcdir (assumes that topsrcdir is already set)
ifeq ($(topobjdir),)
topobjdir := $(call _noslash,$(dir $(lastword $(filter %/config.mk config.mk,$(MAKEFILE_LIST)))))
endif
   objdir := $(call _noslash,$(dir $(lastword $(filter-out %.mk,$(MAKEFILE_LIST)))))
   srcdir := $(firstword $(call _relto,., $(topsrcdir)/$(call _relto,$(topobjdir),$(objdir)) ) .)

included_makefiles := $(included_makefiles) $(abspath $(objdir)/Makefile)

## Set module name
module := $(subst /,_,$(if $(call _is_subdir,.,$(objdir)),$(firstword $(call _relto,.,$(objdir)) all),dep-$(firstword $(call _relto,$(topobjdir),$(objdir)) top)))

## Empty variables for use by the module
subdirs =
depdirs =

src_files =
obj_files =
sys_files =

clean_files =

slow_files =
conf_files =
dist_files =
