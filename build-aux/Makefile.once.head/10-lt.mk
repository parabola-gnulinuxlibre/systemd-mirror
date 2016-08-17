mod.lt.description = (systemd) Easy handling of libtool dependencies
mod.lt.deps += files quote

_lt.patsubst-all = $(if $1,$(call _lt.patsubst-all,$(wordlist 2,$(words $1),$1),$2,$(patsubst $(firstword $1),$2,$3)),$3)
_lt.unLIBPATTERNS = $(foreach _lt.tmp,$1,$(if $(filter $(.LIBPATTERNS),$(notdir $(_lt.tmp))),$(call _lt.patsubst-all,$(.LIBPATTERNS),-l%,$(notdir $(_lt.tmp))),$(_lt.tmp)))
_lt.rest = $(wordlist 2,$(words $1),$1)
_lt.dedup = $(if $1,$(if $(filter $(firstword $1),$(call _lt.rest,$1)),,$(firstword $1) )$(call _lt.dedup,$(call _lt.rest,$1)))
_lt.dependency_libs = $(foreach _lt.tmp,$1,$(_lt.tmp)$(if $(filter %.la,$(_lt.tmp)), $(shell . $(_lt.tmp); echo $$dependency_libs)))

 lt.lib.rpath     = $(dir $(patsubst $(DESTDIR)%,%,$(filter %/$(@F),$(files.sys))))
 lt.lib.files.all = $(filter %.lo %.la -l%,$(call _lt.unLIBPATTERNS,$^))
 lt.lib.files.ld  = $(strip $(if $(lt.lib.rpath),\
                                 $(lt.lib.files.all),\
                                 $(filter-out %.la,$(lt.lib.files.all))))
_lt.lib.files.dep = $(strip $(call _lt.dedup,$(filter %.la -l%,$(call _lt.dependency_libs,$(lt.lib.files.all)))))
 lt.lib.post      = $(strip $(if $(lt.lib.rpath),\
                                 true,\
                                 sed -i 's|^dependency_libs=.*|dependency_libs='\''$(_lt.lib.files.dep)'\''|' $@))

lt.exe.files.all = $(filter %.o %.la -l%,$(call _lt.unLIBPATTERNS,$^))
lt.exe.files.ld  = $(lt.exe.files.all)
