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
include $(dir $(lastword $(MAKEFILE_LIST)))/../../../config.mk
include $(topsrcdir)/build-aux/Makefile.head.mk

noinst_LTLIBRARIES += \
	libsystemd-shared.la

libsystemd_shared_la_SOURCES = \
	src/shared/output-mode.h \
	src/shared/output-mode.c \
	src/shared/gpt.h \
	src/shared/udev-util.h \
	src/shared/linux/auto_dev-ioctl.h \
	src/shared/initreq.h \
	src/shared/dns-domain.c \
	src/shared/dns-domain.h \
	src/shared/efivars.c \
	src/shared/efivars.h \
	src/shared/fstab-util.c \
	src/shared/fstab-util.h \
	src/shared/sleep-config.c \
	src/shared/sleep-config.h \
	src/shared/conf-parser.c \
	src/shared/conf-parser.h \
	src/shared/pager.c \
	src/shared/pager.h \
	src/shared/spawn-polkit-agent.c \
	src/shared/spawn-polkit-agent.h \
	src/shared/apparmor-util.c \
	src/shared/apparmor-util.h \
	src/shared/ima-util.c \
	src/shared/ima-util.h \
	src/shared/ptyfwd.c \
	src/shared/ptyfwd.h \
	src/shared/base-filesystem.c \
	src/shared/base-filesystem.h \
	src/shared/uid-range.c \
	src/shared/uid-range.h \
	src/shared/install.c \
	src/shared/install.h \
	src/shared/install-printf.c \
	src/shared/install-printf.h \
	src/shared/path-lookup.c \
	src/shared/path-lookup.h \
	src/shared/specifier.c \
	src/shared/specifier.h \
	src/shared/dev-setup.c \
	src/shared/dev-setup.h \
	src/shared/dropin.c \
	src/shared/dropin.h \
	src/shared/condition.c \
	src/shared/condition.h \
	src/shared/clean-ipc.c \
	src/shared/clean-ipc.h \
	src/shared/generator.h \
	src/shared/generator.c \
	src/shared/acpi-fpdt.h \
	src/shared/acpi-fpdt.c \
	src/shared/boot-timestamps.h \
	src/shared/boot-timestamps.c \
	src/shared/cgroup-show.c \
	src/shared/cgroup-show.h \
	src/shared/utmp-wtmp.h \
	src/shared/watchdog.c \
	src/shared/watchdog.h \
	src/shared/spawn-ask-password-agent.c \
	src/shared/spawn-ask-password-agent.h \
	src/shared/ask-password-api.c \
	src/shared/ask-password-api.h \
	src/shared/switch-root.h \
	src/shared/switch-root.c \
	src/shared/import-util.c \
	src/shared/import-util.h \
	src/shared/sysctl-util.c \
	src/shared/sysctl-util.h \
	src/shared/bus-util.c \
	src/shared/bus-util.h \
	src/shared/logs-show.c \
	src/shared/logs-show.h \
	src/shared/machine-image.c \
	src/shared/machine-image.h \
	src/shared/machine-pool.c \
	src/shared/machine-pool.h \
	src/shared/resolve-util.c \
	src/shared/resolve-util.h \
	src/shared/bus-unit-util.c \
	src/shared/bus-unit-util.h \
	src/shared/vlan-util.h \
	src/shared/vlan-util.c \
	src/shared/tests.h \
	src/shared/tests.c \
	src/shared/fdset.c \
	src/shared/fdset.h

ifneq ($(HAVE_UTMP),)
libsystemd_shared_la_SOURCES += \
	src/shared/utmp-wtmp.c
endif # HAVE_UTMP

ifneq ($(HAVE_SECCOMP),)
libsystemd_shared_la_SOURCES += \
	src/shared/seccomp-util.h \
	src/shared/seccomp-util.c
endif # HAVE_SECCOMP

ifneq ($(HAVE_ACL),)
libsystemd_shared_la_SOURCES += \
	src/shared/acl-util.c \
	src/shared/acl-util.h
endif # HAVE_ACL

libsystemd_shared_la_CFLAGS = \
	$(ACL_CFLAGS) \
	$(LIBIDN_CFLAGS) \
	$(SECCOMP_CFLAGS)

libsystemd_shared_la_LIBADD = \
	libsystemd-internal.la \
	libsystemd-basic.la \
	libsystemd-journal-internal.la \
	libudev-internal.la \
	$(ACL_LIBS) \
	$(LIBIDN_LIBS) \
	$(SECCOMP_LIBS)

rootlibexec_LTLIBRARIES += \
	libsystemd-shared.la

libsystemd_shared_la_SOURCES = \
	$(libsystemd_basic_la_SOURCES) \
	$(libsystemd_shared_la_SOURCES) \
	$(libsystemd_internal_la_SOURCES) \
	$(libsystemd_journal_internal_la_SOURCES) \
	$(libudev_internal_la_SOURCES)

libsystemd_shared_la_CFLAGS = \
	$(libsystemd_basic_la_CFLAGS) \
	$(libsystemd_shared_la_CFLAGS) \
	$(libsystemd_internal_la_CFLAGS) \
	$(libsystemd_journal_internal_la_CFLAGS) \
	$(libudev_internal_la_CFLAGS) \
	$(ACL_CFLAGS) \
	$(LIBIDN_CFLAGS) \
	$(SECCOMP_CFLAGS) \
	-fvisibility=default

# We can't use libsystemd_shared_la_LIBADD here because it would
# pull in libsystemd*-internal.la
libsystemd_shared_la_LIBADD = \
	libsystemd-basic.la \
	libsystemd_internal.la \
	libsystemd_journal_internal.la \
	libudev_internal.la \
	$(ACL_LIBS) \
	$(LIBIDN_LIBS) \
	$(SECCOMP_LIBS)

libsystemd_shared_la_LDFLAGS = \
	-release $(PACKAGE_VERSION)

include $(topsrcdir)/build-aux/Makefile.tail.mk
