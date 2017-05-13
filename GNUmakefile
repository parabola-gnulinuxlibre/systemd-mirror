# This file is a hack to let us pass whatever flags we want to Make,
# since adjusting MAKEFLAGS at runtime only half-works.
#
# Most of the complexity is dancing around to avoid having any
# possibly conflicting identifiers.

MAKEFLAGS += --no-print-directory
rest = $(wordlist 2,$(words $1),$1)
target = $(or $(firstword $(MAKECMDGOALS)),default)
$(target):
	@+$(MAKE) -f Makefile --no-builtin-rules --no-builtin-variables --warn-undefined-variables $(MAKECMDGOALS)
$(or $(call rest,$(MAKECMDGOALS)),_$(target)): $(target)
	@:
.PHONY: $(sort $(target) _$(target) $(MAKECMDGOALS))
