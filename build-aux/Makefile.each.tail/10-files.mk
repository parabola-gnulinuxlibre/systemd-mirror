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
# Add some more defaults to the *_files variables

$(foreach _files.var,$(patsubst files.%,%,files.src $(filter files.src.%,$(.VARIABLES))),\
          $(eval _files.$(_files.var) = $$(call at.addprefix,$$(srcdir),$$(files.$(_files.var)))))
$(foreach _files.var,$(patsubst files.%,%,files.out $(filter files.out.%,$(.VARIABLES))),\
          $(eval _files.$(_files.var) = $$(call at.addprefix,$$(outdir),$$(files.$(_files.var)))))
$(foreach _files.var,$(patsubst files.%,%,files.sys $(filter files.sys.%,$(.VARIABLES))),\
          $(eval _files.$(_files.var) = $$(addprefix $$(DESTDIR),$$(files.$(_files.var)))))

_files.all = $(_files.src) $(_files.out) $(_files.sys)

at.targets += $(subst *,%,$(_files.all))

# Creative targets
$(outdir)/$(files.generate): $(_files.src.gen) $(_files.src.cfg)
$(outdir)/install: $(_files.sys.$(files.default))
$(outdir)/installdirs: $(sort $(dir $(_files.sys)))
$(foreach _files.g,$(files.groups),\
          $(eval $$(outdir)/$(_files.g): $$(_files.out.$(_files.g))))
$(foreach _files.g,$(filter-out $(files.default),$(files.groups)),\
          $(eval $$(outdir)/install-$(_files.g): $$(_files.sys.$(_files.g))))

# Destructive targets
_files.uninstall   = $(_files.sys)
_files.mostlyclean = $(filter-out $(_files.out.slow) $(_files.out.cfg),$(_files.out))
_files.clean       = $(filter-out                    $(_files.out.cfg),$(_files.out))
_files.distclean   =                                                   $(_files.out)
$(addprefix $(outdir)/,uninstall mostlyclean clean distclean): %: %-hook
	$(RM)    -- $(sort $(filter-out %/,$(_files.$(@F))))
	$(RM) -r -- $(sort $(filter     %/,$(_files.$(@F))))
	$(RMDIR_P) -- $(sort $(dir $(_files.$(@F))))
_files.maintainer-clean   = $(filter-out $(_files.src.cfg) $(_files.src.src),$(_files.src))
_files.$(files.vcsclean)  = $(filter-out                   $(_files.src.src),$(_files.src))
$(addprefix $(outdir)/,maintainer-clean $(files.vcsclean)): $(outdir)/%: $(outdir)/distclean $(outdir)/%-hook
	@echo 'This command is intended for maintainers to use; it'
	@echo 'deletes files that may need special tools to rebuild.'
	$(RM)    -- $(sort $(filter-out %/,$(_files.$(@F))))
	$(RM) -r -- $(sort $(filter     %/,$(_files.$(@F))))
	$(RMDIR_P) -- $(sort $(dir $(_files.$(@F))))
$(foreach t,uninstall mostlyclean clean distclean maintainer-clean $(files.vcsclean), $(outdir)/$t-hook)::
.PHONY: $(foreach t,uninstall mostlyclean clean distclean maintainer-clean $(files.vcsclean), $(outdir)/$t-hook)
