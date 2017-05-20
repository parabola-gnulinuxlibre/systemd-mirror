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

# using _CFLAGS = in the conditional below would suppress AM_CFLAGS
journalctl_CFLAGS = \

journalctl_SOURCES = \
	src/journal/journalctl.c

journalctl_LDADD = \
	libsystemd-shared.la \
	libudev-core.la

ifneq ($(HAVE_QRENCODE),)
journalctl_SOURCES += \
	src/journal/journal-qrcode.c \
	src/journal/journal-qrcode.h

journalctl_CFLAGS += \
	$(QRENCODE_CFLAGS)

journalctl_LDADD += \
	$(QRENCODE_LIBS)
endif # HAVE_QRENCODE

rootbin_PROGRAMS += \
	journalctl

nodist_systemunit_DATA += \
	units/systemd-journal-flush.service \
	units/systemd-journal-catalog-update.service
SYSINIT_TARGET_WANTS += \
	systemd-journal-flush.service \
	systemd-journal-catalog-update.service
EXTRA_DIST += \
	units/systemd-journal-flush.service.in \
	units/systemd-journal-catalog-update.service.in

include $(topsrcdir)/build-aux/Makefile.tail.mk
