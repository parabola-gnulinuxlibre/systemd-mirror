include $(dir $(lastword $(MAKEFILE_LIST)))/../../../../config.mk
include $(topsrcdir)/build-aux/Makefile.head.mk

systemd.CPPFLAGS += $(libsystemd.CPPFLAGS)
systemd.CPPFLAGS += $(libbasic.CPPFLAGS)
systemd.CPPFLAGS += $(libshared.CPPFLAGS)
systemd.CPPFLAGS += -DLIBDIR=\"$(libdir)\"
systemd.CPPFLAGS += -DUDEVLIBEXECDIR=\"$(udevlibexecdir)\"

include $(topsrcdir)/build-aux/Makefile.tail.mk
