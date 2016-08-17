mod.lt.description = (systemd) Easy handling of libtool dependencies
mod.lt.deps += files quote

_lt.patsubst-all = $(if $1,$(call _lt.patsubst-all,$(wordlist 2,$(words $1),$1),$2,$(patsubst $(firstword $1),$2,$3)),$3)
_lt.unLIBPATTERNS = $(foreach _lt.tmp,$1,$(if $(filter $(.LIBPATTERNS),$(notdir $(_lt.tmp))),$(call _lt.patsubst-all,$(.LIBPATTERNS),-l%,$(notdir $(_lt.tmp))),$(_lt.tmp)))
_lt.rest = $(wordlist 2,$(words $1),$1)
_lt.dedup = $(if $1,$(if $(filter $(firstword $1),$(call _lt.rest,$1)),,$(firstword $1) )$(call _lt.dedup,$(call _lt.rest,$1)))
_lt.static_dependency_libs = $(foreach _lt.tmp,$1,$(_lt.tmp)$(if $(filter %.la,$(_lt.tmp)), $(shell . $(_lt.tmp); echo $$static_dependency_libs)))

lt.lib.rpath = $(dir $(patsubst $(DESTDIR)%,%,$(filter %/$(@F),$(files.sys))))
lt.lib.files.all = $(call _lt.dedup,$(filter %.lo %.la -l%,$(call _lt.static_dependency_libs,$(call _lt.unLIBPATTERNS,$^))))
lt.lib.files.ld  = $(filter %.lo -l% $(if $(lt.lib.rpath),%.la),$(lt.lib.files.all))
lt.lib.files.la  = $(filter %.la,$(lt.lib.files.all))
lt.lib.post      = $(if $(lt.lib.files.la),printf '\nstatic_dependency_libs="$(lt.lib.files.la)"\ndependency_libs="$$dependency_libs $$static_dependency_libs"\n' >> $@)

lt.exe.files.all = $(call _lt.dedup,$(filter %.o %.la -l%,$(call _lt.static_dependency_libs,$(call _lt.unLIBPATTERNS,$^))))
lt.exe.files.ld  = $(lt.exe.files.all)
