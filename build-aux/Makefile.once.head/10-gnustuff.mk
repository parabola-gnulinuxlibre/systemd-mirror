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

mod.gnustuff.description = Misc parts of the GNU Coding Standards
mod.gnustuff.depends += files nested
define mod.gnustuff.doc
# User variables:
#   - `STRIP     ?= strip`
#   - `MKDIR_P   ?= mkdir -p`
# Inputs:
#   - Global variable : `gnustuff.program_dirs ?= $(bindir) $(sbindir) $(libexecdir)`
#   - Global variable : `gnustuff.data_dirs    ?= <every variable from gnuconf ending in `dir`
#                                                  that isn't bindir, sbindir, or libexecdir> 
#   - Global variable : `gnustuff.dirs         ?= $(gnustuff.program_dirs) $(gnustuff.data_dirs)
# Outputs:
#   - Global variable : `nested.targets += install-strip`
#   - .PHONY target   : `$(outdir)/install-strip`
#
# gnustuff.info_docs:
#   The list of texinfo documents in the current directory, without
#   the `.texi` suffix.
#
# gnustuff.program_dirs:
#   Directories to use $(INSTALL_PROGRAM) for inserting files into.
#
# gnustuff.data_dirs:
#   Directories to use $(INSTALL_DATA) for inserting files into.
#
# gnustuff.dirs:
#   Directories to create
endef
mod.gnustuff.doc := $(value mod.gnustuff.doc)

STRIP     ?= strip
MKDIR_P   ?= mkdir -p

gnustuff.program_dirs ?= $(bindir) $(sbindir) $(libexecdir)
gnustuff.data_dirs ?= \
  $(datarootdir) $(datadir) $(sysconfdir) $(sharedstatedir) $(localstatedir) $(runstatedir) \
  $(includedir) $(oldincludedir) $(docdir) $(infodir) $(htmldir) $(dvidir) $(pdfdir) $(psdir) $(libdir) $(lispdir) $(localedir) \
  $(mandir) $(man1dir) $(man2dir) $(man3dir) $(man4dir) $(man5dir) $(man6dir) $(man7dir) $(man8dir)
gnustuff.dirs += $(gnu.program_dirs) $(gnu.data_dirs)

nested.targets += install-strip
