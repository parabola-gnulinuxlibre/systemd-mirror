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

files.src.src ?=
files.src.int ?=
files.src.cfg ?=
files.src.gen ?=

files.out.slow ?=
files.out.int ?=
files.out.cfg ?=

# define files.out.$(group) files.sys.$(group) for every files.group
$(eval $(foreach t,$(files.groups),files.out.$t ?=$(at.nl)files.sys.$t ?=$(at.nl)))

# define files.src, files.out, and files.sys aggregates
$(eval \
  files.src = $$(sort $(foreach _files.v,$(filter files.src.%,$(.VARIABLES)),$$($(_files.v))))$(at.nl)\
  files.out = $$(sort $(foreach _files.v,$(filter files.out.%,$(.VARIABLES)),$$($(_files.v))))$(at.nl)\
  files.sys = $$(sort $(foreach _files.v,$(filter files.sys.%,$(.VARIABLES)),$$($(_files.v)))))
