-include $(outdir)/$(DEPDIR)/*.P*
std.clean_files += *.o *.lo .deps/ .libs/
include $(topsrcdir)/am-pat-rules.mk
