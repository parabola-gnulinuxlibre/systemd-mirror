mod.write-ifchanged.description = `write-ifchanged` auxiliary build script
mod.write-ifchanged.files += $(topsrcdir)/build-aux/write-ifchanged
define mod.write-ifchanged.doc
# User variables:
#   - `WRITE_IFCHANGED ?= $(topsrcdir)/build-aux/write-ifchanged`
# Inputs:
#   (none)
# Outputs:
#   (none)
#
# The $(WRITE_IFCHANGED) program reads a file from stdin, and writes it to the
# file named in argv[1], but does so atomically, but more importantly, does so
# in a way that does not bump the file's ctime if the new content is the same
# as the old content.
#
# That is, the following lines are almost equivalient:
#
#     ... > $@
#     ... | $(WRITE_ATOMIC) $@
endef
mod.write-ifchanged.doc := $(value mod.write-ifchanged.doc)

WRITE_IFCHANGED ?= $(topsrcdir)/build-aux/write-ifchanged
