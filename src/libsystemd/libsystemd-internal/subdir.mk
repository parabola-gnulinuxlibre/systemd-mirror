include $(dir $(lastword $(MAKEFILE_LIST)))/../../../../config.mk
include $(topsrcdir)/automake.head.mk

#CPPFLAGS += $(libsystemd.CPPFLAGS) $(libbasic.CPPFLAGS) $(libshared.CPPFLAGS)
#CPPFLAGS += -DLIBDIR=\"$(libdir)\" -DUDEVLIBEXECDIR=\"$(udevlibexecdir)\"

include $(topsrcdir)/automake.tail.mk
