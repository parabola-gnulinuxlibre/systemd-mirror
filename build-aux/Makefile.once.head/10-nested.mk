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

mod.nested.description = Easy nested .PHONY targets
define mod.nested.doc
# Inputs:
#   - Global variable    : `nested.targets`
#   - Directory variable : `nested.subdirs`
# Outputs:
#   - .PHONY Targets     : `$(addprefix $(outdir)/,$(nested.targets))`
#   - Variable           : `at.subdirs`
#
# The Autothing `at.subdirs` slates a subdirectory's Makefile for inclusion,
# but doesn't help with recursive targets like `all`, `install`, or `clean`,
# which one would expect to descend into subdirectories.  Enter `nested`:
# Define a global list of targets that are recursive/nested, and then in each
# directory define a list of subdirectries that one would expect them to
# recurse into.
#
# Directories added to `nested.subdirs` are automatically added to `at.subdirs`
# during the each.tail phase.
#
# It may help to think of at.subdirs and nested.subdirs in terms of their
# Automake conterparts:
#
#      | Autothing      | GNU Automake |
#      +----------------+--------------+
#      | at.subdirs     | DIST_SUBDIRS |
#      | nested.subdirs | SUBDIRS      |
endef
mod.nested.doc := $(value mod.nested.doc)

nested.targets ?=
