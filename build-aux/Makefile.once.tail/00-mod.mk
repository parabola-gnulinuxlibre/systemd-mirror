# Copyright (C) 2016  Luke Shumaker
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

$(_mod.target):
	@printf 'Autothing modules used in this project:\n'
	@printf ' - %s\n' $(foreach _mod.tmp,$(_mod.modules),$(call _mod.quote,$(_mod.tmp)))
$(addprefix $(_mod.target)/,$(_mod.modules)): $(_mod.target)/%:
	@printf 'Module : %s ' $(call _mod.quote,$*)
	@$(if $(at.mod.$*.description),printf ' - %s' $(call _mod.quote,mod.$*.description))
	@echo
	@echo Depends on : $(mod.$*.depends)
	@echo User configuration:
	@egrep '^\s*[^:#]+\s*\?=' $(topsrcdir)/build-aux/Makefile.once.head/??-$*.mk 2>/dev/null | sed 's|^\s*||;s|?=|:' | grep -v -e ^'_' -e ^'$*' | column -t -s: |sed 's|^|    |'
	@echo Developer global inputs:
	@egrep '^\s*[^:#]+\s*\?=' $(topsrcdir)/build-aux/Makefile.once.head/??-$*.mk 2>/dev/null | sed 's|^\s*||;s|?=|:' | grep ^'$*\.' | column -t -s: |sed 's|^|    |'
	@echo Developer per-directory inputs:
	@egrep '^\s*[^:#]+\s*\?=' $(topsrcdir)/build-aux/Makefile.each.head/??-$*.mk 2>/dev/null | sed 's|^\s*||;s|?=|:' | grep ^'$*\.' | column -t -s: |sed 's|^|    |'
	@echo Developer outputs:
	@egrep '^\s*[^:#]+\s*(|:|::)=' $(topsrcdir)/build-aux/Makefile.once.head/??-$*.mk 2>/dev/null | sed 's|^\s*||;s|?=|:' | grep ^'$*\.' | column -t -s: |sed 's|^|    |'
.PHONY: $(addprefix mod/,$(_mod.modules))
