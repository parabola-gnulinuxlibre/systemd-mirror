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
$(_am)clean_files += $($(_am)gen_files) $($(_am)cfg_files) $($(_am)out_files)

# Make each of the standard variables relative to the correct directory 
$(_am)src_files   := $(addprefix $(srcdir)/,$($(_am)src_files))
$(_am)gen_files   := $(addprefix $(srcdir)/,$($(_am)gen_files))
$(_am)cfg_files   := $(addprefix $(outdir)/,$($(_am)cfg_files))
$(_am)out_files   := $(addprefix $(outdir)/,$($(_am)out_files))
$(_am)sys_files   := $(addprefix $(DESTDIR),$($(_am)sys_files))
$(_am)clean_files := $(addprefix $(outdir)/,$($(_am)clean_files))
$(_am)slow_files  := $(addprefix $(outdir)/,$($(_am)slow_files))
$(_am)subdirs     := $(addprefix $(outdir)/,$($(_am)subdirs))

# Creative targets
$(outdir)/build      :       $($(_am)out_files/$(outdir))
$(outdir)/install    :       $($(_am)sys_files/$(outdir))
$(outdir)/installdirs: $(dir $($(_am)sys_files/$(outdir)))

# Destructive targets
_am_uninstall/$(outdir)        = $(_am_sys_files/$(outdir))
_am_mostlyclean/$(outdir)      = $(filter-out $(_am_slow_files/$(outdir)) $(_am_cfg_files/$(outdir)) $(_am_gen_files/$(outdir)) $(_am_src_files/$(outdir)),$(_am_clean_files/$(outdir)))
_am_clean/$(outdir)            = $(filter-out                             $(_am_cfg_files/$(outdir)) $(_am_gen_files/$(outdir)) $(_am_src_files/$(outdir)),$(_am_clean_files/$(outdir)))
_am_distclean/$(outdir)        = $(filter-out                                                        $(_am_gen_files/$(outdir)) $(_am_src_files/$(outdir)),$(_am_clean_files/$(outdir)))
_am_maintainer-clean/$(outdir) = $(filter-out                                                                                   $(_am_src_files/$(outdir)),$(_am_clean_files/$(outdir)))
$(addprefix $(outdir)/,uninstall mostlyclean clean distclean maintainer-clean):
	$(RM) -- $(sort $(_am_$(@F)/$(@D)))
	$(RMDIRS) $(sort $(dir $(_am_$(@F)/$(@D)))) 2>/dev/null || $(TRUE)
