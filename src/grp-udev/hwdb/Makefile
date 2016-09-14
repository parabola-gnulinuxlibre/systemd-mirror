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

.PHONY: hwdb-update
hwdb-update:
	( cd $(top_srcdir)/hwdb && \
	wget -O usb.ids 'http://www.linux-usb.org/usb.ids' && \
	wget -O pci.ids 'http://pci-ids.ucw.cz/v2.2/pci.ids' && \
	wget -O ma-large.txt 'http://standards.ieee.org/develop/regauth/oui/oui.txt' && \
	wget -O ma-medium.txt 'http://standards.ieee.org/develop/regauth/oui28/mam.txt' && \
	wget -O ma-small.txt 'http://standards.ieee.org/develop/regauth/oui36/oui36.txt' && \
	wget -O pnp_id_registry.html 'http://www.uefi.org/uefi-pnp-export' && \
	wget -O acpi_id_registry.html 'http://www.uefi.org/uefi-acpi-export' && \
	./ids-update.pl && \
	./acpi-update.py > 20-acpi-vendor.hwdb.base && \
	patch -p0 -o- 20-acpi-vendor.hwdb.base < 20-acpi-vendor.hwdb.patch > 20-acpi-vendor.hwdb )

include $(topsrcdir)/build-aux/Makefile.tail.mk
