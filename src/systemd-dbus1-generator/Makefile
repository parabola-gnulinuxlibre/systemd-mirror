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

systemgenerator_PROGRAMS +=  \
	systemd-dbus1-generator

systemd_dbus1_generator_SOURCES = \
	src/dbus1-generator/dbus1-generator.c

systemd_dbus1_generator_LDADD = \
	libshared.la

dbus1-generator-install-hook:
	$(AM_V_at)$(MKDIR_P) $(DESTDIR)$(usergeneratordir)
	$(AM_V_RM)rm -f $(DESTDIR)$(usergeneratordir)/systemd-dbus1-generator
	$(AM_V_LN)$(LN_S) --relative -f $(DESTDIR)$(systemgeneratordir)/systemd-dbus1-generator $(DESTDIR)$(usergeneratordir)/systemd-dbus1-generator

dbus1-generator-uninstall-hook:
	rm -f $(DESTDIR)$(usergeneratordir)/systemd-dbus1-generator

dist_xinitrc_SCRIPTS = \
	xorg/50-systemd-user.sh

INSTALL_EXEC_HOOKS += dbus1-generator-install-hook
UNINSTALL_EXEC_HOOKS += dbus1-generator-uninstall-hook

sd.CPPFLAGS += -DSYSTEM_DATA_UNIT_PATH=\"$(systemunitdir)\"
sd.CPPFLAGS += -DUSER_DATA_UNIT_PATH=\"$(userunitdir)\"

include $(topsrcdir)/build-aux/Makefile.tail.mk
