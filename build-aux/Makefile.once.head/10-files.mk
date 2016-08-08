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

files.groups ?= all
files.default ?= all
files.vcsclean ?= files.vcsclean
files.generate ?= files.generate

.DEFAULT_GOAL = $(files.default)

# Standard creative PHONY targets
nested.targets += $(foreach g,$(files.groups), $g install-$g install-$gdirs)
# Standard destructive PHONY targets
nested.targets += uninstall mostlyclean clean distclean maintainer-clean

# User configuration

DESTDIR ?=

RM      ?= rm -f
RMDIR_P ?= rmdir -p --ignore-fail-on-non-empty
TRUE    ?= true
