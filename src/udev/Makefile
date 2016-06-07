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

dist_network_DATA = \
	network/99-default.link \
	network/80-container-host0.network \
	network/80-container-ve.network

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

bin_PROGRAMS += \
	udevadm

libexec_PROGRAMS += \
	systemd-udevd

noinst_LTLIBRARIES += \
	libudev-core.la

$(outdir)/keyboard-keys-list.txt:
	$(AM_V_at)$(MKDIR_P) $(dir $@)
	$(AM_V_GEN)$(CPP) $(CFLAGS) $(AM_CPPFLAGS) $(CPPFLAGS) -dM -include linux/input.h - < /dev/null | $(AWK) '/^#define[ \t]+KEY_[^ ]+[ \t]+[0-9K]/ { if ($$2 != "KEY_MAX") { print $$2 } }' > $@

$(outdir)/keyboard-keys-from-name.gperf: src/udev/keyboard-keys-list.txt
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "struct key { const char* name; unsigned short id; };"; print "%null-strings"; print "%%";} { print tolower(substr($$1 ,5)) ", " $$1 }' < $< > $@

$(outdir)/keyboard-keys-from-name.h: src/udev/keyboard-keys-from-name.gperf
	$(AM_V_GPERF)$(GPERF) -L ANSI-C -t -N keyboard_lookup_key -H hash_key_name -p -C < $< > $@

gperf_txt_sources += \
	src/udev/keyboard-keys-list.txt

libudev_core_la_SOURCES = \
	src/udev/udev.h \
	src/udev/udev-event.c \
	src/udev/udev-watch.c \
	src/udev/udev-node.c \
	src/udev/udev-rules.c \
	src/udev/udev-ctrl.c \
	src/udev/udev-builtin.c \
	src/udev/udev-builtin-btrfs.c \
	src/udev/udev-builtin-hwdb.c \
	src/udev/udev-builtin-input_id.c \
	src/udev/udev-builtin-keyboard.c \
	src/udev/udev-builtin-net_id.c \
	src/udev/udev-builtin-net_setup_link.c \
	src/udev/udev-builtin-path_id.c \
	src/udev/udev-builtin-usb_id.c \
	src/udev/net/link-config.h \
	src/udev/net/link-config.c \
	src/udev/net/ethtool-util.h \
	src/udev/net/ethtool-util.c

nodist_libudev_core_la_SOURCES = \
	src/udev/keyboard-keys-from-name.h \
	src/udev/net/link-config-gperf.c

gperf_gperf_sources += \
	src/udev/net/link-config-gperf.gperf

libudev_core_la_CFLAGS = \
	$(AM_CFLAGS) \
	$(BLKID_CFLAGS) \
	$(KMOD_CFLAGS)

libudev_core_la_LIBADD = \
	libsystemd-network.la \
	libshared.la \
	$(BLKID_LIBS) \
	$(KMOD_LIBS)

ifneq ($(HAVE_KMOD),)
libudev_core_la_SOURCES += \
	src/udev/udev-builtin-kmod.c

dist_udevrules_DATA += \
	rules/80-drivers.rules
endif # HAVE_KMOD

ifneq ($(HAVE_BLKID),)
libudev_core_la_SOURCES += \
	src/udev/udev-builtin-blkid.c
endif # HAVE_BLKID

ifneq ($(HAVE_ACL),)
libudev_core_la_SOURCES += \
	src/udev/udev-builtin-uaccess.c \
	src/login/logind-acl.c \
	src/libsystemd/sd-login/sd-login.c \
	src/systemd/sd-login.h
endif # HAVE_ACL

systemd_udevd_SOURCES = \
	src/udev/udevd.c

systemd_udevd_LDADD = \
	libudev-core.la

udevadm_SOURCES = \
	src/udev/udevadm.c \
	src/udev/udevadm-info.c \
	src/udev/udevadm-control.c \
	src/udev/udevadm-monitor.c \
	src/udev/udevadm-hwdb.c \
	src/udev/udevadm-settle.c \
	src/udev/udevadm-trigger.c \
	src/udev/udevadm-test.c \
	src/udev/udevadm-test-builtin.c \
	src/udev/udevadm-util.c \
	src/udev/udevadm-util.h

udevadm_LDADD = \
	libudev-core.la

include $(topsrcdir)/build-aux/Makefile.tail.mk
