$(eval $(foreach v,$(foreach p,$(am.primaries),am.sys_$p am.out_$p am.check_$p),$v ?=$(at.nl)))
am.CFLAGS ?=
am.CPPFLAGS ?=
am.subdirs ?=
