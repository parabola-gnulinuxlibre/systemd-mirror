MAKEFLAGS += --no-builtin-variables

# This version is more correct, but is slower:
#   $(foreach v,$(shell bash -c 'comm -23 <(env -i $(MAKE) -f - <<<"\$$(info \$$(.VARIABLES))all:"|sed "s/ /\n/g"|sort) <(env -i $(MAKE) -R -f - <<<"\$$(info \$$(.VARIABLES))all:"|sed "s/ /\n/g"|sort)'),
#     $(if $(filter default,$(origin $v)),$(eval undefine $v)))

_default_variables = $(foreach v,$(.VARIABLES),$(if $(filter default,$(origin $v)),$v))
$(foreach v,$(filter-out .% MAKE% SUFFIXES,$(_default_variables))\
            $(filter .LIBPATTERNS MAKEINFO,$(_default_variables)),\
  $(eval undefine $v))
undefine _default_variables

# Because Make uses .LIBPATTERNS internally, it should always be
# defined in case --warn-undefined-variables
.LIBPATTERNS ?=
