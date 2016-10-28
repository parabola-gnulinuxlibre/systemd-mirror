_dist.files := $(strip $(_dist.files) $(call at.addprefix,$(srcdir),$(files.src)))
