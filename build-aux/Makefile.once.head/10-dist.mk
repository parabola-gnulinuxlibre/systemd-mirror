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

mod.dist.description = `dist` target to create distribution tarballs
define mod.dist.doc
# User variables:
#   - `CP      ?= cp`
#   - `GZIP    ?= gzip`
#   - `MKDIR   ?= mkdir`
#   - `MKDIR_P ?= mkdir -p`
#   - `MV      ?= mv`
#   - `RM      ?= rm -f`
#   - `TAR     ?= tar`
#   - `GZIPFLAGS ?= $(GZIP_ENV)`
#   - `GZIP_ENV ?= --best` (only used via `GZIPFLAGS`, not directly)
# Inputs:
#   - Global variable    : `dist.exts`    (Default: `.tar.gz`)
#   - Global variable    : `dist.pkgname` (Default: first of PACKAGE_TARNAME PACKAGE PACKAGE_NAME)
#   - Global variable    : `dist.version` (Default: first of PACKAGE_VERSION VERSION)
#   - Directory variable : `files.src`
# Outputs:
#   - Directory variable : `files.out.int` (only in top dir)
#   - .PHONY Target      : `$(outdir)/dist`
#   - Target             : `$(topoutdir)/$(dist.pkgname)-$(dist.version)`
#   - Target             : `$(topoutdir)/$(dist.pkgname)-$(dist.version).tar`
#   - Target             : `$(topoutdir)/$(dist.pkgname)-$(dist.version).tar.gz`
#
# Provide the standard `dist` .PHONY target, based on the `files` module
# information.
#
# You may change the default compression target easily via the
# `dist.exts` variable, but you must define the rule for it manually.
#
# Bugs:
#
#   The tarball isn't reproducible.  It uses file-system ordering of
#   files, and includes timestamps.
endef
mod.dist.doc := $(value mod.dist.doc)

# Developer configuration

dist.exts ?= .tar.gz
dist.pkgname ?= $(firstword $(PACKAGE_TARNAME) $(PACKAGE) $(PACKAGE_NAME))
dist.version ?= $(firstword $(PACKAGE_VERSION) $(VERSION))

ifeq ($(dist.pkgname),)
$(error Autothing module: dist: dist.pkgname must be set)
endif
ifeq ($(dist.version),)
$(error Autothing module: dist: dist.version must be set)
endif

_dist.files =

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
