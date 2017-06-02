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

EXTRA_DIST =
BUILT_SOURCES =
INSTALL_EXEC_HOOKS =
UNINSTALL_EXEC_HOOKS =
INSTALL_DATA_HOOKS =
UNINSTALL_DATA_HOOKS =
DISTCLEAN_LOCAL_HOOKS =
CLEAN_LOCAL_HOOKS =
pkginclude_HEADERS =
noinst_LTLIBRARIES =
lib_LTLIBRARIES =
rootlibexec_LTLIBRARIES =
include_HEADERS =
noinst_DATA =
pkgconfigdata_DATA =
pkgconfiglib_DATA =
polkitpolicy_in_in_files =
polkitpolicy_in_files =
polkitpolicy_files =
dist_udevrules_DATA =
nodist_udevrules_DATA =
dist_pkgsysconf_DATA =
nodist_pkgsysconf_DATA =
dist_dbuspolicy_DATA =
dist_dbussystemservice_DATA =
dist_systemunit_DATA_busnames =
dist_sysusers_DATA =
check_PROGRAMS =
check_DATA =
dist_rootlibexec_DATA =
tests=
manual_tests =
TEST_EXTENSIONS = .py
PY_LOG_COMPILER = $(PYTHON)
DISABLE_HARD_ERRORS = yes
ifneq ($(ENABLE_TESTS),)
noinst_PROGRAMS = $(manual_tests) $(tests) $(unsafe_tests)
TESTS = $(tests)
ifneq ($(ENABLE_UNSAFE_TESTS),)
TESTS += \
	$(unsafe_tests)
endif # ENABLE_UNSAFE_TESTS
else # ENABLE_TESTS
noinst_PROGRAMS =
TESTS =
endif # ENABLE_TESTS
AM_TESTS_ENVIRONMENT = \
	export SYSTEMD_KBD_MODEL_MAP=$(abs_top_srcdir)/src/locale/kbd-model-map; \
	export SYSTEMD_LANGUAGE_FALLBACK_MAP=$(abs_top_srcdir)/src/locale/language-fallback-map;

ifneq ($(ENABLE_BASH_COMPLETION),)
dist_bashcompletion_DATA = $(dist_bashcompletion_data)
nodist_bashcompletion_DATA = $(nodist_bashcompletion_data)
endif # ENABLE_BASH_COMPLETION
ifneq ($(ENABLE_ZSH_COMPLETION),)
dist_zshcompletion_DATA = $(dist_zshcompletion_data)
nodist_zshcompletion_DATA = $(nodist_zshcompletion_data)
endif # ENABLE_ZSH_COMPLETION
udevlibexec_PROGRAMS =
gperf_gperf_sources =
rootlib_LTLIBRARIES =

in_files = $(filter %.in,$(EXTRA_DIST))
in_in_files = $(filter %.in.in, $(in_files))
m4_files = $(filter %.m4,$(EXTRA_DIST) $(in_files:.m4.in=.m4))

CLEANFILES = $(BUILT_SOURCES) \
	$(pkgconfigdata_DATA) \
	$(pkgconfiglib_DATA) \
	$(nodist_bashcompletion_data) \
	$(nodist_zshcompletion_data) \
	$(in_files:.in=) $(in_in_files:.in.in=) \
	$(m4_files:.m4=)

.PHONY: $(INSTALL_EXEC_HOOKS) $(UNINSTALL_EXEC_HOOKS) \
	$(INSTALL_DATA_HOOKS) $(UNINSTALL_DATA_HOOKS) \
	$(DISTCLEAN_LOCAL_HOOKS) $(CLEAN_LOCAL_HOOKS)

AM_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-DPKGSYSCONFDIR=\"$(pkgsysconfdir)\" \
	-DSYSTEM_CONFIG_UNIT_PATH=\"$(pkgsysconfdir)/system\" \
	-DSYSTEM_DATA_UNIT_PATH=\"$(systemunitdir)\" \
	-DSYSTEM_SYSVINIT_PATH=\"$(SYSTEM_SYSVINIT_PATH)\" \
	-DSYSTEM_SYSVRCND_PATH=\"$(SYSTEM_SYSVRCND_PATH)\" \
	-DUSER_CONFIG_UNIT_PATH=\"$(pkgsysconfdir)/user\" \
	-DUSER_DATA_UNIT_PATH=\"$(userunitdir)\" \
	-DCERTIFICATE_ROOT=\"$(CERTIFICATEROOT)\" \
	-DCATALOG_DATABASE=\"$(catalogstatedir)/database\" \
	-DSYSTEMD_CGROUP_AGENT_PATH=\"$(rootlibexecdir)/systemd-cgroups-agent\" \
	-DSYSTEMD_BINARY_PATH=\"$(rootlibexecdir)/systemd\" \
	-DSYSTEMD_FSCK_PATH=\"$(rootlibexecdir)/systemd-fsck\" \
	-DSYSTEMD_SHUTDOWN_BINARY_PATH=\"$(rootlibexecdir)/systemd-shutdown\" \
	-DSYSTEMD_SLEEP_BINARY_PATH=\"$(rootlibexecdir)/systemd-sleep\" \
	-DSYSTEMCTL_BINARY_PATH=\"$(rootbindir)/systemctl\" \
	-DSYSTEMD_TTY_ASK_PASSWORD_AGENT_BINARY_PATH=\"$(rootbindir)/systemd-tty-ask-password-agent\" \
	-DSYSTEMD_STDIO_BRIDGE_BINARY_PATH=\"$(bindir)/systemd-stdio-bridge\" \
	-DROOTPREFIX=\"$(rootprefix)\" \
	-DRANDOM_SEED_DIR=\"$(localstatedir)/lib/systemd/\" \
	-DRANDOM_SEED=\"$(localstatedir)/lib/systemd/random-seed\" \
	-DSYSTEMD_CRYPTSETUP_PATH=\"$(rootlibexecdir)/systemd-cryptsetup\" \
	-DSYSTEM_GENERATOR_PATH=\"$(systemgeneratordir)\" \
	-DUSER_GENERATOR_PATH=\"$(usergeneratordir)\" \
	-DSYSTEM_SHUTDOWN_PATH=\"$(systemshutdowndir)\" \
	-DSYSTEM_SLEEP_PATH=\"$(systemsleepdir)\" \
	-DSYSTEMD_KBD_MODEL_MAP=\"$(pkgdatadir)/kbd-model-map\" \
	-DSYSTEMD_LANGUAGE_FALLBACK_MAP=\"$(pkgdatadir)/language-fallback-map\" \
	-DUDEVLIBEXECDIR=\"$(udevlibexecdir)\" \
	-DPOLKIT_AGENT_BINARY_PATH=\"$(bindir)/pkttyagent\" \
	-DQUOTACHECK=\"$(QUOTACHECK)\" \
	-DKEXEC=\"$(KEXEC)\" \
	-DMOUNT_PATH=\"$(MOUNT_PATH)\" \
	-DUMOUNT_PATH=\"$(UMOUNT_PATH)\" \
	-DLIBDIR=\"$(libdir)\" \
	-DROOTLIBDIR=\"$(rootlibdir)\" \
	-DROOTLIBEXECDIR=\"$(rootlibexecdir)\" \
	-DTEST_DIR=\"$(abs_top_srcdir)/test\" \
	-I $(top_srcdir)/src \
	-I $(top_builddir)/src/basic \
	-I $(top_srcdir)/src/basic \
	-I $(top_srcdir)/src/shared \
	-I $(top_builddir)/src/shared \
	-I $(top_srcdir)/src/network \
	-I $(top_srcdir)/src/locale \
	-I $(top_srcdir)/src/login \
	-I $(top_srcdir)/src/journal \
	-I $(top_builddir)/src/journal \
	-I $(top_srcdir)/src/timedate \
	-I $(top_srcdir)/src/timesync \
	-I $(top_srcdir)/src/nspawn \
	-I $(top_srcdir)/src/resolve \
	-I $(top_builddir)/src/resolve \
	-I $(top_srcdir)/src/systemd \
	-I $(top_builddir)/src/core \
	-I $(top_srcdir)/src/core \
	-I $(top_srcdir)/src/libudev \
	-I $(top_srcdir)/src/udev \
	-I $(top_srcdir)/src/udev/net \
	-I $(top_builddir)/src/udev \
	-I $(top_srcdir)/src/libsystemd/sd-bus \
	-I $(top_srcdir)/src/libsystemd/sd-event \
	-I $(top_srcdir)/src/libsystemd/sd-login \
	-I $(top_srcdir)/src/libsystemd/sd-netlink \
	-I $(top_srcdir)/src/libsystemd/sd-network \
	-I $(top_srcdir)/src/libsystemd/sd-hwdb \
	-I $(top_srcdir)/src/libsystemd/sd-device \
	-I $(top_srcdir)/src/libsystemd/sd-id128 \
	-I $(top_srcdir)/src/libsystemd-network \
	$(OUR_CPPFLAGS)

AM_CFLAGS = $(OUR_CFLAGS)
AM_LDFLAGS = $(OUR_LDFLAGS)

# ------------------------------------------------------------------------------
INSTALL_DIRS =

SHUTDOWN_TARGET_WANTS =
LOCAL_FS_TARGET_WANTS =
MULTI_USER_TARGET_WANTS =
GRAPHICAL_TARGET_WANTS =
RESCUE_TARGET_WANTS =
SYSINIT_TARGET_WANTS =
SOCKETS_TARGET_WANTS =
BUSNAMES_TARGET_WANTS =
TIMERS_TARGET_WANTS =
USER_SOCKETS_TARGET_WANTS =
USER_DEFAULT_TARGET_WANTS =
USER_BUSNAMES_TARGET_WANTS =

SYSTEM_UNIT_ALIASES =
USER_UNIT_ALIASES =
GENERAL_ALIASES =

install-target-wants-hook:
	what="$(SHUTDOWN_TARGET_WANTS)" && wants=shutdown.target && dir=$(systemunitdir) && $(add-wants)
	what="$(LOCAL_FS_TARGET_WANTS)" && wants=local-fs.target && dir=$(systemunitdir) && $(add-wants)
	what="$(MULTI_USER_TARGET_WANTS)" && wants=multi-user.target && dir=$(systemunitdir) && $(add-wants)
	what="$(GRAPHICAL_TARGET_WANTS)" && wants=graphical.target && dir=$(systemunitdir) && $(add-wants)
	what="$(RESCUE_TARGET_WANTS)" && wants=rescue.target && dir=$(systemunitdir) && $(add-wants)
	what="$(SYSINIT_TARGET_WANTS)" && wants=sysinit.target && dir=$(systemunitdir) && $(add-wants)
	what="$(SOCKETS_TARGET_WANTS)" && wants=sockets.target && dir=$(systemunitdir) && $(add-wants)
	what="$(TIMERS_TARGET_WANTS)" && wants=timers.target && dir=$(systemunitdir) && $(add-wants)
	what="$(SLICES_TARGET_WANTS)" && wants=slices.target && dir=$(systemunitdir) && $(add-wants)
	what="$(USER_SOCKETS_TARGET_WANTS)" && wants=sockets.target && dir=$(userunitdir) && $(add-wants)
	what="$(USER_DEFAULT_TARGET_WANTS)" && wants=default.target && dir=$(userunitdir) && $(add-wants)

install-busnames-target-wants-hook:
	what="$(BUSNAMES_TARGET_WANTS)" && wants=busnames.target && dir=$(systemunitdir) && $(add-wants)
	what="$(USER_BUSNAMES_TARGET_WANTS)" && wants=busnames.target && dir=$(userunitdir) && $(add-wants)

define add-wants
	[ -z "$$what" ] || ( \
	  dir=$(DESTDIR)$$dir/$$wants.wants && \
	  $(MKDIR_P) -m 0755 $$dir && \
	  cd $$dir && \
	  rm -f $$what && \
	  for i in $$what; do $(LN_S) ../$$i . || exit $$? ; done )
endef

install-directories-hook:
	$(MKDIR_P) $(addprefix $(DESTDIR),$(INSTALL_DIRS))

install-aliases-hook:
	set -- $(SYSTEM_UNIT_ALIASES) && \
		dir=$(systemunitdir) && $(install-aliases)
	set -- $(USER_UNIT_ALIASES) && \
		dir=$(userunitdir) && $(install-relative-aliases)
	set -- $(GENERAL_ALIASES) && \
		dir= && $(install-relative-aliases)

define install-aliases
	while [ -n "$$1" ]; do \
		$(MKDIR_P) `dirname $(DESTDIR)$$dir/$$2` && \
		rm -f $(DESTDIR)$$dir/$$2 && \
		$(LN_S) $$1 $(DESTDIR)$$dir/$$2 && \
		shift 2 || exit $$?; \
	done
endef

define install-relative-aliases
	while [ -n "$$1" ]; do \
		$(MKDIR_P) `dirname $(DESTDIR)$$dir/$$2` && \
		rm -f $(DESTDIR)$$dir/$$2 && \
		$(LN_S) --relative $(DESTDIR)$$1 $(DESTDIR)$$dir/$$2 && \
		shift 2 || exit $$?; \
	done
endef

install-touch-usr-hook:
	touch -c $(DESTDIR)/$(prefix)

INSTALL_EXEC_HOOKS += \
	install-target-wants-hook \
	install-directories-hook \
	install-aliases-hook \
	install-touch-usr-hook

INSTALL_EXEC_HOOKS += \
	install-busnames-target-wants-hook

dist_bashcompletion_data = \
	shell-completion/bash/busctl \
	shell-completion/bash/journalctl \
	shell-completion/bash/systemd-analyze \
	shell-completion/bash/systemd-cat \
	shell-completion/bash/systemd-cgls \
	shell-completion/bash/systemd-cgtop \
	shell-completion/bash/systemd-delta \
	shell-completion/bash/systemd-detect-virt \
	shell-completion/bash/systemd-nspawn \
	shell-completion/bash/systemd-path \
	shell-completion/bash/systemd-run \
	shell-completion/bash/udevadm \
	shell-completion/bash/kernel-install

nodist_bashcompletion_data = \
	shell-completion/bash/systemctl

dist_zshcompletion_data = \
	shell-completion/zsh/_busctl \
	shell-completion/zsh/_journalctl \
	shell-completion/zsh/_udevadm \
	shell-completion/zsh/_kernel-install \
	shell-completion/zsh/_systemd-nspawn \
	shell-completion/zsh/_systemd-analyze \
	shell-completion/zsh/_systemd-run \
	shell-completion/zsh/_sd_hosts_or_user_at_host \
	shell-completion/zsh/_sd_outputmodes \
	shell-completion/zsh/_sd_unit_files \
	shell-completion/zsh/_systemd-delta \
	shell-completion/zsh/_systemd

nodist_zshcompletion_data = \
	shell-completion/zsh/_systemctl

EXTRA_DIST += \
	shell-completion/bash/systemctl.in \
	shell-completion/zsh/_systemctl.in

dist_sysctl_DATA = \
	sysctl.d/50-default.conf

dist_systemunit_DATA = \
	units/graphical.target \
	units/multi-user.target \
	units/emergency.target \
	units/sysinit.target \
	units/basic.target \
	units/getty.target \
	units/halt.target \
	units/kexec.target \
	units/exit.target \
	units/local-fs.target \
	units/local-fs-pre.target \
	units/initrd.target \
	units/initrd-fs.target \
	units/initrd-root-device.target \
	units/initrd-root-fs.target \
	units/remote-fs.target \
	units/remote-fs-pre.target \
	units/network.target \
	units/network-pre.target \
	units/network-online.target \
	units/nss-lookup.target \
	units/nss-user-lookup.target \
	units/poweroff.target \
	units/reboot.target \
	units/rescue.target \
	units/rpcbind.target \
	units/time-sync.target \
	units/shutdown.target \
	units/final.target \
	units/umount.target \
	units/sigpwr.target \
	units/sleep.target \
	units/sockets.target \
	units/timers.target \
	units/paths.target \
	units/suspend.target \
	units/swap.target \
	units/slices.target \
	units/system.slice \
	units/systemd-initctl.socket \
	units/syslog.socket \
	units/dev-hugepages.mount \
	units/dev-mqueue.mount \
	units/sys-kernel-config.mount \
	units/sys-kernel-debug.mount \
	units/sys-fs-fuse-connections.mount \
	units/tmp.mount \
	units/var-lib-machines.mount \
	units/printer.target \
	units/sound.target \
	units/bluetooth.target \
	units/smartcard.target \
	units/systemd-ask-password-wall.path \
	units/systemd-ask-password-console.path \
	units/systemd-udevd-control.socket \
	units/systemd-udevd-kernel.socket \
	units/system-update.target \
	units/initrd-switch-root.target \
	units/machines.target

dist_systemunit_DATA += \
	$(dist_systemunit_DATA_busnames)

dist_systemunit_DATA_busnames += \
	units/busnames.target

nodist_systemunit_DATA = \
	units/getty@.service \
	units/serial-getty@.service \
	units/console-getty.service \
	units/container-getty@.service \
	units/systemd-initctl.service \
	units/systemd-remount-fs.service \
	units/systemd-ask-password-wall.service \
	units/systemd-ask-password-console.service \
	units/systemd-sysctl.service \
	units/emergency.service \
	units/rescue.service \
	units/user@.service \
	units/systemd-suspend.service \
	units/systemd-halt.service \
	units/systemd-poweroff.service \
	units/systemd-reboot.service \
	units/systemd-kexec.service \
	units/systemd-exit.service \
	units/systemd-fsck@.service \
	units/systemd-fsck-root.service \
	units/systemd-machine-id-commit.service \
	units/systemd-udevd.service \
	units/systemd-udev-trigger.service \
	units/systemd-udev-settle.service \
	units/systemd-hwdb-update.service \
	units/debug-shell.service \
	units/initrd-parse-etc.service \
	units/initrd-cleanup.service \
	units/initrd-udevadm-cleanup-db.service \
	units/initrd-switch-root.service \
	units/systemd-nspawn@.service \
	units/systemd-update-done.service

ifneq ($(HAVE_UTMP),)
nodist_systemunit_DATA += \
	units/systemd-update-utmp.service \
	units/systemd-update-utmp-runlevel.service
endif # HAVE_UTMP

dist_userunit_DATA = \
	units/user/basic.target \
	units/user/default.target \
	units/user/exit.target \
	units/user/graphical-session.target \
	units/user/graphical-session-pre.target \
	units/user/bluetooth.target \
	units/user/busnames.target \
	units/user/paths.target \
	units/user/printer.target \
	units/user/shutdown.target \
	units/user/smartcard.target \
	units/user/sockets.target \
	units/user/sound.target \
	units/user/timers.target

nodist_userunit_DATA = \
	units/user/systemd-exit.service

dist_systempreset_DATA = \
	system-preset/90-systemd.preset

EXTRA_DIST += \
	units/getty@.service.m4 \
	units/serial-getty@.service.m4 \
	units/console-getty.service.m4.in \
	units/container-getty@.service.m4.in \
	units/rescue.service.in \
	units/systemd-initctl.service.in \
	units/systemd-remount-fs.service.in \
	units/systemd-update-utmp.service.in \
	units/systemd-update-utmp-runlevel.service.in \
	units/systemd-ask-password-wall.service.in \
	units/systemd-ask-password-console.service.in \
	units/systemd-sysctl.service.in \
	units/emergency.service.in \
	units/systemd-halt.service.in \
	units/systemd-poweroff.service.in \
	units/systemd-reboot.service.in \
	units/systemd-kexec.service.in \
	units/systemd-exit.service.in \
	units/user/systemd-exit.service.in \
	units/systemd-fsck@.service.in \
	units/systemd-fsck-root.service.in \
	units/systemd-machine-id-commit.service.in \
	units/user@.service.m4.in \
	units/debug-shell.service.in \
	units/systemd-suspend.service.in \
	units/quotaon.service.in \
	units/initrd-parse-etc.service.in \
	units/initrd-cleanup.service.in \
	units/initrd-udevadm-cleanup-db.service.in \
	units/initrd-switch-root.service.in \
	units/systemd-nspawn@.service.in \
	units/systemd-update-done.service.in \
    units/tmp.mount.m4

ifneq ($(HAVE_SYSV_COMPAT),)
nodist_systemunit_DATA += \
	units/rc-local.service \
	units/halt-local.service

systemgenerator_PROGRAMS += \
	systemd-sysv-generator \
	systemd-rc-local-generator
endif # HAVE_SYSV_COMPAT

EXTRA_DIST += \
	src/systemctl/systemd-sysv-install.SKELETON \
	units/rc-local.service.in \
	units/halt-local.service.in

GENERAL_ALIASES += \
	$(systemunitdir)/machines.target $(pkgsysconfdir)/system/multi-user.target.wants/machines.target

dist_doc_DATA = \
	README \
	NEWS \
	CODING_STYLE \
	LICENSE.LGPL2.1 \
	LICENSE.GPL2 \
	DISTRO_PORTING \
	src/libsystemd/sd-bus/PORTING-DBUS1 \
	src/libsystemd/sd-bus/DIFFERENCES \
	src/libsystemd/sd-bus/GVARIANT-SERIALIZATION

EXTRA_DIST += \
	README.md \
	autogen.sh \
	.dir-locals.el \
	.editorconfig \
	.vimrc \
	.ycm_extra_conf.py \
	.travis.yml \
	.mailmap

@INTLTOOL_POLICY_RULE@

ifneq ($(ENABLE_LDCONFIG),)
dist_systemunit_DATA += \
	units/ldconfig.service

SYSINIT_TARGET_WANTS += \
	ldconfig.service
endif # ENABLE_LDCONFIG

gperf_gperf_m4_sources = \
	src/core/load-fragment-gperf.gperf.m4

gperf_txt_sources = \
	src/basic/errno-list.txt \
	src/basic/af-list.txt \
	src/basic/arphrd-list.txt \
	src/basic/cap-list.txt

BUILT_SOURCES += \
	$(gperf_gperf_m4_sources:-gperf.gperf.m4=-gperf.c) \
	$(gperf_gperf_m4_sources:-gperf.gperf.m4=-gperf-nulstr.c) \
	$(gperf_gperf_sources:-gperf.gperf=-gperf.c) \
	$(gperf_txt_sources:-list.txt=-from-name.h) \
	$(filter-out %keyboard-keys-to-name.h,$(gperf_txt_sources:-list.txt=-to-name.h))

CLEANFILES += \
	$(gperf_txt_sources:-list.txt=-from-name.gperf)
DISTCLEANFILES = \
	$(gperf_txt_sources)

EXTRA_DIST += \
	$(gperf_gperf_m4_sources) \
	$(gperf_gperf_sources)

CLEANFILES += \
	$(gperf_txt_sources)

## .PHONY so it always rebuilds it
.PHONY: coverage lcov-run lcov-report coverage-sync

# run lcov from scratch, always
coverage: all
	$(MAKE) lcov-run
	$(MAKE) lcov-report

coverage_dir = coverage
coverage_opts = --base-directory $(srcdir) --directory $(builddir) --rc 'geninfo_adjust_src_path=$(abspath $(srcdir))=>$(abspath $(builddir))'

ifneq ($(ENABLE_COVERAGE),)
# reset run coverage tests
lcov-run:
	@rm -rf $(coverage_dir)
	lcov $(coverage_opts) --zerocounters
	-$(MAKE) check

# generate report based on current coverage data
lcov-report:
	$(MKDIR_P) $(coverage_dir)
	lcov $(coverage_opts) --compat-libtool --capture --no-external \
		| sed 's|$(abspath $(builddir))|$(abspath $(srcdir))|' > $(coverage_dir)/.lcov.info
	lcov --remove $(coverage_dir)/.lcov.info --output-file $(coverage_dir)/.lcov-clean.info 'test-*'
	genhtml -t "systemd test coverage" -o $(coverage_dir) $(coverage_dir)/.lcov-clean.info
	@echo "Coverage report generated in $(abs_builddir)/$(coverage_dir)/index.html"

# lcov doesn't work properly with vpath builds, make sure that bad
# output is not uploaded by mistake.
coverage-sync: coverage
	test "$(builddir)" = "$(srcdir)"
	rsync -rlv --delete --omit-dir-times coverage/ $(www_target)/coverage

else # ENABLE_COVERAGE
lcov-run lcov-report:
	echo "Need to reconfigure with --enable-coverage"
endif # ENABLE_COVERAGE

ifneq ($(HAVE_REMOTE),)
nodist_sysusers_DATA += \
	sysusers.d/systemd-remote.conf
endif # HAVE_REMOTE
dist_factory_etc_DATA = \
	factory/etc/nsswitch.conf

ifneq ($(HAVE_PAM),)
dist_factory_pam_DATA = \
	factory/etc/pam.d/system-auth \
	factory/etc/pam.d/other
endif # HAVE_PAM

ifneq ($(ENABLE_TESTS),)
TESTS += \
	test/udev-test.pl

ifneq ($(HAVE_PYTHON),)
TESTS += \
	test/rule-syntax-check.py \
	hwdb/parse_hwdb.py

ifneq ($(HAVE_SYSV_COMPAT),)
TESTS += \
	test/sysv-generator-test.py
endif # HAVE_SYSV_COMPAT
endif # HAVE_PYTHON
endif # ENABLE_TESTS

tests += \
	test-libudev

manual_tests += \
	test-udev

test_libudev_SOURCES = \
	src/test/test-libudev.c

test_libudev_LDADD = \
	libsystemd-shared.la

test_udev_SOURCES = \
	src/test/test-udev.c

test_udev_LDADD = \
	libudev-core.la  \
	libsystemd-shared.la \
	$(BLKID_LIBS) \
	$(KMOD_LIBS) \
	-lrt

ifneq ($(ENABLE_TESTS),)
check_DATA += \
	test/sys
endif # ENABLE_TESTS

# packed sysfs test tree
$(outdir)/sys: test/sys.tar.xz
	-rm -rf test/sys
	$(AM_V_GEN)tar -C test/ -xJf $(top_srcdir)/test/sys.tar.xz
	-touch test/sys

test-sys-distclean:
	-rm -rf test/sys
DISTCLEAN_LOCAL_HOOKS += test-sys-distclean

EXTRA_DIST += \
	test/sys.tar.xz \
	test/udev-test.pl \
	test/rule-syntax-check.py \
	test/sysv-generator-test.py \
	test/mocks/fsck \
	hwdb/parse_hwdb.py

test_nss_SOURCES = \
	src/test/test-nss.c

test_nss_LDADD = \
	libsystemd-internal.la \
	libsystemd-basic.la \
	-ldl

manual_tests += \
	test-nss

endif # HAVE_GCRYPT
endif # HAVE_BZIP2
endif # HAVE_ZLIB
endif # HAVE_XZ
endif # HAVE_LIBCURL

endif # ENABLE_IMPORTD
EXTRA_DIST += \
	test/Makefile \
	test/README.testsuite \
	test/TEST-01-BASIC \
	test/TEST-01-BASIC/Makefile \
	test/TEST-01-BASIC/test.sh \
	test/TEST-02-CRYPTSETUP \
	test/TEST-02-CRYPTSETUP/Makefile \
	test/TEST-02-CRYPTSETUP/test.sh \
	test/TEST-03-JOBS \
	test/TEST-03-JOBS/Makefile \
	test/TEST-03-JOBS/test-jobs.sh \
	test/TEST-03-JOBS/test.sh \
	test/TEST-04-JOURNAL/Makefile \
	test/TEST-04-JOURNAL/test-journal.sh \
	test/TEST-04-JOURNAL/test.sh \
	test/TEST-05-RLIMITS/Makefile \
	test/TEST-05-RLIMITS/test-rlimits.sh \
	test/TEST-05-RLIMITS/test.sh \
	test/TEST-06-SELINUX/Makefile \
	test/TEST-06-SELINUX/test-selinux-checks.sh \
	test/TEST-06-SELINUX/test.sh \
	test/TEST-06-SELINUX/systemd_test.te \
	test/TEST-06-SELINUX/systemd_test.if \
	test/TEST-07-ISSUE-1981/Makefile \
	test/TEST-07-ISSUE-1981/test-segfault.sh \
	test/TEST-07-ISSUE-1981/test.sh \
	test/TEST-08-ISSUE-2730/Makefile \
	test/TEST-08-ISSUE-2730/test.sh \
	test/TEST-09-ISSUE-2691/Makefile \
	test/TEST-09-ISSUE-2691/test.sh \
	test/TEST-10-ISSUE-2467/Makefile \
	test/TEST-10-ISSUE-2467/test.sh \
	test/TEST-11-ISSUE-3166/Makefile \
	test/TEST-11-ISSUE-3166/test.sh \
	test/TEST-12-ISSUE-3171/Makefile \
	test/TEST-12-ISSUE-3171/test.sh \
	test/TEST-13-NSPAWN-SMOKE/Makefile \
	test/TEST-13-NSPAWN-SMOKE/create-busybox-container \
	test/TEST-13-NSPAWN-SMOKE/test.sh \
	test/test-functions

EXTRA_DIST += \
	test/loopy2.service \
	test/loopy3.service \
	test/loopy4.service \
	test/loopy.service \
	test/loopy.service.d \
	test/loopy.service.d/compat.conf

$(outdir)/%: units/%.in
	$(SED_PROCESS)

$(outdir)/%: man/%.in
	$(SED_PROCESS)

%.pc: %.pc.in
	$(SED_PROCESS)

%.conf: %.conf.in
	$(SED_PROCESS)

$(outdir)/%.systemd: src/core/%.systemd.in
	$(SED_PROCESS)

$(outdir)/%.policy.in: src/%.policy.in.in
	$(SED_PROCESS)

$(outdir)/%: shell-completion/%.in
	$(SED_PROCESS)

%.rules: %.rules.in
	$(SED_PROCESS)

%.conf: %.conf.in
	$(SED_PROCESS)

$(outdir)/%: sysusers.d/%.m4 $(top_builddir)/config.status
	$(AM_V_M4)$(M4) -P $(M4_DEFINES) < $< > $@

$(outdir)/%: tmpfiles.d/%.m4 $(top_builddir)/config.status
	$(AM_V_M4)$(M4) -P $(M4_DEFINES) < $< > $@


$(outdir)/%: units/%.m4 $(top_builddir)/config.status
	$(AM_V_M4)$(M4) -P $(M4_DEFINES) -DFOR_SYSTEM=1 < $< > $@

$(outdir)/%: units/user/%.m4 $(top_builddir)/config.status
	$(AM_V_M4)$(M4) -P $(M4_DEFINES) -DFOR_USER=1 < $< > $@

ifneq ($(ENABLE_POLKIT),)
nodist_polkitpolicy_DATA = \
	$(polkitpolicy_files) \
	$(polkitpolicy_in_in_files:.policy.in.in=.policy)
endif # ENABLE_POLKIT

EXTRA_DIST += \
	$(polkitpolicy_in_files) \
	$(polkitpolicy_in_in_files)

CLEANFILES += \
	man/custom-entities.ent

EXTRA_DIST += \
	man/custom-html.xsl \
	man/custom-man.xsl

# ------------------------------------------------------------------------------
SOCKETS_TARGET_WANTS += \
	systemd-initctl.socket

ifneq ($(HAVE_UTMP),)
ifneq ($(HAVE_SYSV_COMPAT),)
MULTI_USER_TARGET_WANTS += \
	systemd-update-utmp-runlevel.service
GRAPHICAL_TARGET_WANTS += \
	systemd-update-utmp-runlevel.service
RESCUE_TARGET_WANTS += \
	systemd-update-utmp-runlevel.service
endif # HAVE_SYSV_COMPAT

SYSINIT_TARGET_WANTS += \
	systemd-update-utmp.service
endif # HAVE_UTMP

SYSINIT_TARGET_WANTS += \
	systemd-update-done.service

LOCAL_FS_TARGET_WANTS += \
	systemd-remount-fs.service \
	tmp.mount \
	var-lib-machines.mount

MULTI_USER_TARGET_WANTS += \
	getty.target \
	systemd-ask-password-wall.path

SYSINIT_TARGET_WANTS += \
	dev-hugepages.mount \
	dev-mqueue.mount \
	sys-kernel-config.mount \
	sys-kernel-debug.mount \
	sys-fs-fuse-connections.mount \
	systemd-sysctl.service \
	systemd-ask-password-console.path

ifneq ($(HAVE_SYSV_COMPAT),)
SYSTEM_UNIT_ALIASES += \
	poweroff.target runlevel0.target \
	rescue.target runlevel1.target \
	multi-user.target runlevel2.target \
	multi-user.target runlevel3.target \
	multi-user.target runlevel4.target \
	graphical.target runlevel5.target \
	reboot.target runlevel6.target
endif # HAVE_SYSV_COMPAT

SYSTEM_UNIT_ALIASES += \
	graphical.target default.target \
	reboot.target ctrl-alt-del.target \
	getty@.service autovt@.service

GENERAL_ALIASES += \
	$(systemunitdir)/remote-fs.target $(pkgsysconfdir)/system/multi-user.target.wants/remote-fs.target \
	$(systemunitdir)/getty@.service $(pkgsysconfdir)/system/getty.target.wants/getty@tty1.service \
	$(pkgsysconfdir)/user $(sysconfdir)/xdg/systemd/user \
	$(dbussystemservicedir)/org.freedesktop.systemd1.service $(dbussessionservicedir)/org.freedesktop.systemd1.service

ifneq ($(HAVE_SYSV_COMPAT),)
INSTALL_DIRS += \
	$(systemunitdir)/runlevel1.target.wants \
	$(systemunitdir)/runlevel2.target.wants \
	$(systemunitdir)/runlevel3.target.wants \
	$(systemunitdir)/runlevel4.target.wants \
	$(systemunitdir)/runlevel5.target.wants
endif # HAVE_SYSV_COMPAT

INSTALL_DIRS += \
	$(prefix)/lib/modules-load.d \
	$(sysconfdir)/modules-load.d \
	$(prefix)/lib/systemd/network \
	$(sysconfdir)/systemd/network \
	$(prefix)/lib/sysctl.d \
	$(sysconfdir)/sysctl.d \
	$(prefix)/lib/kernel/install.d \
	$(sysconfdir)/kernel/install.d \
	$(systemshutdowndir) \
	$(systemsleepdir) \
	$(systemgeneratordir) \
	$(usergeneratordir) \
	\
	$(userunitdir) \
	$(pkgsysconfdir)/system \
	$(pkgsysconfdir)/system/multi-user.target.wants \
	$(pkgsysconfdir)/system/getty.target.wants \
	$(pkgsysconfdir)/user \
	$(dbussessionservicedir) \
	$(sysconfdir)/xdg/systemd

install-exec-hook: $(INSTALL_EXEC_HOOKS)

uninstall-hook: $(UNINSTALL_DATA_HOOKS) $(UNINSTALL_EXEC_HOOKS)

install-data-hook: $(INSTALL_DATA_HOOKS)

distclean-local: $(DISTCLEAN_LOCAL_HOOKS)

clean-local: $(CLEAN_LOCAL_HOOKS)
	rm -rf $(abs_srcdir)/install-tree
	rm -f $(abs_srcdir)/hwdb/usb.ids $(abs_srcdir)/hwdb/pci.ids $(abs_srcdir)/hwdb/oui.txt \
	      $(abs_srcdir)/hwdb/iab.txt

DISTCHECK_CONFIGURE_FLAGS = \
	--with-dbuspolicydir=$$dc_install_base/$(dbuspolicydir) \
	--with-dbussessionservicedir=$$dc_install_base/$(dbussessionservicedir) \
	--with-dbussystemservicedir=$$dc_install_base/$(dbussystemservicedir) \
	--with-bashcompletiondir=$$dc_install_base/$(bashcompletiondir) \
	--with-zshcompletiondir=$$dc_install_base/$(zshcompletiondir) \
	--with-pamlibdir=$$dc_install_base/$(pamlibdir) \
	--with-pamconfdir=$$dc_install_base/$(pamconfdir) \
	--with-rootprefix=$$dc_install_base \
	--enable-compat-libs

ifneq ($(HAVE_SYSV_COMPAT),)
DISTCHECK_CONFIGURE_FLAGS += \
	--with-sysvinit-path=$$dc_install_base/$(sysvinitdir) \
	--with-sysvrcnd-path=$$dc_install_base/$(sysvrcnddir)
else # HAVE_SYSV_COMPAT
DISTCHECK_CONFIGURE_FLAGS += \
	--with-sysvinit-path= \
	--with-sysvrcnd-path=
endif # HAVE_SYSV_COMPAT

ifneq ($(ENABLE_SPLIT_USR),)
DISTCHECK_CONFIGURE_FLAGS += \
	--enable-split-usr
else # ENABLE_SPLIT_USR
DISTCHECK_CONFIGURE_FLAGS += \
	--disable-split-usr
endif # ENABLE_SPLIT_USR

.PHONY: dist-check-help
dist-check-help: $(rootbin_PROGRAMS) $(bin_PROGRAMS)
	for i in $(abspath $^); do                                             \
            if $$i  --help | grep -v 'default:' | grep -E -q '.{80}.' ; then   \
		echo "$(basename $$i) --help output is too wide:";             \
	        $$i  --help | awk 'length > 80' | grep -E --color=yes '.{80}'; \
	        exit 1;                                                        \
            fi; done

include_compilers = "$(CC)" "$(CC) -ansi" "$(CC) -std=iso9899:1990"
public_headers = $(filter-out src/systemd/_sd-common.h, $(pkginclude_HEADERS) $(include_HEADERS))
.PHONY: dist-check-includes
dist-check-includes: $(public_headers)
	@res=0;                                                        	        \
	for i in $(abspath $^); do	                                        \
	    for cc in $(include_compilers); do                                  \
	        echo "$$cc -o/dev/null -c -x c -include "$$i" - </dev/null";    \
	        $$cc -o/dev/null -c -x c -include "$$i" - </dev/null || res=1;  \
	    done;                                                               \
	done; exit $$res

.PHONY: built-sources
built-sources: $(BUILT_SOURCES)

.PHONY: git-tag
git-tag:
	git tag -s "v$(VERSION)" -m "systemd $(VERSION)"

.PHONY: git-tar
git-tar:
	git archive --format=tar --prefix=systemd-$(VERSION)/ HEAD | gzip > systemd-$(VERSION).tar.gz

www_target = www.freedesktop.org:/srv/www.freedesktop.org/www/software/systemd

.PHONY: doc-sync
doc-sync: all
	rsync -rlv --delete-excluded --include="*.html" --exclude="*" --omit-dir-times man/ $(www_target)/man/

.PHONY: install-tree
install-tree: all
	rm -rf $(abs_srcdir)/install-tree
	$(MAKE) install DESTDIR=$(abs_srcdir)/install-tree
	tree $(abs_srcdir)/install-tree

BUILT_SOURCES += \
	test-libsystemd-sym.c \
	test-libudev-sym.c

CLEANFILES += \
	test-libsystemd-sym.c \
	test-libudev-sym.c

tests += \
	test-libsystemd-sym \
	test-libudev-sym


include $(topsrcdir)/build-aux/Makefile.tail.mk
