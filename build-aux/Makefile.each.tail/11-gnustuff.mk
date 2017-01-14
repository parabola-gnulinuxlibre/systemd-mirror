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

# 7.2.6: Standard Targets for Users
# ---------------------------------

std.gen_files += $(foreach f,$(gnustuff.info_docs),            $f.info )
std.sys_files += $(foreach f,$(gnustuff.info_docs), $(infodir)/$f.info )

$(foreach d,$(gnustuff.program_dirs),$(eval $(call _gnustuff.install_program,$d)))
$(foreach d,$(gnustuff.data_dirs)   ,$(eval $(call _gnustuff.install_data,$d)))

#all: std
install:
	$(foreach f,$(gnustuff.info_docs),$(INSTALL_INFO) $(DESTDIR)$(infodir)/$f.info $(DESTDIR)$(infodir)/dir$(at.nl))
$(outdir)/install-html: $(foreach f,$(gnustuff.info_docs), $(DESTDIR)$(htmldir)/$f.html )
$(outdir)/install-dvi : $(foreach f,$(gnustuff.info_docs), $(DESTDIR)$(dvidir)/$f.dvi  )
$(outdir)/install-pdf : $(foreach f,$(gnustuff.info_docs), $(DESTDIR)$(pdfdir)/$f.pdf  )
$(outdir)/install-ps  : $(foreach f,$(gnustuff.info_docs), $(DESTDIR)$(psdir)/$f.ps   )
#uninstall: std
$(outdir)/install-strip: install
	$(STRIP) $(filter $(addsuffix /%,$(gnustuff.program_dirs)),$(std.sys_files/$(@D)))
#clean: std
#distclean: std
#mostlyclean: std
#maintainer-clean: std
TAGS: TODO
$(outdir)/info: $(addsuffix .info,$(gnustuff.info_docs))
$(outdir)/dvi : $(addsuffix .dvi ,$(gnustuff.info_docs))
$(outdir)/html: $(addsuffix .html,$(gnustuff.info_docs))
$(outdir)/pdf : $(addsuffix .pdf ,$(gnustuff.info_docs))
$(outdir)/ps  : $(addsuffix .ps  ,$(gnustuff.info_docs))
#dist:dist
check: TODO
installcheck: TODO
#installdirs: std

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
#installdirs: std
