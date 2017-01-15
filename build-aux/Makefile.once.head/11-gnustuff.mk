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

STRIP ?= strip
TEXI2HTML ?= makeinfo --html
TEXI2PDF ?= texi2pdf
TEXI2PS ?= texi2dvi --ps
MKDIR_P ?= mkdir -p

gnustuff.program_dirs += $(bindir) $(sbindir) $(libexecdir)
gnustuff.data_dirs += $(datarootdir) $(datadir) $(sysconfdir) $(sharedstatedir) $(localstatedir) $(runstatedir)
gnustuff.data_dirs += $(includedir) $(oldincludedir) $(docdir) $(infodir) $(htmldir) $(dvidir) $(pdfdir) $(psdir) $(libdir) $(lispdir) $(localedir)
gnustuff.data_dirs += $(mandir) $(man1dir) $(man2dir) $(man3dir) $(man4dir) $(man5dir) $(man6dir) $(man7dir) $(man8dir)

gnustuff.info_docs ?=
std.dirlocal += gnustuff.info_docs

gnustuff.dirs += $(gnu.program_dirs) $(gnu.data_dirs)

at.phony += install-html install-dvi install-pdf install-ps
at.phony += info html dvi pdf ps
at.phony += install-strip
