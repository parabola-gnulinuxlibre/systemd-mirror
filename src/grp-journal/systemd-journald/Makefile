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

systemd_journald_SOURCES = \
	src/journal/journald.c \
	src/journal/journald-server.h

systemd_journald_LDADD = \
	libjournal-core.la \
	libsystemd-shared.la

rootlibexec_PROGRAMS += \
	systemd-journald

dist_systemunit_DATA += \
	units/systemd-journald.socket \
	units/systemd-journald-dev-log.socket \
	units/systemd-journald-audit.socket

nodist_systemunit_DATA += \
	units/systemd-journald.service

dist_pkgsysconf_DATA += \
	src/journal/journald.conf

nodist_catalog_DATA = \
	catalog/systemd.bg.catalog \
	catalog/systemd.be.catalog \
	catalog/systemd.be@latin.catalog \
	catalog/systemd.fr.catalog \
	catalog/systemd.it.catalog \
	catalog/systemd.pl.catalog \
	catalog/systemd.pt_BR.catalog \
	catalog/systemd.ru.catalog \
	catalog/systemd.zh_CN.catalog \
	catalog/systemd.zh_TW.catalog \
	catalog/systemd.catalog

EXTRA_DIST += \
	$(nodist_catalog_DATA:.catalog=.catalog.in)

# Note that we don't use @@ for replacement markers here, but %%. This is
# because the catalog uses @@ already for its runtime replacement handling and
# we don't want to conflict with that.
$(outdir)/%.catalog: catalog/%.catalog.in
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && \
	$(SED) -e 's~%SUPPORT_URL%~$(SUPPORT_URL)~' < $< > $@

SOCKETS_TARGET_WANTS += \
	systemd-journald.socket \
	systemd-journald-dev-log.socket \
	systemd-journald-audit.socket

SYSINIT_TARGET_WANTS += \
	systemd-journald.service

EXTRA_DIST += \
	units/systemd-journald.service.in

gperf_gperf_sources += \
	src/journal/journald-gperf.gperf

include $(topsrcdir)/build-aux/Makefile.tail.mk
