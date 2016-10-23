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

ifneq ($(HAVE_LIBCRYPTSETUP),)
rootlibexec_PROGRAMS += \
	systemd-cryptsetup

systemgenerator_PROGRAMS += \
	systemd-cryptsetup-generator

dist_systemunit_DATA += \
	units/cryptsetup.target \
	units/cryptsetup-pre.target

systemd_cryptsetup_SOURCES = \
	src/cryptsetup/cryptsetup.c

systemd_cryptsetup_CFLAGS = \
	$(LIBCRYPTSETUP_CFLAGS)

systemd_cryptsetup_LDADD = \
	libsystemd-shared.la \
	$(LIBCRYPTSETUP_LIBS)

systemd_cryptsetup_generator_SOURCES = \
	src/cryptsetup/cryptsetup-generator.c

systemd_cryptsetup_generator_LDADD = \
	libsystemd-shared.la

SYSINIT_TARGET_WANTS += \
	cryptsetup.target

endif # HAVE_LIBCRYPTSETUP

include $(topsrcdir)/build-aux/Makefile.tail.mk
