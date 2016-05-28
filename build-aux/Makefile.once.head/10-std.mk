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
.PHONY: all

# Standard creative PHONY targets
$(_am)phony += build install installdirs
# Standard destructive PHONY targets
$(_am)phony += uninstall mostlyclean clean distclean maintainer-clean

$(_am)dirlocal += $(_am)src_files
$(_am)dirlocal += $(_am)gen_files
$(_am)dirlocal += $(_am)cfg_files
$(_am)dirlocal += $(_am)out_files
$(_am)dirlocal += $(_am)sys_files
$(_am)dirlocal += $(_am)clean_files
$(_am)dirlocal += $(_am)slow_files
