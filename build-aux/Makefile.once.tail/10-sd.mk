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

_sd.autogen_files = aclocal.m4 automake.mk.in config.h.in configure po/Makefile.in.in
# `$*`/`%` had better be $(topsrcdir), but we can't enforce that
$(addprefix %/,$(_sd.autogen_files)): %/configure.ac %/autogen.sh
	cd $(topsrcdir) && ./autogen.sh

config_files = config.mk automake.mk autoconf.mk gnustandards.mk po/Makefile.in
config_headers = config.h
config_commands = depfiles libtool po/stamp-it
$(topoutdir)/config.status: $(topsrcdir)/configure
	cd $(topoutdir) && ./config.status --recheck
$(addprefix $(topoutdir)/,$(config_files)): $(topoutdir)/%: $(topoutdir)/config.status $(topsrcdir)/%.in
	cd $(topoutdir) && ./config.status --file=$*
$(addprefix $(topoutdir)/,$(config_headers)): $(topoutdir)/%: $(topoutdir)/%.stamp
$(foreach f,$(config_headers),$(topoutdir)/$f.stamp): $(topoutdir)/%.stamp: $(topoutdir)/config.status $(topsrcdir)/%.in
	cd $(topoutdir) && ./config.status --header=$*
	test -f $(topoutdir)/$*
	touch $@

# Let's run all tests of the test suite, but under valgrind. Let's
# exclude perl/python/shell scripts we have in there
.PHONY: valgrind-tests
valgrind-tests: $(TESTS)
	$(AM_V_GEN)for f in $(filter-out %.pl %.py, $^); do \
		if $(LIBTOOL) --mode=execute file $$f | grep -q shell; then \
		echo -e "$${x}Skipping non-binary $$f"; else \
		echo -e "$${x}Running $$f"; \
		$(LIBTOOL) --mode=execute valgrind -q --leak-check=full --max-stackframe=5242880 --error-exitcode=55 $(builddir)/$$f ; fi; \
		x="\n\n"; \
	done

# exported-%: %
# 	$(AM_V_GEN)$(NM) -g --defined-only $(builddir)/.libs/$(<:.la=.so) 2>&1 /dev/null | grep " T " | cut -d" " -f3 > $@

# exported: $(addprefix exported-, $(lib_LTLIBRARIES))
# 	$(AM_V_GEN)sort -u $^ > $@

# .PHONY: check-api-docs
# check-api-docs: exported man
# 	$(AM_V_GEN)for symbol in `cat exported` ; do \
# 		if test -f $(builddir)/man/$$symbol.html ; then \
# 			echo "  Symbol $$symbol() is documented." ; \
# 		else \
# 			echo "‣ Symbol $$symbol() lacks documentation." ; \
# 		fi ; \
# 	done

OBJECT_VARIABLES:=$(filter %_OBJECTS,$(.VARIABLES))
ALL_OBJECTS:=$(foreach v,$(OBJECT_VARIABLES),$($(v)))

undefined defined: $(ALL_OBJECTS)
	$(AM_V_GEN)for f in $(ALL_OBJECTS) ; do \
		$(NM) -g --$@-only `echo $(builddir)/"$$f" | sed -e 's,\([^/]*\).lo$$,.libs/\1.o,'` ; \
	done | cut -c 20- | cut -d @ -f 1 | sort -u > $@

CLEANFILES += \
	defined \
	undefined

.PHONY: check-api-unused
check-api-unused: defined undefined exported
	( cat exported undefined ) | sort -u  | diff -u - defined | grep ^+ | grep -v ^+++ | cut -c2-

.PHONY: check-includes
check-includes: $(top_srcdir)/tools/check-includes.pl
	$(AM_V_GEN) find * -name '*.[hcS]' -type f -print | sort -u \
		| xargs $(top_srcdir)/tools/check-includes.pl

EXTRA_DIST += \
	$(top_srcdir)/tools/check-includes.pl

.PHONY: cppcheck
cppcheck:
	cppcheck --enable=all -q $(top_srcdir)

# Used to extract compile flags for YCM.
print-%:
	@echo $($*)

git-contrib:
	@git shortlog -s `git describe --abbrev=0`.. | cut -c8- | sed 's/ / /g' | awk '{ print $$0 "," }' | sort -u

EXTRA_DIST += \
        tools/gdb-sd_dump_hashmaps.py

list-keys:
	gpg --verbose --no-options --no-default-keyring --no-auto-key-locate --batch --trust-model=always --keyring=$(srcdir)/src/import/import-pubring.gpg --list-keys

add-key:
	gpg --verbose --no-options --no-default-keyring --no-auto-key-locate --batch --trust-model=always --keyring=$(srcdir)/src/import/import-pubring.gpg --import -
