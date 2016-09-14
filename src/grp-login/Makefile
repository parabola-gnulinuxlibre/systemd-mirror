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

test_login_SOURCES = \
	src/libsystemd/sd-login/test-login.c

test_login_LDADD = \
	libsystemd-shared.la

test_login_shared_SOURCES = \
	src/login/test-login-shared.c

test_login_shared_LDADD = \
	libsystemd-shared.la

test_inhibit_SOURCES = \
	src/login/test-inhibit.c

test_inhibit_LDADD = \
	libsystemd-shared.la

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

nested.subdirs += loginctl
nested.subdirs += pam_systemd
nested.subdirs += systemd-inhibit
nested.subdirs += systemd-logind

include $(topsrcdir)/build-aux/Makefile.tail.mk
