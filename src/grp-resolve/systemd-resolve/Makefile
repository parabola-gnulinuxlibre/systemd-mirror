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

systemd_resolve_SOURCES = \
	src/resolve/resolve-tool.c \
	$(basic_dns_sources) \
	src/shared/gcrypt-util.c \
	src/shared/gcrypt-util.h

nodist_systemd_resolve_SOURCES = \
	src/resolve/dns_type-from-name.h \
	src/resolve/dns_type-to-name.h

systemd_resolve_CFLAGS = \
	$(GCRYPT_CFLAGS)

systemd_resolve_LDADD = \
	libsystemd-shared.la \
	$(GCRYPT_LIBS) \
	-lm

bin_PROGRAMS += \
	systemd-resolve

dist_bashcompletion_data += \
	shell-completion/bash/systemd-resolve

dist_zshcompletion_data += \
	shell-completion/zsh/_systemd-resolve

include $(topsrcdir)/build-aux/Makefile.tail.mk
