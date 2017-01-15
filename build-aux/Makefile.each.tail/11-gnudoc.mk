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

$(outdir)/info :  $(addsuffix .info,$(gnudoc.docs))
files.src.gen  += $(addsuffix .info,$(gnudoc.docs))
files.out.dvi  += $(addsuffix .dvi ,$(gnudoc.docs))
files.out.html += $(addsuffix .html,$(gnudoc.docs))
files.out.pdf  += $(addsuffix .pdf ,$(gnudoc.docs))
files.out.ps   += $(addsuffix .ps  ,$(gnudoc.docs))

files.sys.all  += $(foreach f,$(gnudoc.docs), $(infodir)/$f.info )
files.sys.dvi  += $(foreach f,$(gnudoc.docs), $(dvidir)/$f.dvi   )
files.sys.html += $(foreach f,$(gnudoc.docs), $(htmldir)/$f.html )
files.sys.pdf  += $(foreach f,$(gnudoc.docs), $(pdfdir)/$f.pdf   )
files.sys.ps   += $(foreach f,$(gnudoc.docs), $(psdir)/$f.ps     )

$(outdir)/install:
	$(POST_INSTALL)
	$(foreach f,$(gnudoc.docs),$(INSTALL_INFO) $(DESTDIR)$(infodir)/$f.info $(DESTDIR)$(infodir)/dir$(at.nl))

$(outdir)/%.info: $(srcdir)/%.texi; $(MAKEINFO)  -o $(@D) $<
$(outdir)/%.info: $(outdir)/%.texi; $(MAKEINFO)  -o $(@D) $<
$(outdir)/%.dvi : $(srcdir)/%.texi; $(TEXI2DVI)  -o $(@D) $<
$(outdir)/%.dvi : $(outdir)/%.texi; $(TEXI2DVI)  -o $(@D) $<
$(outdir)/%.html: $(srcdir)/%.texi; $(TEXI2HTML) -o $(@D) $<
$(outdir)/%.html: $(outdir)/%.texi; $(TEXI2HTML) -o $(@D) $<
$(outdir)/%.pdf : $(srcdir)/%.texi; $(TEXI2PDF)  -o $(@D) $<
$(outdir)/%.pdf : $(outdir)/%.texi; $(TEXI2PDF)  -o $(@D) $<
$(outdir)/%.ps  : $(srcdir)/%.texi; $(TEXI2PS)   -o $(@D) $<
$(outdir)/%.ps  : $(outdir)/%.texi; $(TEXI2PS)   -o $(@D) $<
