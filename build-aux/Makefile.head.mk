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

_at.MAKEFILE_LIST ?=
_at.MAKEFILE_LIST := $(strip $(_at.MAKEFILE_LIST) $(abspath $(lastword $(filter-out %.mk,$(MAKEFILE_LIST)))))

# This bit only gets evaluated once, at the very beginning
ifeq ($(origin _at.NO_ONCE),undefined)

# Internal functions ###################################################

# These 4 functions are all $(call _at.func,parent,child)
_at.is_strict_subdir  = $(filter $(abspath $1)/%,$(abspath $2))
_at.is_subdir         = $(filter $(abspath $1)/%,$(abspath $2)/.)
_at.relbase = $(strip                                                   \
  $(if $(call _at.is_subdir,$1,$2),                                     \
       $(patsubst %/.,%,$(patsubst $(abspath $1)/%,%,$(abspath $2)/.)), \
       $(abspath $2)))
_at.relto = $(strip                                                     \
  $(if $(call _at.is_subdir,$1,$2),                                     \
       $(patsubst %/.,%,$(patsubst $(abspath $1)/%,%,$(abspath $2)/.)), \
       ../$(call _at.relto,$(dir $1),$2)))

# These 3 functions only take one operand; we define public multi-operand
# versions below.
_at.path = $(strip                                                            \
  $(if $(call _at.is_subdir,$(topoutdir),$1),                                 \
       $(patsubst %/.,%,$(topoutdir)/$(call _at.relto,.,$1)),                 \
       $(if $(call _at.is_subdir,$(topsrcdir),$1),                            \
            $(patsubst %/.,%,$(topsrcdir)/$(call _at.relto,$(topsrcdir),$1)), \
            $(abspath $1))))
_at.out2src = $(call _at.path,$(if $(call _at.is_subdir,$(topoutdir),$1),$(topsrcdir)/$(call _at.path,$1),$1))
_at.addprefix = $(call _at.path,$(if $(filter-out /%,$2),$1/$2,$2))

_at.rest = $(wordlist 2,$(words $1),$1)
_at.reverse = $(if $1,$(call _at.reverse,$(_at.rest))) $(firstword $1)

_at.target_variable           = $(_at.target_variable.$(flavor $2))
_at.target_variable.recursive = $1: private $2  = $(subst $(at.nl),$$(at.nl),$(value $2))
_at.target_variable.simple    = $1: private $2 := $$($2)

_at.quote-pattern = $(subst %,\%,$(subst \,\\,$1))

# Sanity checking ######################################################
ifeq ($(filter undefine,$(.FEATURES)),)
$(error Autothing: We need a version of Make that supports 'undefine')
endif
ifeq ($(topsrcdir),)
$(error Autothing: topsrcdir must be set (and non-empty) before including Makefile.head.mk)
endif
ifeq ($(topoutdir),)
$(error Autothing: topoutdir must be set (and non-empty) before including Makefile.head.mk)
endif
ifneq ($(call _at.is_strict_subdir,$(topoutdir),$(topsrcdir)),)
$(error Autothing: topsrcdir=$(topsrcdir) must not be a subdirectory of topoutdir=$(topoutdir))
endif

# External provisions ##################################################

# These 4 functions are all $(call _at.func,parent,child)
at.is_subdir = $(_at.is_subdir)
at.is_strict_subdir = $(_at.is_strict_subdir)
#at.relbase = $(if $2,$(shell realpath -sm --relative-base=$1 -- $2))
at.relbase = $(foreach _at.tmp,$2,$(call _at.relbase,$1,$(_at.tmp)))
#at.relto   = $(if $2,$(shell realpath -sm --relative-to=$1   -- $2))
at.relto   = $(foreach _at.tmp,$2,$(call _at.relto,$1,$(_at.tmp)))

at.path    = $(foreach _at.tmp,$1,$(call _at.path,$(_at.tmp)))
at.out2src = $(foreach _at.tmp,$1,$(call _at.out2src,$(_at.tmp)))
at.addprefix = $(foreach _at.tmp,$2,$(call _at.addprefix,$1,$(_at.tmp)))

define at.nl


endef

# External configuration ###############################################
at.Makefile ?= Makefile

# Include modules ######################################################
include $(sort $(wildcard $(topsrcdir)/build-aux/Makefile.once.head/*.mk))
_at.tmp_targets =
_at.tmp_subdirs =
_at.VARIABLES =
_at.VARIABLES := $(.VARIABLES)

endif # _at.NO_ONCE

# This bit gets evaluated for each Makefile

outdir := $(call _at.path,$(dir $(lastword $(_at.MAKEFILE_LIST))))
ifeq ($(call _at.is_subdir,$(topoutdir),$(outdir)),)
$(error Autothing: not a subdirectory of topoutdir=$(topoutdir): $(outdir))
endif

# Don't use at.out2src because we *know* that $(outdir) is inside $(topoutdir),
# and has already had $(_at.path) called on it.
srcdir := $(call _at.path,$(topsrcdir)/$(outdir))
ifeq ($(call _at.is_subdir,$(topsrcdir),$(srcdir)),)
$(error Autothing: not a subdirectory of topsrcdir=$(topsrcdir): $(srcdir))
endif

at.subdirs =
at.targets =

include $(sort $(wildcard $(topsrcdir)/build-aux/Makefile.each.head/*.mk))
