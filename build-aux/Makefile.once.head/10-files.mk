# Copyright (C) 2015-2017  Luke Shumaker
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

mod.files.description = Keeping track of groups of files
mod.files.depends += nested
define mod.files.doc
# User variables:
#   - `DESTDIR ?=`
#   - `RM      ?= rm -f`
#   - `RMDIR_P ?= rmdir -p --ignore-fail-on-non-empty`
#   - `TRUE    ?= true`
# Inputs:
#   - Global variable    : `files.groups   ?= all`
#   - Global variable    : `files.default  ?= all`
#   - Global variable    : `files.vcsclean ?= files.vcsclean`
#   - Global variable    : `files.generate ?= files.generate`
#   - Directory variable : `files.src.src`  # Sources
#   - Directory variable : `files.src.int`  # Intermediates; deletable
#   - Directory variable : `files.src.cfg`  # Outputs needed to run ./configure
#   - Directory variable : `files.src.gen`  # Other outputs
#   - Directory variable : `files.out.slow` # Things to exclude from `make mostlyclean`
#   - Directory variable : `files.out.int`  # Intermediates
#   - Directory variable : `files.out.cfg`  # Outputs created by ./configure
#   - Directory variable : `files.out.$(group)` for `group` in `$(files.groups)`
#   - Directory variable : `files.sys.$(group)` for `group` in `$(files.groups)`
# Outputs:
#   - Global variable    : `nested.targets`
#   - Global variable    : `at.targets`
#   - Global variable    : `.DEFAULT_GOAL = $(files.default)`
#   - Directory variable : `files.src`
#   - Directory variable : `files.out`
#   - Directory variable : `files.sys`
#   - Creative .PHONY targets:
#     - `$(outdir)/$(files.generate)`
#     - `$(outdir)/$(group)` for `group` in `$(files.groups)`
#     - `$(outdir)/install`
#     - `$(outdir)/install-$(group)` for `group` in `$(filter-out $(files.default),$(files.groups))`
#     - `$(outdir)/installdirs`
#   - Destructive .PHONY targets:
#     - `$(outdir)/uninstall`
#     - `$(outdir)/mostlyclean`
#     - `$(outdir)/clean`
#     - `$(outdir)/distclean`
#     - `$(outdir)/maintainer-clean`
#     - `$(outdir)/$(files.vcsclean)`
#
# Basic `*` wildcards are supported.  Use `*`, not `%`; it will automatically
# substitute `*`->`%` where appropriate.
#
# Each of the destructive targets depends on `$(target)-hook`, which
# is defined to be a "double-colon rule" allowing you to add your own
# code.
#
# TODO: prose documentation
endef
mod.files.doc := $(value mod.files.doc)

files.groups ?= all
files.default ?= all
files.vcsclean ?= files.vcsclean
files.generate ?= files.generate

.DEFAULT_GOAL = $(files.default)

# Standard creative PHONY targets
nested.targets += $(files.generate)
nested.targets += install installdirs
nested.targets += $(foreach g,$(files.groups),$g)
nested.targets += $(foreach g,$(filter-out $(files.default),$(files.groups)),install-$g install-$gdirs)
# Standard destructive PHONY targets
nested.targets += uninstall mostlyclean clean distclean maintainer-clean $(files.vcsclean)

# User configuration

DESTDIR ?=

RM      ?= rm -f
RMDIR_P ?= rmdir -p --ignore-fail-on-non-empty
TRUE    ?= true

# Utility functions

_files.XARGS = $(if $(strip $2),$1 $(strip $2))

_files.maintainer-clean-warning:
	@echo 'This command is intended for maintainers to use; it'
	@echo 'deletes files that may need special tools to rebuild.'
.PHONY: _files.maintainer-clean-warning
