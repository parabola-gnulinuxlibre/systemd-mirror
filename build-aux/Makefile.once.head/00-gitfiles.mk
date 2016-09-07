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

mod.gitfiles.description = Automatically populate files.src.src from git
mod.gitfiles.depends += files nested write-ifchanged quote

gitfiles.file ?= gitfiles.mk

_gitfiles.all =
-include $(topsrcdir)/$(gitfiles.file)

ifneq ($(wildcard $(topsrcdir)/.git),)
$(topsrcdir)/$(gitfiles.file): _gitfiles.FORCE
	@(cd $(@D) && git ls-files -z) | sed -z -e 's/\$$/\$$$$/g' -e 's/\n/$$(at.nl)/g' | xargs -r0 printf '_gitfiles.all+=%s\n' | $(WRITE_IFCHANGED) $@
.PHONY: _gitfiles.FORCE
endif

_gitfiles.dir = $(call at.relto,$(topsrcdir),$(srcdir))
_gitfiles.dir.all = $(patsubst $(_gitfiles.dir)/%,%,$(filter $(_gitfiles.dir)/%,$(_gitfiles.all)))
_gitfiles.dir.src = $(filter-out $(addsuffix /%,$(nested.subdirs)),$(_gitfiles.dir.all))
