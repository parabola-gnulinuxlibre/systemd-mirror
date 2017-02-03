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
#
# We do our on $(srcdir) / $(outdir) prefixing here because
# at.addprefix (while necessary for dependency lists) doesn't preserve
# trailing slashes, which we care about here; while also not caring
# about the path normalization that at.addprefix does.
_files.uninstall   = $(addprefix $(DESTDIR),$(files.sys))
_files.mostlyclean = $(addprefix $(srcdir)/,$(filter-out $(files.out.slow) $(files.out.cfg),$(files.out)))
_files.clean       = $(addprefix $(srcdir)/,$(filter-out                   $(files.out.cfg),$(files.out)))
_files.distclean   = $(addprefix $(srcdir)/,                                                $(files.out))
_files.maintainer-clean  = $(files.distclean) $(addprefix $(srcdir)/,$(filter-out $(files.src.cfg) $(files.src.src),$(files.src)))
_files.$(files.vcsclean) = $(files.distclean) $(addprefix $(srcdir)/,$(filter-out                  $(files.src.src),$(files.src)))
$(addprefix $(outdir)/,uninstall mostlyclean clean distclean maintainer-clean $(files.vcsclean)): %: %-hook
	$(call _files.XARGS,$(RM)    -- {},                                      $(sort $(filter-out %/,$(_files.$(@F)))) )
	$(call _files.XARGS,$(RM) -r -- {},                                      $(sort $(filter     %/,$(_files.$(@F)))) )
	$(call _files.XARGS,$(RMDIR_P) -- {} 2>/dev/null || true,$(filter-out ./,$(sort $(dir           $(_files.$(@F))))))
$(addprefix $(outdir)/,maintainer-clean $(files.vcsclean)): _files.maintainer-clean-warning
$(foreach t,uninstall mostlyclean clean distclean maintainer-clean $(files.vcsclean), $(outdir)/$t-hook)::
.PHONY: $(foreach t,uninstall mostlyclean clean distclean maintainer-clean $(files.vcsclean), $(outdir)/$t-hook)
