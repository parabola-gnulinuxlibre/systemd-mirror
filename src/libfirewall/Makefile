ifneq ($(HAVE_LIBIPTC),)
noinst_LTLIBRARIES += \
	libfirewall.la

libfirewall_la_SOURCES = \
	src/shared/firewall-util.h \
	src/shared/firewall-util.c

libfirewall_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(LIBIPTC_CFLAGS)

libfirewall_la_LIBADD = \
	$(LIBIPTC_LIBS)
endif

include $(topsrcdir)/automake.tail.mk
