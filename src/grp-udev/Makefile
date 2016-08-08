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

INSTALL_DIRS += \
	$(sysconfdir)/udev/rules.d

dist_udevrules_DATA += \
	rules/50-udev-default.rules \
	rules/60-block.rules \
	rules/60-drm.rules \
	rules/60-evdev.rules \
	rules/60-persistent-storage-tape.rules \
	rules/60-persistent-input.rules \
	rules/60-persistent-alsa.rules \
	rules/60-persistent-storage.rules \
	rules/60-serial.rules \
	rules/64-btrfs.rules \
	rules/70-mouse.rules \
	rules/75-net-description.rules \
	rules/78-sound-card.rules \
	rules/80-net-setup-link.rules

nodist_udevrules_DATA += \
	rules/99-systemd.rules

udevconfdir = $(sysconfdir)/udev
dist_udevconf_DATA = \
	src/udev/udev.conf

pkgconfigdata_DATA += \
	src/udev/udev.pc

EXTRA_DIST += \
	rules/99-systemd.rules.in \
	src/udev/udev.pc.in

EXTRA_DIST += \
	units/systemd-udevd.service.in \
	units/systemd-udev-trigger.service.in \
	units/systemd-udev-settle.service.in

SOCKETS_TARGET_WANTS += \
	systemd-udevd-control.socket \
	systemd-udevd-kernel.socket

SYSINIT_TARGET_WANTS += \
	systemd-udevd.service \
	systemd-udev-trigger.service

nested.subdirs += systemd-udevd udevadm
nested.subdirs += ata_id cdrom_id collect mtd_probe scsi_id v4l_id

include $(topsrcdir)/build-aux/Makefile.tail.mk
