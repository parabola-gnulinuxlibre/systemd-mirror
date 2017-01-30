# Copyright (C) 2016-2017  Luke Shumaker
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

mod.texinfo.description = The GNU documentation system
mod.texinfo.depends += files nested gnuconf
define mod.texinfo.doc
# User variables (in addition to gnuconf):
#   - `TEXI2HTML ?= makeinfo --html`
#   - `TEXI2PDF  ?= texi2pdf`
#   - `TEXI2PS   ?= texi2dvi --ps`
# Inputs:
#   - Directory variable : `texinfo.docs ?=`
# Outputs:
#   - Global variable    : `files.groups += html dvi pdf ps`
#   - Global variable    : `nested.targets += info`
#   - Directory variable : `files.src.gen`
#   - Directory variable : `files.out.{dvi,html,pdf,ps}`
#   - Directory variable : `files.sys.{dvi,html,pdf,ps,all}`
#   - .PHONY target      : `$(outdir)/info`
#   - Target             : `$(outdir)/%.info`
#   - Target             : `$(outdir)/%.dvi`
#   - target             : `$(outdir)/%.html`
#   - target             : `$(outdir)/%.pdf`
#   - Target             : `$(outdir)/%.ps`
#
# TODO: prose documentation
endef
mod.texinfo.doc := $(value mod.texinfo.doc)

TEXI2HTML ?= makeinfo --html
TEXI2PDF  ?= texi2pdf
TEXI2PS   ?= texi2dvi --ps

files.groups += html dvi pdf ps
nested.targets += info
