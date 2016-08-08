mod.am.depends += files

$(eval \
  $(foreach p,$(am.primaries)  ,$(call _am.per_primary,$p)$(at.nl)))
$(eval \
  $(foreach f,$(am.PROGRAMS)   ,$(call _am.per_PROGRAM,$f,$(call am.file2var,$f))$(at.nl))\
  $(foreach f,$(am.LTLIBRARIES),$(call _am.per_LTLIBRARY,$f,$(call am.file2var,$f))$(at.nl))\
  $(foreach d,$(am.inst2dirs)  ,$(call _am.per_directory,$d)$(at.nl)))

files.sys.all += $(foreach p,$(am.primaries),$(am.inst_$p))
files.out.all += $(foreach p,$(am.primaries),$(am.noinst_$p))
files.out.all += $(foreach p,$(am.primaries),$(call am.inst2noinst_$p,$(am.inst_$p)))
files.out.check += $(foreach p,$(am.primaries),$(am.check_$p))
