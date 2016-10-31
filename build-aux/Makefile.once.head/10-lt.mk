mod.lt.description = (systemd) Easy handling of libtool dependencies
mod.lt.depends += files
define mod.lt.doc
# Inputs:
#   - Global variable    : `$(.LIBPATTERNS)`
#   - Directory variable : `$(files.sys)`
#   - Target variable    : `$@`
#   - Target variable    : `$(@F)`
#   - Target variable    : `$^`
# Outputs:
#   - Target variable : `lt.lib.rpath`     (`$(@D)`, `$(files.sys)`)
#   - Target variable : `lt.lib.files.all` (`$^`, `$(.LIBPATTERNS)`)
#   - Target variable : `lt.lib.files.ld`  (`$(lt.lib.rpath)`, `$(lt.lib.files.all)`)
#   - Target variable : `lt.lib.post`      (`$(lt.lib.rpath)`, `$(lt.lib.files.all)`)
#   - Target variable : `lt.exe.files.all` (`$^`, `$(.LIBPATTERNS)`)
#   - Target variable : `lt.exe.files.ld`  (an alias for `lt.exe.files.all`)
#
# A set of variables to make interacting with libtool a little easier.
#
# libtool tries to abstract away the difference between static libraries
# and dynamic libraries.  But, it does a crappy job.  If one library
# depends on another library, you have to care about which of them are
# static and which are dynamic, or you'll either end up with missing
# symbols, or duplicate symbols.  Either way, it will fail to link your
# executable.
#
# So, our workaround: don't pass any .la libraries to libtool/ld when
# linking a convenience library, but then run a post-libtool command to
# insert them into the .la file's dependency_libs. This uses the
# emptiness of lt.lib.rpath to determine if a library is a static
# convenience library or not.
endef
mod.lt.doc := $(value mod.lt.doc)

_lt.patsubst-all = $(if $1,$(call _lt.patsubst-all,$(wordlist 2,$(words $1),$1),$2,$(patsubst $(firstword $1),$2,$3)),$3)
_lt.unLIBPATTERNS = $(foreach _lt.tmp,$1,$(if $(filter $(.LIBPATTERNS),$(notdir $(_lt.tmp))),$(call _lt.patsubst-all,$(.LIBPATTERNS),-l%,$(notdir $(_lt.tmp))),$(_lt.tmp)))
_lt.rest = $(wordlist 2,$(words $1),$1)
# Usage: $(call _lt.dedup,ITEM1 ITEM2 ITEM3)
# Removes duplicate items while keeping the order the same (the leftmost of a duplicate is used).
_lt.dedup = $(if $1,$(if $(filter $(firstword $1),$(call _lt.rest,$1)),,$(firstword $1) )$(call _lt.dedup,$(call _lt.rest,$1)))
# Usage: $(call _lt.dependency_libs,LIB1.la OBJ.lo LIB2.la)
#        => LIB1.la LIB1.la:dependency_libs OBJ.lo LIB2.la LIB2.la:dependency_libs
# Insert a .la library's dependency_libs after the library itself in the list.
_lt.dependency_libs = $(foreach _lt.tmp,$1,$(_lt.tmp)$(if $(filter %.la,$(_lt.tmp)), $(shell . $(_lt.tmp); echo $$dependency_libs)))

 lt.lib.rpath     = $(dir $(filter %/$(@F),$(files.sys)))
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
