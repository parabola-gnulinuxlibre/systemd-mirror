mod.sdcompletion.description = (systemd) shell completion
mod.sdcompletion.depends += am
define mod.sdcompletion.doc
# Inputs:
#   - Directory variable : `rootbin_PROGRAMS`
#   - Directory variable : `bin_PROGRAMS`
#   - Directory variable : `dist_bin_SCRIPTS`
#   - Directory variable : `bashcompletion_DATA`
#   - Directory variable : `zshcompletion_DATA`
# Outputs:
#   - Directory variable : `dist_bashcompletion_DATA`
#   - Directory variable : `dist_zshcompletion_DATA`
endef
mod.sdcompletion.doc := $(value mod.sdcompletion.doc)

rootbin_PROGRAMS ?=
bin_PROGRAMS ?=
dist_bin_SCRIPTS ?=

_pf = $(patsubst $1,$2,$(filter $1,$3))

dist_bashcompletion_DATA ?=
nodist_bashcompletion_DATA ?=
_bashcompletion_DATA := $(notdir $(rootbin_PROGRAMS) $(bin_PROGRAMS) $(dist_bin_SCRIPTS))
dist_bashcompletion_DATA   := $(sort   $(dist_bashcompletion_DATA) $(filter     $(call _pf,%.completion.bash,%,$(files.src)),$(_bashcompletion_DATA)))
nodist_bashcompletion_DATA := $(sort $(nodist_bashcompletion_DATA) $(filter-out $(call _pf,%.completion.bash,%,$(files.src)),$(_bashcompletion_DATA)))
undefine _bashcompletion_DATA

dist_zshcompletion_DATA ?=
nodist_zshcompletion_DATA ?=
_zshcompletion_DATA := $(addprefix _,$(notdir $(rootbin_PROGRAMS) $(bin_PROGRAMS) $(dist_bin_SCRIPTS)))
dist_zshcompletion_DATA   := $(sort   $(dist_zshcompletion_DATA) $(filter     $(call _pf,%.completion.zsh,_%,$(files.src)),$(_zshcompletion_DATA)))
nodist_zshcompletion_DATA := $(sort $(nodist_zshcompletion_DATA) $(filter-out $(call _pf,%.completion.zsh,_%,$(files.src)),$(_zshcompletion_DATA)))
undefine _zshcompletion_DATA
