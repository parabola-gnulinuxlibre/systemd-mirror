# Copyright (C) 2015-2017  Luke Shumaker
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

$(eval \
  $(foreach _files.var,$(filter files.src files.src.%,$(.VARIABLES)),\
    _$(_files.var) = $$(call at.addprefix,$$(srcdir),$$($(_files.var)))$(at.nl))\
  $(foreach _files.var,$(filter files.out files.out.%,$(.VARIABLES)),\
    _$(_files.var) = $$(call at.addprefix,$$(outdir),$$($(_files.var)))$(at.nl))\
  $(foreach _files.var,$(filter files.sys files.sys.%,$(.VARIABLES)),\
    _$(_files.var) = $$(addprefix $$(DESTDIR),$$($(_files.var)))$(at.nl)))

_files.all = $(_files.src) $(_files.out) $(_files.sys)

at.targets += $(subst *,%,$(_files.all))

# Creative targets
$(outdir)/$(files.generate): $(_files.src.gen) $(_files.src.cfg)
$(outdir)/install: $(_files.sys.$(files.default))
$(outdir)/installdirs: $(sort $(dir $(_files.sys)))
$(eval \
  $(foreach _files.g,$(files.groups),\
    $$(outdir)/$(_files.g): $$(_files.out.$(_files.g))$(at.nl))\
  $(foreach _files.g,$(filter-out $(files.default),$(files.groups)),\
    $$(outdir)/install-$(_files.g): $$(_files.sys.$(_files.g))$(at.nl)))

# Destructive targets
_files.uninstall   = $(_files.sys)
_files.mostlyclean = $(filter-out $(_files.out.slow) $(_files.out.cfg),$(_files.out))
_files.clean       = $(filter-out                    $(_files.out.cfg),$(_files.out))
_files.distclean   =                                                   $(_files.out)
_files.maintainer-clean  = $(_files.distclean) $(filter-out $(_files.src.cfg) $(_files.src.src),$(_files.src))
_files.$(files.vcsclean) = $(_files.distclean) $(filter-out                   $(_files.src.src),$(_files.src))
$(addprefix $(outdir)/,uninstall mostlyclean clean distclean maintainer-clean $(files.vcsclean)): %: %-hook
	$(call _files.XARGS,$(RM)    --,                  $(sort $(wildcard $(filter-out %/,$(_files.$(@F))))) )
	$(call _files.XARGS,$(RM) -r --,                  $(sort $(wildcard $(filter     %/,$(_files.$(@F))))) )
	$(call _files.XARGS,$(RMDIR_P) --,$(filter-out ./,$(sort $(wildcard $(dir           $(_files.$(@F)))))))
$(addprefix $(outdir)/,maintainer-clean $(files.vcsclean)): _files.maintainer-clean-warning
$(foreach t,uninstall mostlyclean clean distclean maintainer-clean $(files.vcsclean), $(outdir)/$t-hook)::
.PHONY: $(foreach t,uninstall mostlyclean clean distclean maintainer-clean $(files.vcsclean), $(outdir)/$t-hook)
