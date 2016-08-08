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
include $(dir $(lastword $(MAKEFILE_LIST)))/config.mk
include $(topsrcdir)/build-aux/Makefile.head.mk

nested.subdirs += src

# intltoolize
std.gen_files += m4/intltool.m4
std.gen_files += po/Makefile.in.in
# autoreconf
std.gen_files += aclocal.m4
std.gen_files += automake.mk.in
std.gen_files += build-aux/compile
std.gen_files += build-aux/config.guess
std.gen_files += build-aux/config.sub
std.gen_files += build-aux/install-sh
std.gen_files += build-aux/ltmain.sh
std.gen_files += build-aux/missing
std.gen_files += m4/libtool.m4
std.gen_files += m4/ltoptions.m4
std.gen_files += m4/ltsugar.m4
std.gen_files += m4/ltversion.m4
std.gen_files += m4/lt~obsolete.m4
std.gen_files += config.h.in
std.gen_files += configure

include $(topsrcdir)/build-aux/Makefile.tail.mk
