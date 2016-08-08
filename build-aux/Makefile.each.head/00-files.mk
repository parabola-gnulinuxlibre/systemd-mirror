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
files.src = $(sort $(foreach _files.v,$(filter files.src.%,$(.VARIABLES)),$($(_files.v))))

files.out.slow ?=
files.out.int ?=
files.out.cfg ?=
$(foreach t,$(files.groups),$(eval files.out.$t ?=))
files.out = $(sort $(foreach _files.v,$(filter files.out.%,$(.VARIABLES)),$($(_files.v))))

$(foreach t,$(files.groups),$(eval files.sys.$t ?=))
files.sys = $(sort $(foreach _files.v,$(filter files.sys.%,$(.VARIABLES)),$($(_files.v))))
