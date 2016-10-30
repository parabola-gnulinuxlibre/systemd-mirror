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
#   - Directory variable : `files.src.src`
#   - Directory variable : `files.src.int`
#   - Directory variable : `files.src.cfg`
#   - Directory variable : `files.src.gen`
#   - Directory variable : `files.out.slow`
#   - Directory variable : `files.out.int`
#   - Directory variable : `files.out.cfg`
#   - Directory variable : `files.out.$(files.groups)` (well, $(addprefix...))
#   - Directory variable : `files.sys.$(files.groups)` (well, $(addprefix...))
# Outputs:
#   - Global variable : `nested.targets`
#   - Global variable : `at.targets`
#   - Global variable : `.DEFAULT_GOAL = $(files.default)`
#   - Creative .PHONY targets:
#     - `$(outdir)/$(files.generate))`
#     - `$(addprefix $(outdir)/,$(files.groups))`
#     - `$(outdir)/installdirs`
#     - `$(outdir)/install`
#   - Destructive .PHONY targets:
#     - `$(outdir)/uninstall`
#     - `$(outdir)/mostlyclean`
#     - `$(outdir)/clean`
#     - `$(outdir)/distclean`
#     - `$(outdir)/maintainer-clean`
#     - `$(outdir)/$(files.vcsclean)`
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
nested.targets += uninstall mostlyclean clean distclean maintainer-clean

# User configuration

DESTDIR ?=

RM      ?= rm -f
RMDIR_P ?= rmdir -p --ignore-fail-on-non-empty
TRUE    ?= true
