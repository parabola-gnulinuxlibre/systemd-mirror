dir_variables = $(foreach v,$(filter-out _%,$(patsubst %/$(@D),%,$(filter %/$(@D),$(.VARIABLES)))),$(if $(findstring /,$v),, $v))
$(outdir)/directory-info:
	$(AM_V_at)printf '%s = «%s»\n' $(foreach v,$(dir_variables),$(if $($v/$(@D)),'$v' '$($v/$(@D))')) | sort | column -s= -o= -t
.PHONY: $(outdir)/module-info
