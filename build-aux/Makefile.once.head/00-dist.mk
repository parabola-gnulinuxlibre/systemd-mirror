# Copyright (C) 2015-2016  Luke Shumaker
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Developer configuration

dist.exts ?= .tar.gz
dist.pkgname ?= $(PACKAGE)
dist.version ?= $(VERSION)

ifeq ($(dist.pkgname),)
$(error dist.pkgname must be set)
endif
ifeq ($(dist.version),)
$(error dist.version must be set)
endif

# User configuration

CP      ?= cp
GZIP    ?= gzip
MKDIR   ?= mkdir
MKDIR_P ?= mkdir -p
MV      ?= mv
RM      ?= rm -f
TAR     ?= tar

GZIPFLAGS ?= $(GZIP_ENV)
GZIP_ENV ?= --best

# Implementation

at.phony += dist
