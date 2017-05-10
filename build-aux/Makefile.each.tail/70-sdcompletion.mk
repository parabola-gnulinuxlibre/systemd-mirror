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

_bashcompletion_DATA := $(sort $(bashcompletion_DATA) $(rootbin_PROGRAMS) $(bin_PROGRAMS) $(dist_bin_SCRIPTS))
dist_bashcompletion_DATA   += $(filter     $(files.src),$(_bashcompletion_DATA))
nodist_bashcompletion_DATA += $(filter-out $(files.src),$(_bashcompletion_DATA))

_zshcompletion_DATA := $(sort $(zshcompletion_DATA) $(addprefix _,$(notdir $(rootbin_PROGRAMS) $(bin_PROGRAMS) $(dist_bin_SCRIPTS))))
dist_zshcompletion_DATA   += $(filter     $(files.src),$(_zshcompletion_DATA))
nodist_zshcompletion_DATA += $(filter-out $(files.src),$(_zshcompletion_DATA))
