am.bindirs = bin rootbin libexec rootlibexec systemgenerator udevlibexec
at.dirlocal += am.CFLAGS am.CPPFLAGS am.LDFLAGS am.LIBTOOLFLAGS
at.dirlocal += noinst_LTLIBRARIES lib_LTLIBRARIES
at.dirlocal += $(addsuffix _PROGRAMS,noinst $(am.bindirs))
at.dirlocal += pkgconfiglib_DATA
automake_name = $(subst -,_,$(subst .,_,$1))
automake_sources = $(addprefix $(outdir)/,$(notdir $($(automake_name)_SOURCES) $(nodist_$(automake_name)_SOURCES)))
automake_lo = $(patsubst %.c,%.lo,$(filter %.c,$(automake_sources)))
automake_o = $(patsubst %.c,%.o,$(filter %.c,$(automake_sources)))
automake_lib = $(foreach l,   $($(automake_name)_$2),$(if $(filter lib%.la,$l), $($(l:.la=).DEPENDS)  , $l ))
automake_cpp = $(foreach l,$1 $($(automake_name)_$2),$(if $(filter lib%.la,$l), $($(l:.la=).CPPFLAGS) ,    ))
