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
	libshared.la

libshared_la_SOURCES = \
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
	src/shared/tests.h \
	src/shared/tests.c

ifneq ($(HAVE_UTMP),)
libshared_la_SOURCES += \
	src/shared/utmp-wtmp.c
endif # HAVE_UTMP

ifneq ($(HAVE_SECCOMP),)
libshared_la_SOURCES += \
	src/shared/seccomp-util.h \
	src/shared/seccomp-util.c
endif # HAVE_SECCOMP

ifneq ($(HAVE_ACL),)
libshared_la_SOURCES += \
	src/shared/acl-util.c \
	src/shared/acl-util.h
endif # HAVE_ACL

libshared_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(ACL_CFLAGS) \
	$(LIBIDN_CFLAGS) \
	$(SECCOMP_CFLAGS)

libshared_la_LIBADD = \
	libsystemd-internal.la \
	libudev-internal.la \
	$(ACL_LIBS) \
	$(LIBIDN_LIBS) \
	$(SECCOMP_LIBS)

systemd.CPPFLAGS += -DPKGSYSCONFDIR=\"$(pkgsysconfdir)\"
systemd.CPPFLAGS += -DSYSTEM_CONFIG_UNIT_PATH=\"$(pkgsysconfdir)/system\"
systemd.CPPFLAGS += -DSYSTEM_DATA_UNIT_PATH=\"$(systemunitdir)\"
systemd.CPPFLAGS += -DSYSTEM_SYSVINIT_PATH=\"$(SYSTEM_SYSVINIT_PATH)\"
systemd.CPPFLAGS += -DSYSTEM_SYSVRCND_PATH=\"$(SYSTEM_SYSVRCND_PATH)\"
systemd.CPPFLAGS += -DUSER_CONFIG_UNIT_PATH=\"$(pkgsysconfdir)/user\"
systemd.CPPFLAGS += -DUSER_DATA_UNIT_PATH=\"$(userunitdir)\"

systemd.CPPFLAGS += -DSYSTEMD_FSCK_PATH=\"$(libexecdir)/systemd-fsck\"

systemd.CPPFLAGS += -DSYSTEMD_TTY_ASK_PASSWORD_AGENT_BINARY_PATH=\"$(bindir)/systemd-tty-ask-password-agent\"

systemd.CPPFLAGS += -DSYSTEM_GENERATOR_PATH=\"$(systemgeneratordir)\"
systemd.CPPFLAGS += -DUSER_GENERATOR_PATH=\"$(usergeneratordir)\"

systemd.CPPFLAGS += -DPOLKIT_AGENT_BINARY_PATH=\"$(bindir)/pkttyagent\"

include $(topsrcdir)/build-aux/Makefile.tail.mk
