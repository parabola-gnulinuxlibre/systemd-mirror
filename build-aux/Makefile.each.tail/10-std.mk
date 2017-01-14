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
std.clean_files += $(std.gen_files) $(std.cfg_files) $(std.out_files)

# Fix each variable at its current value to avoid any weirdness
$(foreach c,src gen cfg out sys clean slow,$(eval std.$c_files := $$(std.$c_files)))

# Make each of the standard variables relative to the correct directory
std.src_files   := $(patsubst ./%,%,$(addprefix $(srcdir)/,$(std.src_files)))
std.gen_files   := $(patsubst ./%,%,$(addprefix $(srcdir)/,$(std.gen_files)))
std.cfg_files   := $(patsubst ./%,%,$(addprefix $(outdir)/,$(std.cfg_files)))
std.out_files   := $(patsubst ./%,%,$(addprefix $(outdir)/,$(std.out_files)))
std.sys_files   :=                  $(addprefix $(DESTDIR),$(std.sys_files))
std.clean_files := $(patsubst ./%,%,$(addprefix $(outdir)/,$(std.clean_files)))
std.slow_files  := $(patsubst ./%,%,$(addprefix $(outdir)/,$(std.slow_files)))

# Creative targets
$(outdir)/build      :        $(std.out_files)
$(outdir)/install    :        $(std.sys_files)
$(outdir)/installdirs: $(sort $(dir $(std.sys_files)))

# Destructive targets
_std.uninstall/$(outdir)        := $(std.sys_files)
_std.mostlyclean/$(outdir)      := $(filter-out $(std.slow_files) $(std.cfg_files) $(std.gen_files) $(std.src_files),$(std.clean_files))
_std.clean/$(outdir)            := $(filter-out                   $(std.cfg_files) $(std.gen_files) $(std.src_files),$(std.clean_files))
_std.distclean/$(outdir)        := $(filter-out                                    $(std.gen_files) $(std.src_files),$(std.clean_files))
_std.maintainer-clean/$(outdir) := $(filter-out                                                     $(std.src_files),$(std.clean_files))
$(addprefix $(outdir)/,mostlyclean clean distclean maintainer-clean): %: %-hook
	$(RM)    -- $(sort $(filter-out %/,$(_std.$(@F)/$(@D))))
	$(RM) -r -- $(sort $(filter     %/,$(_std.$(@F)/$(@D))))
	$(RMDIR_P) $(sort $(dir $(_std.$(@F)/$(@D)))) 2>/dev/null || $(TRUE)
# separate uninstall to support GNU Coding Standards' NORMAL_UNINSTALL
$(addprefix $(outdir)/,uninstall): %: %-hook
	$(NORMAL_UNINSTALL)
	$(RM)    -- $(sort $(filter-out %/,$(_std.$(@F)/$(@D))))
	$(RM) -r -- $(sort $(filter     %/,$(_std.$(@F)/$(@D))))
	$(RMDIR_P) $(sort $(dir $(_std.$(@F)/$(@D)))) 2>/dev/null || $(TRUE)
$(foreach t,uninstall mostlyclean clean distclean maintainer-clean, $(outdir)/$t-hook)::
.PHONY: $(foreach t,uninstall mostlyclean clean distclean maintainer-clean, $(outdir)/$t-hook)
