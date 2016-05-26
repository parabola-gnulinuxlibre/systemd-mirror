#  -*- Mode: makefile; indent-tabs-mode: t -*-
#
#  This file is part of systemd.
#
#  Copyright 2010-2012 Lennart Poettering
#  Copyright 2010-2012 Kay Sievers
#  Copyright 2013 Zbigniew Jędrzejewski-Szmek
#  Copyright 2013 David Strauss
#  Copyright 2016 Luke Shumaker
#
#  systemd is free software; you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation; either version 2.1 of the License, or
#  (at your option) any later version.
#
#  systemd is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with systemd; If not, see <http://www.gnu.org/licenses/>.
include $(dir $(lastword $(MAKEFILE_LIST)))/../../config.mk
include $(topsrcdir)/automake.head.mk

systemd_nspawn_SOURCES = \
	src/nspawn/nspawn.c \
	src/nspawn/nspawn-settings.c \
	src/nspawn/nspawn-settings.h \
	src/nspawn/nspawn-mount.c \
	src/nspawn/nspawn-mount.h \
	src/nspawn/nspawn-network.c \
	src/nspawn/nspawn-network.h \
	src/nspawn/nspawn-expose-ports.c \
	src/nspawn/nspawn-expose-ports.h \
	src/nspawn/nspawn-cgroup.c \
	src/nspawn/nspawn-cgroup.h \
	src/nspawn/nspawn-register.c \
	src/nspawn/nspawn-register.h \
	src/nspawn/nspawn-setuid.c \
	src/nspawn/nspawn-setuid.h \
	src/nspawn/nspawn-stub-pid1.c \
	src/nspawn/nspawn-stub-pid1.h \
	src/core/mount-setup.c \
	src/core/mount-setup.h \
	src/core/loopback-setup.c \
	src/core/loopback-setup.h

am_out_files = systemd-nspawn
CPPFLAGS += -I$(topsrcdir)/src/libbasic
CPPFLAGS += -I$(topoutdir)/src/libbasic
CPPFLAGS += -I$(topsrcdir)/src/libshared
CPPFLAGS += -I$(topoutdir)/src/libshared
CPPFLAGS += -I$(topsrcdir)/src/libsystemd/include
CPPFLAGS += -I$(topoutdir)/src/libsystemd/include
CPPFLAGS += -I$(topsrcdir)/src/libudev/include
CPPFLAGS += -I$(topoutdir)/src/libudev/include
$(outdir)/systemd-nspawn: \
	$(outdir)/nspawn.o \
	$(outdir)/nspawn-settings.o \
	$(outdir)/nspawn-mount.o \
	$(outdir)/nspawn-network.o \
	$(outdir)/nspawn-expose-ports.o \
	$(outdir)/nspawn-cgroup.o \
	$(outdir)/nspawn-register.o \
	$(outdir)/nspawn-setuid.o \
	$(outdir)/nspawn-stub-pid1.o \
	$(outdir)/mount-setup.o \
	$(outdir)/loopback-setup.o \
	$(topsrcdir)/libshared/libshared.la $(BLKID_LIBS)

$(outdir)/mount-setup.c: $(topsrcdir)/src/libcore/mount-setup.c; $(CP) $< $@
$(outdir)/mount-setup.h: $(topsrcdir)/src/libcore/mount-setup.h; $(CP) $< $@
$(outdir)/loopback-setup.c: $(topsrcdir)/src/libcore/loopback-setup.c; $(CP) $< $@
$(outdir)/loopback-setup.h: $(topsrcdir)/src/libcore/loopback-setup.h; $(CP) $< $@

$(outdir)/nspawn.o: $(outdir)/loopback-setup.h

nodist_systemd_nspawn_SOURCES = \
	src/nspawn/nspawn-gperf.c

gperf_gperf_sources += \
	src/nspawn/nspawn-gperf.gperf

systemd_nspawn_CFLAGS = \
	$(AM_CFLAGS) \
	$(BLKID_CFLAGS) \
	$(SECCOMP_CFLAGS)

systemd_nspawn_LDADD = \
	libshared.la \
	$(BLKID_LIBS)

ifneq ($(HAVE_LIBIPTC),)
systemd_nspawn_LDADD += \
	libfirewall.la
endif

include $(topsrcdir)/automake.tail.mk
