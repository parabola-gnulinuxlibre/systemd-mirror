mod.write-atomic.description = `write-atomic` auxiliary build script
mod.write-atomic.files += $(topsrcdir)/build-aux/write-atomic
define mod.write-atomic.doc
# User variables:
#   - `WRITE_ATOMIC ?= $(topsrcdir)/build-aux/write-atomic`
# Inputs:
#   (none)
# Outputs:
#   (none)
#
# The $(WRITE_ATOMIC) program reads a file from stdin, and writes it to
# the file named in argv[1], but does so atomically.
#
# That is, the following lines are almost equivalient:
#
#     ... > $@
#     ... | $(WRITE_ATOMIC) $@
#
# The are only different in that one is atomic, while the other is not.
endef

WRITE_ATOMIC ?= $(topsrcdir)/build-aux/write-atomic
