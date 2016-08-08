_mod.each := $(sort $(_mod.each) $(filter-out $(_mod.once),$(_mod.vars)))
