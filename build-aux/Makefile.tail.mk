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

include $(wildcard $(topsrcdir)/build-aux/Makefile.each.tail/*.mk)

# Make the namespaced versions of all of the dirlocal variables
$(foreach v,$($(_am)dirlocal),$(eval $v/$(outdir) = $($v)))

# Remember that this is a directory that we've visited
_am_outdirs := $(_am_outdirs) $(outdir)

# Generic phony target declarations:
# mark them phony
.PHONY: $(addprefix $(outdir)/,$($(_am)phony))
# have them depend on subdirs
$(foreach t,$($(_am)phony),$(eval $(outdir)/$t: $(addsuffix /$t,$(subdirs))))

# Include Makefiles from other directories

define _am_nl


endef
$(foreach _am_NO_ONCE,y,\
	$(foreach makefile,$(call am_path,$(addsuffix /Makefile,$($(_am)subdirs) $($(_am)depdirs))),\
		$(eval include $(filter-out $(_am_included_makefiles),$(makefile)))))

# This bit only gets evaluated once, after all of the other Makefiles are read
ifeq ($(_am_NO_ONCE),)

outdir = /bogus
srcdir = /bogus

$(foreach v,$($(_am)dirlocal),$(eval $v=))

include $(wildcard $(topsrcdir)/build-aux/Makefile.once.tail/*.mk)

endif # _am_NO_ONCE
