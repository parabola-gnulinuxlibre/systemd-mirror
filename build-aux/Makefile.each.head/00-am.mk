$(eval $(foreach v,$(foreach p,$(am.primaries),am.inst_$p am.noinst_$p am.check_$p),$v ?=$(at.nl)))
am.CFLAGS ?=
am.CPPFLAGS ?=
am.subdirs ?=
