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

# This bit gets evaluated for each Makefile processed

include $(call _at.reverse,$(sort $(wildcard $(topsrcdir)/build-aux/Makefile.each.tail/*.mk)))

_at.tmp_targets := $(at.targets)
_at.tmp_subdirs := $(call at.addprefix,$(outdir),$(at.subdirs))

# Clean the environment
$(eval \
  $(foreach _at.tmp_variable,$(filter-out $(call _at.quote-pattern,_at.tmp_variable $(_at.VARIABLES)),$(.VARIABLES)),\
    $(call _at.target_variable,$(_at.tmp_targets),$(_at.tmp_variable))$(at.nl)\
    undefine $(_at.tmp_variable)$(at.nl)))

# Recurse
$(foreach _at.NO_ONCE,y,\
          $(foreach _at.tmp,$(call at.path,$(addsuffix /$(at.Makefile),$(_at.tmp_subdirs))),\
                    $(if $(filter-out $(_at.MAKEFILE_LIST),$(abspath $(_at.tmp))),\
                         $(eval include $(_at.tmp)))))

# This bit only gets evaluated once, after all of the other Makefiles are read
ifeq ($(origin _at.NO_ONCE),undefined)

include $(call _at.reverse,$(sort $(wildcard $(topsrcdir)/build-aux/Makefile.once.tail/*.mk)))

endif # _at.NO_ONCE
