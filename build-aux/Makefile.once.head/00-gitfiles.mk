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
mod.gitfiles.files += $(topsrcdir)/$(gitfiles.file)
define mod.gitfiles.doc
# Inputs:
#   - Global variable    : `gitfiles.file` (Default: gitfiles.mk)
#   - Directory variable : `nested.subdirs`
#   - External           : git
# Outputs:
#   - File               : `$(topsrcdir)/$(gitfiles.file)`
#   - Directory variable : `files.src.src`
#   - Directory variable : `files.src.gen` (only in top dir)
#
# The `files` module has a variable (`files.src.src`) that you (the
# developer) set to list "pure" source files; the type of files that you
# would check into a version control system.  Since you are a
# responsible developer, you use a version control system.  Since the
# computer is already maintaining a list of these files *in the VCS*,
# why should you--a filthy human--need to also maintain the list?  Enter
# gitfiles, which will talk to git to maintain `files.src.src`, but
# won't require that the git repository be distributed to
# installing-users.
#
# If `$(topsrcdir)/.git` exists, then it will generate
# `$(topsrcdir)/$(gitfiles.file)`.  Otherwise, it will assume that
# `$(topsrcdir)/$(gitfiles.file)` already exists.
#
# It will use the information in `$(topsrcdir)/$(gitfiles.file)` to
# append to `files.src.src` in each directory
#
# Finally, since the generated `$(topsrcdir)/$(gitfiles.file)` must be
# distributed to users, it is added to $(topsrcdirs)'s `files.src.gen`.
#
# When setting `files.src.src`, it needs to know which files "belong" to
# the current directory directly, and which "belong" to a further
# subdirectory.  To do this, it uses an expression involving
# `$(nested.subdirs)`.
#
# While gitfiles sets `files.src.src` very early
# in `each.head`, because `nested.subdirs` might not be set yet, it may
# or may not be safe to use the value of `$(files.src.src)` in your
# Makefile, depending on how you set `nested.subdirs`.
endef
mod.gitfiles.doc := $(value mod.gitfiles.doc)

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
