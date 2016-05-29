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

std.gen_files += $(foreach f,$(gnu.info_docs),            $f.info )
std.sys_files += $(foreach f,$(gnu.info_docs), $(infodir)/$f.info )

$(foreach d,$(gnu.program_dirs),$(eval $(call _gnu.install_program,$d)))
$(foreach d,$(gnu.data_dirs)   ,$(eval $(call _gnu.install_data,$d)))

#all: std
#install: std
$(outdir)/install-html: $(foreach f,$(gnu.info_docs), $(DESTDIR)$(htmldir)/$f.html )
$(outdir)/install-dvi : $(foreach f,$(gnu.info_docs), $(DESTDIR)$(dvidir)/$f.dvi  )
$(outdir)/install-pdf : $(foreach f,$(gnu.info_docs), $(DESTDIR)$(pdfdir)/$f.pdf  )
$(outdir)/install-ps  : $(foreach f,$(gnu.info_docs), $(DESTDIR)$(psdir)/$f.ps   )
#uninstall: std
$(outdir)/install-strip: install
	$(STRIP) $(filter $(addsuffix /%,$(gnu.program_dirs)),$(std.sys_files/$(@D)))
#clean: std
#distclean: std
#mostlyclean: std
#maintainer-clean: std
TAGS: TODO
$(outdir)/info: $(addsuffix .info,$(gnu.info_docs))
$(outdir)/dvi : $(addsuffix .dvi ,$(gnu.info_docs))
$(outdir)/html: $(addsuffix .html,$(gnu.info_docs))
$(outdir)/pdf : $(addsuffix .pdf ,$(gnu.info_docs))
$(outdir)/ps  : $(addsuffix .ps  ,$(gnu.info_docs))
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
