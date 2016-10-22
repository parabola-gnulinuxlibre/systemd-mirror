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
include $(topsrcdir)/build-aux/Makefile.head.mk

bin_PROGRAMS += systemd-nspawn
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
	src/nspawn/nspawn-seccomp.c \
	src/nspawn/nspawn-seccomp.h \
	src/nspawn/nspawn-register.c \
	src/nspawn/nspawn-register.h \
	src/nspawn/nspawn-setuid.c \
	src/nspawn/nspawn-setuid.h \
	src/nspawn/nspawn-stub-pid1.c \
	src/nspawn/nspawn-stub-pid1.h \
	src/nspawn/nspawn-patch-uid.c \
	src/nspawn/nspawn-patch-uid.h \
	src/core/mount-setup.c \
	src/core/mount-setup.h \
	src/core/loopback-setup.c \
	src/core/loopback-setup.h

nodist_systemd_nspawn_SOURCES = \
	src/nspawn/nspawn-gperf.c

gperf_gperf_sources += \
	src/nspawn/nspawn-gperf.gperf

systemd_nspawn_CFLAGS = \
	$(ACL_CFLAGS) \
	$(BLKID_CFLAGS) \
	$(SECCOMP_CFLAGS) \
	$(SELINUX_CFLAGS)

systemd_nspawn_LDADD = \
	libsystemd-shared.la \
	$(ACL_LIBS) \
	$(BLKID_LIBS) \
	$(SECCOMP_LIBS) \
	$(SELINUX_LIBS)

ifneq ($(HAVE_LIBIPTC),)
systemd_nspawn_LDADD += \
	libsystemd-firewall.la
endif # HAVE_LIBIPTC

test_patch_uid_SOURCES = \
	src/nspawn/nspawn-patch-uid.c \
	src/nspawn/nspawn-patch-uid.h \
	src/nspawn/test-patch-uid.c

test_patch_uid_CFLAGS = \
	$(ACL_CFLAGS)

test_patch_uid_LDADD = \
	libsystemd-shared.la \
	$(ACL_LIBS)

manual_tests += \
	test-patch-uid

include $(topsrcdir)/build-aux/Makefile.tail.mk
