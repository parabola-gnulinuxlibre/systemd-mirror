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

# Make each of the standard variables relative to the correct directory 
std.src_files   := $(addprefix $(srcdir)/,$(std.src_files))
std.gen_files   := $(addprefix $(srcdir)/,$(std.gen_files))
std.cfg_files   := $(addprefix $(outdir)/,$(std.cfg_files))
std.out_files   := $(addprefix $(outdir)/,$(std.out_files))
std.sys_files   := $(addprefix $(DESTDIR),$(std.sys_files))
std.clean_files := $(addprefix $(outdir)/,$(std.clean_files))
std.slow_files  := $(addprefix $(outdir)/,$(std.slow_files))
std.subdirs     := $(addprefix $(outdir)/,$(std.subdirs))

# Creative targets
$(outdir)/build      :       $(std.out_files)
$(outdir)/install    :       $(std.sys_files)
$(outdir)/installdirs: $(dir $(std.sys_files))

# Destructive targets
_std.uninstall/$(outdir)        := $(_std.sys_files)
_std.mostlyclean/$(outdir)      := $(filter-out $(_std.slow_files) $(_std.cfg_files) $(_std.gen_files) $(_std.src_files),$(_std.clean_files))
_std.clean/$(outdir)            := $(filter-out                    $(_std.cfg_files) $(_std.gen_files) $(_std.src_files),$(_std.clean_files))
_std.distclean/$(outdir)        := $(filter-out                                      $(_std.gen_files) $(_std.src_files),$(_std.clean_files))
_std.maintainer-clean/$(outdir) := $(filter-out                                                        $(_std.src_files),$(_std.clean_files))
$(addprefix $(outdir)/,uninstall mostlyclean clean distclean maintainer-clean)::
	$(RM) -- $(sort $(_std.$(@F)/$(@D)))
	$(RMDIR_P) $(sort $(dir $(_std.$(@F)/$(@D)))) 2>/dev/null || $(TRUE)
