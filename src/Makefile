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
at.subdirs += grp-hostname
at.subdirs += grp-initutils
at.subdirs += grp-journal
at.subdirs += grp-locale
at.subdirs += grp-login
#at.subdirs += grp-machine
at.subdirs += grp-network
at.subdirs += grp-resolve
at.subdirs += grp-system
at.subdirs += grp-timedate
at.subdirs += grp-udev
at.subdirs += grp-utils
at.subdirs += libbasic
at.subdirs += libfirewall
at.subdirs += libshared
at.subdirs += libsystemd
at.subdirs += libsystemd-network
at.subdirs += libudev
at.subdirs += nss-myhostname
at.subdirs += systemd-ask-password
at.subdirs += systemd-cgls
at.subdirs += systemd-cgroups-agent
at.subdirs += systemd-cgtop
at.subdirs += systemd-cryptsetup
at.subdirs += systemd-dbus1-generator
at.subdirs += systemd-debug-generator
at.subdirs += systemd-getty-generator
at.subdirs += systemd-gpt-auto-generator
at.subdirs += systemd-hibernate-resume
at.subdirs += systemd-initctl
at.subdirs += systemd-machine-id-setup
at.subdirs += systemd-modules-load
at.subdirs += systemd-nspawn
at.subdirs += systemd-rc-local-generator
at.subdirs += systemd-remount-fs
at.subdirs += systemd-reply-password
at.subdirs += systemd-shutdown
at.subdirs += systemd-socket-proxyd
at.subdirs += systemd-stdio-bridge
at.subdirs += systemd-system-update-generator
at.subdirs += systemd-timesyncd
at.subdirs += systemd-tty-ask-password-agent

include $(topsrcdir)/build-aux/Makefile.tail.mk
