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

# Declare the default target
all: build
.PHONY: all noop

# Standard creative PHONY targets
at.phony += build install installdirs
# Standard destructive PHONY targets
at.phony += uninstall mostlyclean clean distclean maintainer-clean

at.dirlocal += std.src_files
at.dirlocal += std.gen_files
at.dirlocal += std.cfg_files
at.dirlocal += std.out_files
at.dirlocal += std.sys_files
at.dirlocal += std.clean_files
at.dirlocal += std.slow_files

# User configuration

DESTDIR ?=

RM      ?= rm -f
RMDIR_P ?= rmdir -p
TRUE    ?= true
