include $(dir $(lastword $(MAKEFILE_LIST)))/../../../../config.mk
include $(topsrcdir)/build-aux/Makefile.head.mk

AM_CPPFLAGS += $(libsystemd.CPPFLAGS)
AM_CPPFLAGS += $(libbasic.CPPFLAGS)
AM_CPPFLAGS += $(libshared.CPPFLAGS)
AM_CPPFLAGS += -DLIBDIR=\"$(libdir)\"
AM_CPPFLAGS += -DUDEVLIBEXECDIR=\"$(udevlibexecdir)\"

include $(topsrcdir)/build-aux/Makefile.tail.mk
