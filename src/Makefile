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

nested.subdirs += busctl
nested.subdirs += grp-boot
nested.subdirs += grp-coredump
nested.subdirs += grp-hostname
nested.subdirs += grp-initprogs
nested.subdirs += grp-journal
nested.subdirs += grp-locale
nested.subdirs += grp-login
nested.subdirs += grp-machine
nested.subdirs += grp-network
nested.subdirs += grp-resolve
nested.subdirs += grp-system
nested.subdirs += grp-timedate
nested.subdirs += grp-udev
nested.subdirs += grp-utils
nested.subdirs += libsystemd
nested.subdirs += libsystemd-basic
nested.subdirs += libsystemd-firewall
nested.subdirs += libsystemd-gcrypt
nested.subdirs += libsystemd-network
nested.subdirs += libsystemd-shared
nested.subdirs += libudev
nested.subdirs += manpages
nested.subdirs += nss-myhostname
nested.subdirs += nss-systemd
nested.subdirs += systemd-ask-password
nested.subdirs += systemd-cgls
nested.subdirs += systemd-cgtop
nested.subdirs += systemd-cryptsetup
nested.subdirs += systemd-debug-generator
nested.subdirs += systemd-getty-generator
nested.subdirs += systemd-gpt-auto-generator
nested.subdirs += systemd-initctl
nested.subdirs += systemd-machine-id-setup
nested.subdirs += systemd-mount
nested.subdirs += systemd-nspawn
nested.subdirs += systemd-rc-local-generator
nested.subdirs += systemd-remount-fs
nested.subdirs += systemd-reply-password
nested.subdirs += systemd-socket-proxyd
nested.subdirs += systemd-stdio-bridge
nested.subdirs += systemd-system-update-generator
nested.subdirs += systemd-timesyncd
nested.subdirs += systemd-tty-ask-password-agent
nested.subdirs += test

include $(topsrcdir)/build-aux/Makefile.tail.mk
