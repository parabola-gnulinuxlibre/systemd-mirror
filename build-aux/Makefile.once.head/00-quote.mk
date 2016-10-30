# Copyright (C) 2016  Luke Shumaker
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

mod.quote.description = Macros to quote tricky strings
define mod.quote.doc
# Inputs:
#   (none)
# Outputs:
#   - Global variable: `quote.var`       : GNU Make variables
#   - Global variable: `quote.pattern`   : GNU Make patterns
#   - Global variable: `quote.ere`       : POSIX Extended Regular Expressions
#   - Global variable: `quote.bre`       : POSIX Basic Regular Expressions
#   - Global variable: `quote.shell`     : POSIX sh(1) strings
#   - Global variable: `quote.shell-each`: POSIX sh(1) strings
#
# Escaping/quoting things is hard!  This module provides a number of
# functions to escape/quote strings for various contexts.
#
# `quote.shell-each` quotes each list-item separately (munging
# whitespace), while `quote.shell` keeps them as one string (preserving
# whitespace).
endef
mod.quote.doc := $(value mod.quote.doc)

_quote.backslash = $(if $1,$(call _quote.backslash,$(wordlist 2,$(words $1),$1),$(subst $(firstword $1),\$(firstword $1),$2)),$2)

quote.var     = $(subst $(at.nl),\$(at.nl),$(subst $$,$$$$,$1))
quote.pattern = $(call _quote.backslash, \ % ,$1)
quote.ere     = $(call _quote.backslash, \ ^ . [ $$ ( ) | * + ? { ,$1)
quote.bre     = $(call _quote.backslash, \ ^ . [ $$       *       ,$1)

quote.shell-each = $(foreach _quote.tmp,$1,$(call quote.shell,$(_quote.tmp)))

# I put this as the last line in the file because it confuses Emacs
# syntax highlighting and makes the remainder of the file difficult to
# edit.
quote.shell = $(subst $(at.nl),'$$'\n'','$(subst ','\'',$1)')
