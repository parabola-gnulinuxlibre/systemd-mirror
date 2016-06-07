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
include $(dir $(lastword $(MAKEFILE_LIST)))../../../config.mk
include $(topsrcdir)/build-aux/Makefile.head.mk

ifneq ($(ENABLE_LOGIND),)
systemd_logind_SOURCES = \
	src/login/logind.c \
	src/login/logind.h

nodist_systemd_logind_SOURCES = \
	src/login/logind-gperf.c

systemd_logind_LDADD = \
	liblogind-core.la

liblogind_core_la_SOURCES = \
	src/login/logind-core.c \
	src/login/logind-device.c \
	src/login/logind-device.h \
	src/login/logind-button.c \
	src/login/logind-button.h \
	src/login/logind-action.c \
	src/login/logind-action.h \
	src/login/logind-seat.c \
	src/login/logind-seat.h \
	src/login/logind-session.c \
	src/login/logind-session.h \
	src/login/logind-session-device.c \
	src/login/logind-session-device.h \
	src/login/logind-user.c \
	src/login/logind-user.h \
	src/login/logind-inhibit.c \
	src/login/logind-inhibit.h \
	src/login/logind-dbus.c \
	src/login/logind-session-dbus.c \
	src/login/logind-seat-dbus.c \
	src/login/logind-user-dbus.c \
	src/login/logind-utmp.c \
	src/login/logind-acl.h

liblogind_core_la_LIBADD = \
	libshared.la

ifneq ($(HAVE_ACL),)
liblogind_core_la_SOURCES += \
	src/login/logind-acl.c
endif # HAVE_ACL

noinst_LTLIBRARIES += \
	liblogind-core.la

libexec_PROGRAMS += \
	systemd-logind

loginctl_SOURCES = \
	src/login/loginctl.c \
	src/login/sysfs-show.h \
	src/login/sysfs-show.c

loginctl_LDADD = \
	libshared.la

bin_PROGRAMS += \
	loginctl

dist_bashcompletion_data += \
	shell-completion/bash/loginctl

dist_zshcompletion_data += \
	shell-completion/zsh/_loginctl \
	shell-completion/zsh/_systemd-inhibit

systemd_inhibit_SOURCES = \
	src/login/inhibit.c

systemd_inhibit_LDADD = \
	libshared.la

bin_PROGRAMS += \
	systemd-inhibit

test_login_SOURCES = \
	src/libsystemd/sd-login/test-login.c

test_login_LDADD = \
	libshared.la

test_login_shared_SOURCES = \
	src/login/test-login-shared.c

test_login_shared_LDADD = \
	libshared.la

test_inhibit_SOURCES = \
	src/login/test-inhibit.c

test_inhibit_LDADD = \
	libshared.la

test_login_tables_SOURCES = \
	src/login/test-login-tables.c

test_login_tables_LDADD = \
	liblogind-core.la

manual_tests += \
	test-login \
	test-inhibit

tests += \
	test-login-tables \
	test-login-shared

ifneq ($(HAVE_PAM),)
pam_systemd_la_SOURCES = \
	src/login/pam_systemd.sym \
	src/login/pam_systemd.c

pam_systemd_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(PAM_CFLAGS)

pam_systemd_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-module \
	-export-dynamic \
	-avoid-version \
	-shared \
	-Wl,--version-script=$(top_srcdir)/src/login/pam_systemd.sym

pam_systemd_la_LIBADD = \
	libshared.la \
	$(PAM_LIBS)

pamlib_LTLIBRARIES = \
	pam_systemd.la

dist_pamconf_DATA = \
	src/login/systemd-user

EXTRA_DIST += \
	src/login/systemd-user.m4
endif # HAVE_PAM

nodist_systemunit_DATA += \
	units/systemd-logind.service

dist_systemunit_DATA += \
	units/user.slice

dist_systemunit_DATA_busnames += \
	units/org.freedesktop.login1.busname

dist_dbussystemservice_DATA += \
	src/login/org.freedesktop.login1.service

dist_dbuspolicy_DATA += \
	src/login/org.freedesktop.login1.conf

dist_pkgsysconf_DATA += \
	src/login/logind.conf

polkitpolicy_files += \
	src/login/org.freedesktop.login1.policy

INSTALL_DIRS += \
	$(systemdstatedir)

MULTI_USER_TARGET_WANTS += \
	systemd-logind.service

SYSTEM_UNIT_ALIASES += \
	systemd-logind.service dbus-org.freedesktop.login1.service

BUSNAMES_TARGET_WANTS += \
	org.freedesktop.login1.busname

dist_udevrules_DATA += \
	src/login/70-uaccess.rules \
	src/login/70-power-switch.rules

nodist_udevrules_DATA += \
	src/login/71-seat.rules \
	src/login/73-seat-late.rules

endif # ENABLE_LOGIND

polkitpolicy_in_files += \
	src/login/org.freedesktop.login1.policy.in

gperf_gperf_sources += \
	src/login/logind-gperf.gperf

EXTRA_DIST += \
	src/login/71-seat.rules.in \
	src/login/73-seat-late.rules.in \
	units/systemd-logind.service.in

# ------------------------------------------------------------------------------
ifneq ($(HAVE_PAM),)

systemd_user_sessions_SOURCES = \
	src/user-sessions/user-sessions.c

systemd_user_sessions_LDADD = \
	libshared.la

libexec_PROGRAMS += \
	systemd-user-sessions

nodist_systemunit_DATA += \
	units/systemd-user-sessions.service

MULTI_USER_TARGET_WANTS += \
	systemd-user-sessions.service

endif # HAVE_PAM

EXTRA_DIST += \
	units/systemd-user-sessions.service.in

include $(topsrcdir)/build-aux/Makefile.tail.mk
