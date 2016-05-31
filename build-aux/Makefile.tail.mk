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

# This bit gets evaluated for each Makefile processed

include $(call _at.reverse,$(sort $(wildcard $(topsrcdir)/build-aux/Makefile.each.tail/*.mk)))

at.subdirs := $(patsubst ./%,%,$(addprefix $(outdir)/,$(at.subdirs)))
at.depdirs := $(patsubst ./%,%,$(addprefix $(outdir)/,$(at.depdirs)))

# Move all of the dirlocal variables to their namespaced version
$(foreach v,$(at.dirlocal),$(eval $v/$(outdir) := $$($v)))
$(foreach v,$(at.dirlocal),$(eval undefine $v))

# Remember that this is a directory that we've visited
_at.outdirs := $(_at.outdirs) $(outdir)

# Generic phony target declarations:
# mark them phony
.PHONY: $(addprefix $(outdir)/,$(at.phony))
# have them depend on subdirs
$(foreach t,$(at.phony),$(eval $(outdir)/$t: $(addsuffix /$t,$(at.subdirs/$(outdir)))))

# Include Makefiles from other directories
$(foreach _at.NO_ONCE,y,\
	$(foreach makefile,$(call at.path,$(addsuffix /Makefile,$(at.subdirs/$(outdir)) $(at.depdirs/$(outdir)))),\
		$(eval include $(filter-out $(_at.included_makefiles),$(makefile)))))

# This bit only gets evaluated once, after all of the other Makefiles are read
ifeq ($(_at.NO_ONCE),)

outdir = /bogus
srcdir = /bogus

$(foreach v,$(at.dirlocal),$(eval $v=))

include $(call _at.reverse,$(sort $(wildcard $(topsrcdir)/build-aux/Makefile.once.tail/*.mk)))

endif # _at.NO_ONCE
