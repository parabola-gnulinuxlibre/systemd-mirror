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
include $(dir $(lastword $(MAKEFILE_LIST)))/../config.mk
include $(topsrcdir)/build-aux/Makefile.head.mk

at.subdirs += busctl
at.subdirs += grp-boot
at.subdirs += grp-coredump
#at.subdirs += grp-hostname
#at.subdirs += grp-import
#at.subdirs += grp-journal
#at.subdirs += grp-journal-remote
#at.subdirs += grp-locale
#at.subdirs += grp-login
#at.subdirs += grp-machine
#at.subdirs += grp-network
at.subdirs += grp-resolve
#at.subdirs += grp-system
at.subdirs += grp-timedate
at.subdirs += kernel-install
at.subdirs += libbasic
at.subdirs += libcore
at.subdirs += libfirewall
at.subdirs += libshared
at.subdirs += libsystemd
at.subdirs += libsystemd-network
at.subdirs += libudev
at.subdirs += nss-myhostname
at.subdirs += systemd-activate
#at.subdirs += systemd-ac-power
#at.subdirs += systemd-analyze

#at.subdirs += systemd-machine-id-setup
#at.subdirs += systemd-modules-load
#at.subdirs += systemd-sleep
#at.subdirs += systemd-socket-proxyd
#at.subdirs += systemd-sysctl

at.subdirs += systemd-nspawn
at.subdirs += systemd-initctl

include $(topsrcdir)/build-aux/Makefile.tail.mk
