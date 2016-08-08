mod.lt.description = Easy handling of libtool dependencies
mod.lt.deps += files

_lt.dups = $(sort $(foreach l,$1,$(if $(filter-out 1,$(words $(filter $l,$1))),$l)))
_lt.patsubst-all = $(if $1,$(call _sd.patsubst-all,$(wordlist 2,$(words $1),$1),$2,$(patsubst $(firstword $1),$2,$3)),$3)
_lt.unLIBPATTERNS = $(foreach _lt.tmp,$1,$(if $(filter $(.LIBPATTERNS),$(_lt.tmp)),$(call _lt.patsubst-all,$(.LIBPATTERNS),-l%,$(_lt.tmp)),$(_lt.tmp)))

# The semantics for the de-dup bit are a bit weird.  My head hurts thinking
# about them.  TODO: clarify/simplify/something
lt.rpath = $(dir $(patsubst $(DESTDIR)%,%,$(filter %/$(@F),$(files.sys))))
_lt.link_files = $(filter %.o %.lo %.la -l%,$(call _lt.unLIBPATTERNS$,$^))
lt.link_files = $(filter-out $(call _lt.dups,$(_lt.link_files)),$(_lt.link_files))
