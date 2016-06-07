std.out_files += $(noinst_LTLIBRARIES) $(lib_LTLIBRARIES)
std.sys_files += $(addprefix $(libdir)/,$(lib_LTLIBRARIES))

std.out_files += $(bin_PROGRAMS) $(libexec_PROGRAMS)
std.sys_files += $(addprefix $(bindir)/,$(bin_PROGRAMS))
std.sys_files += $(addprefix $(libexecdir)/,$(libexec_PROGRAMS))

std.out_files += $(notdir $(pkgconfiglib_DATA))
std.sys_files += $(addprefix $(pkgconfiglibdir)/,$(notdir $(lib_pkgconfiglib_DATA)))

$(foreach n,$(call automake_name,$(std.out_files)),\
  $(eval $n_SOURCES ?=)\
  $(eval nodist_$n_SOURCES ?=)\
  $(eval $n_CFLAGS ?=)\
  $(eval $n_CPPFLAGS ?=)\
  $(eval $n_LDFLAGS ?=)\
  $(eval $n_LIBADD ?=))
$(foreach t,$(filter %.la,$(std.out_files)),\
	$(eval $(outdir)/$t: $(call at.path,$(call automake_lo,$t) $(call automake_libs,$t)) )\
	$(eval am.CFLAGS += $($(call automake_name,$t)_CFLAGS)                               )\
	$(eval am.CPPFLAGS += $($(call automake_name,$t)_CPPFLAGS)                           )\
	$(eval am.LDFLAGS += $($(call automake_name,$t)_LDFLAGS)                             ))
$(foreach t,$(bin_PROGRAMS) $(libexec_PROGRAMS),\
	$(eval $(outdir)/$t: $(call at.path,$(call automake_o,$t) $(call automake_libs,$t))  )\
	$(eval am.CFLAGS += $($(call automake_name,$t)_CFLAGS)                               )\
	$(eval am.CPPFLAGS += $($(call automake_name,$t)_CPPFLAGS)                           )\
	$(eval am.LDFLAGS += $($(call automake_name,$t)_LDFLAGS)                             ))
