# Copyright (C) 2016  Luke Shumaker
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

mod.mod.description = Print information about Autothing modules

_mod.target = at-mod-info
_mod.modules := $(sort $(patsubst %.mk,%,$(filter %.mk,$(subst -, ,$(notdir $(wildcard $(topsrcdir)/build-aux/Makefile.*/??-*.mk))))))
_mod.quote = '$(subst ','\'',$1)'

$(eval $(foreach _mod.tmp,$(_mod.modules),mod.$(_mod.tmp).description ?=$(at.nl)mod.$(_mod.tmp).depends ?=$(at.nl)))

_mod.vars = $(filter $(addsuffix .%,$(_mod.modules)),$(.VARIABLES))
_mod.once := $(_mod.vars)
_mod.each :=
