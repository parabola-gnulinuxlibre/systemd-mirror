_dist.files := $(strip $(_dist.files) $(call at.addprefix,$(srcdir),$(filter-out $(files.src.int),$(files.src))))
