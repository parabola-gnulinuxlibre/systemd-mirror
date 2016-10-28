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

$(outdir)/at-variables $(outdir)/at-variables-local: _mod.VARIABLES := $(filter-out $(call quote.pattern,$(_at.VARIABLES)),$(.VARIABLES))
$(outdir)/at-variables-global:
	@printf '%s\n' $(call quote.shell-each,$(sort $(.VARIABLES)))
$(outdir)/at-variables-local:
	@printf '%s\n' $(call quote.shell-each,$(sort $(_mod.VARIABLES)))
$(outdir)/at-variables $(outdir)/at-values:
	@printf '%s\n' $(call quote.shell-each,$(sort $(.VARIABLES),$(_mod.VARIABLES)))
$(outdir)/at-variables/%:
	@printf '%s\n' $(call quote.shell,$($*))
$(outdir)/at-values/%:
	@printf '%s\n' $(call quote.shell,$(value $*))
.PHONY:       $(addprefix $(outdir)/, at-variables-global at-variables-local at-variables at-values)
at.targets += $(addprefix $(outdir)/, at-variables-global at-variables-local at-variables at-values at-variables/% at-values/%)

$(outdir)/at-modules:
	@printf 'Autothing modules used in this project:\n'
	@printf ' - %s\n' $(foreach _mod.tmp,$(_mod.modules),$(call quote.shell,$(_mod.tmp)	$(mod.$(_mod.tmp).description)	$(if $(value mod.$(_mod.tmp).doc),(more))))|column -t -s $$'\t'
$(addprefix $(outdir)/at-modules/,$(_mod.modules)): $(outdir)/at-modules/%:
	@printf 'Name          : %s\n' $(call quote.shell,$*)
	@printf 'Description   : %s\n' $(call quote.shell,$(mod.$*.description))
	@echo   'Depends on    :' $(sort $(mod.$*.depends))
	@echo   'Files         :'
	@printf '  %s\n' $(call quote.shell-each,$(call at.relto,$(topsrcdir),$(sort $(mod.$*.files) $(wildcard $(topsrcdir)/build-aux/Makefile.*/??-$*.mk))))
	@echo   'Documentation :'
	@printf '%s\n' $(call quote.shell,$(value mod.$*.doc)) | sed -e 's/^# /  /' -e 's/^#//'

$(outdir)/at-noop:
.PHONY:       $(outdir)/at-noop
at.targets += $(outdir)/at-noop
