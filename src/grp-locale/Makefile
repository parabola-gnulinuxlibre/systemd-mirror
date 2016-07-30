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

ifneq ($(ENABLE_LOCALED),)
systemd_localed_SOURCES = \
	src/locale/localed.c

systemd_localed_LDADD = \
	libshared.la \
	-ldl

systemd_localed_CFLAGS = \
	$(AM_CFLAGS) \
	$(XKBCOMMON_CFLAGS)

nodist_systemunit_DATA += \
	units/systemd-localed.service

dist_systemunit_DATA_busnames += \
	units/org.freedesktop.locale1.busname

rootlibexec_PROGRAMS += \
	systemd-localed

dist_dbuspolicy_DATA += \
	src/locale/org.freedesktop.locale1.conf

dist_dbussystemservice_DATA += \
	src/locale/org.freedesktop.locale1.service

polkitpolicy_files += \
	src/locale/org.freedesktop.locale1.policy

SYSTEM_UNIT_ALIASES += \
	systemd-localed.service dbus-org.freedesktop.locale1.service

BUSNAMES_TARGET_WANTS += \
	org.freedesktop.locale1.busname

dist_pkgdata_DATA = \
	src/locale/kbd-model-map \
	src/locale/language-fallback-map

localectl_SOURCES = \
	src/locale/localectl.c

localectl_LDADD = \
	libshared.la

bin_PROGRAMS += \
	localectl

dist_bashcompletion_data += \
	shell-completion/bash/localectl

dist_zshcompletion_data += \
	shell-completion/zsh/_localectl
endif # ENABLE_LOCALED

.PHONY: update-kbd-model-map

polkitpolicy_in_files += \
	src/locale/org.freedesktop.locale1.policy.in

EXTRA_DIST += \
	units/systemd-localed.service.in

systemd.CPPFLAGS += -DSYSTEMD_KBD_MODEL_MAP=\"$(pkgdatadir)/kbd-model-map\"
systemd.CPPFLAGS += -DSYSTEMD_LANGUAGE_FALLBACK_MAP=\"$(pkgdatadir)/language-fallback-map\"

include $(topsrcdir)/build-aux/Makefile.tail.mk
