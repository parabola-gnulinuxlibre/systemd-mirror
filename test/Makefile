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
include $(dir $(lastword $(MAKEFILE_LIST)))/../config.mk
include $(topsrcdir)/build-aux/Makefile.head.mk

manual_tests += \
	test-ns \
	test-cgroup \
	test-install \
	test-btrfs \
	test-acd \
	test-ipv4ll-manual \
	test-ask-password-api

unsafe_tests = \
	test-hostname \
	test-ipcrm

ifneq ($(HAVE_LIBIPTC),)
manual_tests += \
	test-firewall-util
endif # HAVE_LIBIPTC

ifneq ($(HAVE_KMOD),)
manual_tests += \
	test-netlink-manual
endif # HAVE_KMOD

tests += \
	test-daemon \
	test-log \
	test-loopback \
	test-engine \
	test-watchdog \
	test-cgroup-mask \
	test-job-type \
	test-env-util \
	test-strbuf \
	test-strv \
	test-path \
	test-path-util \
	test-strxcpyx \
	test-siphash24 \
	test-unit-name \
	test-unit-file \
	test-utf8 \
	test-ellipsize \
	test-util \
	test-cpu-set-util \
	test-hexdecoct \
	test-escape \
	test-alloc-util \
	test-proc-cmdline \
	test-io-util \
	test-glob-util \
	test-xattr-util \
	test-fs-util \
	test-web-util \
	test-stat-util \
	test-fd-util \
	test-string-util \
	test-extract-word \
	test-parse-util \
	test-user-util \
	test-hostname-util \
	test-process-util \
	test-terminal-util \
	test-path-lookup \
	test-barrier \
	test-tmpfiles \
	test-namespace \
	test-date \
	test-sleep \
	test-replace-var \
	test-sched-prio \
	test-calendarspec \
	test-strip-tab-ansi \
	test-cgroup-util \
	test-fstab-util \
	test-prioq \
	test-fileio \
	test-time \
	test-clock \
	test-hashmap \
	test-set \
	test-bitmap \
	test-list \
	test-unaligned \
	test-tables \
	test-device-nodes \
	test-xml \
	test-architecture \
	test-socket-util \
	test-fdset \
	test-conf-files \
	test-conf-parser \
	test-capability \
	test-async \
	test-ratelimit \
	test-condition \
	test-uid-range \
	test-locale-util \
	test-execute \
	test-copy \
	test-cap-list \
	test-sigbus \
	test-verbs \
	test-af-list \
	test-arphrd-list \
	test-dns-domain \
	test-install-root \
	test-rlimit-util \
	test-signal-util \
	test-selinux \
	test-sizeof

ifneq ($(HAVE_ACL),)
tests += \
	test-acl-util
endif # HAVE_ACL

ifneq ($(HAVE_SECCOMP),)
tests += \
	test-seccomp
endif # HAVE_SECCOMP

EXTRA_DIST += \
	test/a.service \
	test/basic.target \
	test/b.service \
	test/c.service \
	test/daughter.service \
	test/d.service \
	test/end.service \
	test/e.service \
	test/f.service \
	test/grandchild.service \
	test/g.service \
	test/hello-after-sleep.target \
	test/hello.service \
	test/h.service \
	test/parent-deep.slice \
	test/parent.slice \
	test/sched_idle_bad.service \
	test/sched_idle_ok.service \
	test/sched_rr_bad.service \
	test/sched_rr_change.service \
	test/sched_rr_ok.service \
	test/shutdown.target \
	test/sleep.service \
	test/sockets.target \
	test/son.service \
	test/sysinit.target \
	test/testsuite.target \
	test/timers.target \
	test/unstoppable.service \
	test/test-path/paths.target \
	test/test-path/basic.target \
	test/test-path/sysinit.target \
	test/test-path/path-changed.service \
	test/test-path/path-directorynotempty.service \
	test/test-path/path-existsglob.service \
	test/test-path/path-exists.service \
	test/test-path/path-makedirectory.service \
	test/test-path/path-modified.service \
	test/test-path/path-mycustomunit.service \
	test/test-path/path-service.service \
	test/test-path/path-changed.path \
	test/test-path/path-directorynotempty.path \
	test/test-path/path-existsglob.path \
	test/test-path/path-exists.path \
	test/test-path/path-makedirectory.path \
	test/test-path/path-modified.path \
	test/test-path/path-unit.path \
	test/test-execute/exec-environment-empty.service \
	test/test-execute/exec-environment-multiple.service \
	test/test-execute/exec-environment.service \
	test/test-execute/exec-passenvironment-absent.service \
	test/test-execute/exec-passenvironment-empty.service \
	test/test-execute/exec-passenvironment-repeated.service \
	test/test-execute/exec-passenvironment.service \
	test/test-execute/exec-group.service \
	test/test-execute/exec-group-nfsnobody.service \
	test/test-execute/exec-supplementarygroups.service \
	test/test-execute/exec-supplementarygroups-single-group.service \
	test/test-execute/exec-supplementarygroups-single-group-user.service \
	test/test-execute/exec-supplementarygroups-multiple-groups-default-group-user.service \
	test/test-execute/exec-supplementarygroups-multiple-groups-withgid.service \
	test/test-execute/exec-supplementarygroups-multiple-groups-withuid.service \
	test/test-execute/exec-dynamicuser-fixeduser.service \
	test/test-execute/exec-dynamicuser-fixeduser-one-supplementarygroup.service \
	test/test-execute/exec-dynamicuser-supplementarygroups.service \
	test/test-execute/exec-ignoresigpipe-no.service \
	test/test-execute/exec-ignoresigpipe-yes.service \
	test/test-execute/exec-personality-x86-64.service \
	test/test-execute/exec-personality-x86.service \
	test/test-execute/exec-personality-s390.service \
	test/test-execute/exec-personality-ppc64.service \
	test/test-execute/exec-personality-ppc64le.service \
	test/test-execute/exec-personality-aarch64.service \
	test/test-execute/exec-privatedevices-no.service \
	test/test-execute/exec-privatedevices-yes.service \
	test/test-execute/exec-privatedevices-no-capability-mknod.service \
	test/test-execute/exec-privatedevices-yes-capability-mknod.service \
	test/test-execute/exec-protectkernelmodules-no-capabilities.service \
	test/test-execute/exec-protectkernelmodules-yes-capabilities.service \
	test/test-execute/exec-protectkernelmodules-yes-mount-propagation.service \
	test/test-execute/exec-privatetmp-no.service \
	test/test-execute/exec-privatetmp-yes.service \
	test/test-execute/exec-readonlypaths.service \
	test/test-execute/exec-readonlypaths-mount-propagation.service \
	test/test-execute/exec-readwritepaths-mount-propagation.service \
	test/test-execute/exec-inaccessiblepaths-mount-propagation.service \
	test/test-execute/exec-spec-interpolation.service \
	test/test-execute/exec-systemcallerrornumber.service \
	test/test-execute/exec-systemcallfilter-failing2.service \
	test/test-execute/exec-systemcallfilter-failing.service \
	test/test-execute/exec-systemcallfilter-not-failing2.service \
	test/test-execute/exec-systemcallfilter-not-failing.service \
	test/test-execute/exec-systemcallfilter-system-user.service \
	test/test-execute/exec-systemcallfilter-system-user-nfsnobody.service \
	test/test-execute/exec-user.service \
	test/test-execute/exec-user-nfsnobody.service \
	test/test-execute/exec-workingdirectory.service \
	test/test-execute/exec-umask-0177.service \
	test/test-execute/exec-umask-default.service \
	test/test-execute/exec-privatenetwork-yes.service \
	test/test-execute/exec-environmentfile.service \
	test/test-execute/exec-oomscoreadjust-positive.service \
	test/test-execute/exec-oomscoreadjust-negative.service \
	test/test-execute/exec-ioschedulingclass-best-effort.service \
	test/test-execute/exec-ioschedulingclass-idle.service \
	test/test-execute/exec-ioschedulingclass-none.service \
	test/test-execute/exec-ioschedulingclass-realtime.service \
	test/test-execute/exec-capabilityboundingset-invert.service \
	test/test-execute/exec-capabilityboundingset-merge.service \
	test/test-execute/exec-capabilityboundingset-reset.service \
	test/test-execute/exec-capabilityboundingset-simple.service \
	test/test-execute/exec-capabilityambientset.service \
	test/test-execute/exec-capabilityambientset-nfsnobody.service \
	test/test-execute/exec-capabilityambientset-merge.service \
	test/test-execute/exec-capabilityambientset-merge-nfsnobody.service \
	test/test-execute/exec-runtimedirectory.service \
	test/test-execute/exec-runtimedirectory-mode.service \
	test/test-execute/exec-runtimedirectory-owner.service \
	test/test-execute/exec-runtimedirectory-owner-nfsnobody.service \
	test/bus-policy/hello.conf \
	test/bus-policy/methods.conf \
	test/bus-policy/ownerships.conf \
	test/bus-policy/signals.conf \
	test/bus-policy/check-own-rules.conf \
	test/bus-policy/many-rules.conf \
	test/bus-policy/test.conf

EXTRA_DIST += \
	src/test/test-helper.h

test_device_nodes_SOURCES = \
	src/test/test-device-nodes.c

test_device_nodes_LDADD = \
	libsystemd-shared.la

test_engine_SOURCES = \
	src/test/test-engine.c

test_engine_CFLAGS = \
	$(SECCOMP_CFLAGS) \
	$(MOUNT_CFLAGS)

test_engine_LDADD = \
	libcore.la

test_job_type_SOURCES = \
	src/test/test-job-type.c

test_job_type_CFLAGS = \
	$(SECCOMP_CFLAGS) \
	$(MOUNT_CFLAGS)

test_job_type_LDADD = \
	libcore.la

test_ns_SOURCES = \
	src/test/test-ns.c

test_ns_CFLAGS = \
	$(SECCOMP_CFLAGS)

test_ns_LDADD = \
	libcore.la

test_loopback_SOURCES = \
	src/test/test-loopback.c

test_loopback_LDADD = \
	libcore.la

test_hostname_SOURCES = \
	src/test/test-hostname.c

test_hostname_LDADD = \
	libcore.la

test_dns_domain_SOURCES = \
	src/test/test-dns-domain.c

test_dns_domain_LDADD = \
	libsystemd-network.la \
	libsystemd-shared.la

ifneq ($(ENABLE_EFI),)
tests += \
	test-boot-timestamps

test_boot_timestamps_SOURCES = \
	src/test/test-boot-timestamps.c

test_boot_timestamps_LDADD = \
	libsystemd-shared.la
endif # ENABLE_EFI

test_unit_name_SOURCES = \
	src/test/test-unit-name.c

test_unit_name_CFLAGS = \
	$(SECCOMP_CFLAGS) \
	$(MOUNT_CFLAGS)

test_unit_name_LDADD = \
	libcore.la

test_unit_file_SOURCES = \
	src/test/test-unit-file.c

test_unit_file_CFLAGS = \
	$(SECCOMP_CFLAGS) \
	$(MOUNT_CFLAGS)

test_unit_file_LDADD = \
	libcore.la

test_utf8_SOURCES = \
	src/test/test-utf8.c

test_utf8_LDADD = \
	libsystemd-shared.la

test_capability_SOURCES = \
	src/test/test-capability.c

test_capability_CFLAGS = \
	$(CAP_CFLAGS)

test_capability_LDADD = \
	libsystemd-shared.la \
	$(CAP_LIBS)

test_async_SOURCES = \
	src/test/test-async.c

test_async_LDADD = \
	libsystemd-shared.la

test_locale_util_SOURCES = \
	src/test/test-locale-util.c

test_locale_util_LDADD = \
	libsystemd-shared.la

test_copy_SOURCES = \
	src/test/test-copy.c

# Link statically to ensure file is large
test_copy_LDADD = \
	libsystemd-shared.la

test_sigbus_SOURCES = \
	src/test/test-sigbus.c

test_sigbus_LDADD = \
	libsystemd-shared.la

test_condition_SOURCES = \
	src/test/test-condition.c

test_condition_LDADD = \
	libsystemd-shared.la

test_fdset_SOURCES = \
	src/test/test-fdset.c

test_fdset_LDADD = \
	libsystemd-shared.la

test_fstab_util_SOURCES = \
	src/test/test-fstab-util.c

test_fstab_util_LDADD = \
	libsystemd-shared.la

test_ratelimit_SOURCES = \
	src/test/test-ratelimit.c

test_ratelimit_LDADD = \
	libsystemd-shared.la

test_util_SOURCES = \
	src/test/test-util.c

test_util_LDADD = \
	libsystemd-shared.la

test_hexdecoct_SOURCES = \
	src/test/test-hexdecoct.c

test_hexdecoct_LDADD = \
	libsystemd-shared.la

test_alloc_util_SOURCES = \
	src/test/test-alloc-util.c

test_alloc_util_LDADD = \
	libsystemd-shared.la

test_xattr_util_SOURCES = \
	src/test/test-xattr-util.c

test_xattr_util_LDADD = \
	libsystemd-shared.la

test_io_util_SOURCES = \
	src/test/test-io-util.c

test_io_util_LDADD = \
	libsystemd-shared.la

test_glob_util_SOURCES = \
	src/test/test-glob-util.c

test_glob_util_LDADD = \
	libsystemd-shared.la

test_fs_util_SOURCES = \
	src/test/test-fs-util.c

test_fs_util_LDADD = \
	libsystemd-shared.la

test_proc_cmdline_SOURCES = \
	src/test/test-proc-cmdline.c

test_proc_cmdline_LDADD = \
	libsystemd-shared.la

test_fd_util_SOURCES = \
	src/test/test-fd-util.c

test_fd_util_LDADD = \
	libsystemd-shared.la

test_web_util_SOURCES = \
	src/test/test-web-util.c

test_web_util_LDADD = \
	libsystemd-shared.la

test_cpu_set_util_SOURCES = \
	src/test/test-cpu-set-util.c

test_cpu_set_util_LDADD = \
	libsystemd-shared.la

test_stat_util_SOURCES = \
	src/test/test-stat-util.c

test_stat_util_LDADD = \
	libsystemd-shared.la

test_escape_SOURCES = \
	src/test/test-escape.c

test_escape_LDADD = \
	libsystemd-shared.la

test_string_util_SOURCES = \
	src/test/test-string-util.c

test_string_util_LDADD = \
	libsystemd-shared.la

test_extract_word_SOURCES = \
	src/test/test-extract-word.c

test_extract_word_LDADD = \
	libsystemd-shared.la

test_parse_util_SOURCES = \
	src/test/test-parse-util.c

test_parse_util_LDADD = \
	libsystemd-shared.la

test_user_util_SOURCES = \
	src/test/test-user-util.c

test_user_util_LDADD = \
	libsystemd-shared.la

test_hostname_util_SOURCES = \
	src/test/test-hostname-util.c

test_hostname_util_LDADD = \
	libsystemd-shared.la

test_process_util_SOURCES = \
	src/test/test-process-util.c

test_process_util_LDADD = \
	libsystemd-shared.la

test_terminal_util_SOURCES = \
	src/test/test-terminal-util.c

test_terminal_util_LDADD = \
	libsystemd-shared.la

test_path_lookup_SOURCES = \
	src/test/test-path-lookup.c

test_path_lookup_LDADD = \
	libsystemd-shared.la

test_uid_range_SOURCES = \
	src/test/test-uid-range.c

test_uid_range_LDADD = \
	libsystemd-shared.la

test_cap_list_SOURCES = \
	src/test/test-cap-list.c

test_cap_list_CFLAGS = \
	$(CAP_CFLAGS)

test_cap_list_LDADD = \
	libsystemd-shared.la \
	$(CAP_LIBS)

test_socket_util_SOURCES = \
	src/test/test-socket-util.c

test_socket_util_LDADD = \
	libsystemd-shared.la

test_barrier_SOURCES = \
	src/test/test-barrier.c

test_barrier_LDADD = \
	libsystemd-shared.la

test_tmpfiles_SOURCES = \
	src/test/test-tmpfiles.c

test_tmpfiles_LDADD = \
	libsystemd-shared.la

test_namespace_SOURCES = \
	src/test/test-namespace.c

test_verbs_SOURCES = \
	src/test/test-verbs.c

test_verbs_LDADD = \
	libsystemd-shared.la

test_install_root_SOURCES = \
	src/test/test-install-root.c

test_install_root_LDADD = \
	libsystemd-shared.la

test_acl_util_SOURCES = \
	src/test/test-acl-util.c

test_acl_util_LDADD = \
	libsystemd-shared.la

test_seccomp_SOURCES = \
	src/test/test-seccomp.c

test_seccomp_LDADD = \
	libsystemd-shared.la

test_namespace_LDADD = \
	libcore.la

test_rlimit_util_SOURCES = \
	src/test/test-rlimit-util.c

test_rlimit_util_LDADD = \
	libsystemd-shared.la

test_ask_password_api_SOURCES = \
	src/test/test-ask-password-api.c

test_ask_password_api_LDADD = \
	libsystemd-shared.la

test_signal_util_SOURCES = \
	src/test/test-signal-util.c

test_signal_util_LDADD = \
	libsystemd-shared.la

test_selinux_SOURCES = \
	src/test/test-selinux.c

test_selinux_LDADD = \
	libsystemd-shared.la

test_sizeof_SOURCES = \
	src/test/test-sizeof.c

test_sizeof_LDADD = \
	libsystemd-shared.la

BUILT_SOURCES += \
	src/test/test-hashmap-ordered.c

$(outdir)/test-hashmap-ordered.c: src/test/test-hashmap-plain.c
	$(AM_V_GEN)$(AWK) 'BEGIN { print "/* GENERATED FILE */\n#define ORDERED" } \
	                   { if (!match($$0, "^#include"))          \
	                         gsub(/hashmap/, "ordered_hashmap"); \
	                     gsub(/HASHMAP/, "ORDERED_HASHMAP");     \
	                     gsub(/Hashmap/, "OrderedHashmap");      \
	                     print }' <$< >$@

nodist_test_hashmap_SOURCES = \
	src/test/test-hashmap-ordered.c

test_hashmap_SOURCES = \
	src/test/test-hashmap.c \
	src/test/test-hashmap-plain.c

test_hashmap_LDADD = \
	libsystemd-shared.la

test_set_SOURCES = \
	src/test/test-set.c

test_set_LDADD = \
	libsystemd-shared.la

test_bitmap_SOURCES = \
	src/test/test-bitmap.c

test_bitmap_LDADD = \
	libsystemd-shared.la

test_xml_SOURCES = \
	src/test/test-xml.c

test_xml_LDADD = \
	libsystemd-shared.la

test_list_SOURCES = \
	src/test/test-list.c

test_list_LDADD = \
	libsystemd-shared.la

test_unaligned_LDADD = \
	libsystemd-shared.la

test_unaligned_SOURCES = \
	src/test/test-unaligned.c

test_tables_SOURCES = \
	src/test/test-tables.c \
	src/shared/test-tables.h \
	src/journal/journald-server.c \
	src/journal/journald-server.h

test_tables_CPPFLAGS = \

test_tables_CFLAGS = \
	$(SECCOMP_CFLAGS) \
	$(MOUNT_CFLAGS)

test_tables_LDADD = \
	libjournal-core.la \
	libcore.la \
	libudev-core.la

test_prioq_SOURCES = \
	src/test/test-prioq.c

test_prioq_LDADD = \
	libsystemd-shared.la

test_fileio_SOURCES = \
	src/test/test-fileio.c

test_fileio_LDADD = \
	libsystemd-shared.la

test_time_SOURCES = \
	src/test/test-time.c

test_time_LDADD = \
	libsystemd-shared.la

test_clock_SOURCES = \
	src/test/test-clock.c

test_clock_LDADD = \
	libsystemd-shared.la

test_architecture_SOURCES = \
	src/test/test-architecture.c

test_architecture_LDADD = \
	libsystemd-shared.la

test_log_SOURCES = \
	src/test/test-log.c

test_log_LDADD = \
	libsystemd-shared.la

test_ipcrm_SOURCES = \
	src/test/test-ipcrm.c

test_ipcrm_LDADD = \
	libsystemd-shared.la

test_btrfs_SOURCES = \
	src/test/test-btrfs.c

test_btrfs_LDADD = \
	libsystemd-shared.la

ifneq ($(HAVE_LIBIPTC),)
test_firewall_util_SOURCES = \
	src/test/test-firewall-util.c

test_firewall_util_CFLAGS = \
	$(LIBIPTC_CFLAGS)

test_firewall_util_LDADD = \
	libsystemd-firewall.la \
	libsystemd-shared.la \
	$(LIBIPTC_LIBS)
endif # HAVE_LIBIPTC

test_netlink_manual_SOURCES = \
	src/test/test-netlink-manual.c

test_netlink_manual_CFLAGS = \
	$(KMOD_CFLAGS)

test_netlink_manual_LDADD = \
	libsystemd-shared.la \
	$(KMOD_LIBS)

test_ellipsize_SOURCES = \
	src/test/test-ellipsize.c

test_ellipsize_LDADD = \
	libsystemd-shared.la

test_date_SOURCES = \
	src/test/test-date.c

test_date_LDADD = \
	libsystemd-shared.la

test_sleep_SOURCES = \
	src/test/test-sleep.c

test_sleep_LDADD = \
	libcore.la

test_replace_var_SOURCES = \
	src/test/test-replace-var.c

test_replace_var_LDADD = \
	libsystemd-shared.la

test_calendarspec_SOURCES = \
	src/test/test-calendarspec.c

test_calendarspec_LDADD = \
	libsystemd-shared.la

test_strip_tab_ansi_SOURCES = \
	src/test/test-strip-tab-ansi.c

test_strip_tab_ansi_LDADD = \
	libsystemd-shared.la

test_daemon_SOURCES = \
	src/test/test-daemon.c

test_daemon_LDADD = \
	libsystemd-shared.la

test_cgroup_SOURCES = \
	src/test/test-cgroup.c

test_cgroup_LDADD = \
	libsystemd-shared.la

test_cgroup_mask_SOURCES = \
	src/test/test-cgroup-mask.c

test_cgroup_mask_CPPFLAGS = \
	$(MOUNT_CFLAGS)

test_cgroup_mask_CFLAGS = \
	$(SECCOMP_CFLAGS)

test_cgroup_mask_LDADD = \
	libcore.la

test_cgroup_util_SOURCES = \
	src/test/test-cgroup-util.c

test_cgroup_util_LDADD = \
	libsystemd-shared.la

test_env_util_SOURCES = \
	src/test/test-env-util.c

test_env_util_LDADD = \
	libsystemd-shared.la

test_strbuf_SOURCES = \
	src/test/test-strbuf.c

test_strbuf_LDADD = \
	libsystemd-shared.la

test_strv_SOURCES = \
	src/test/test-strv.c

test_strv_LDADD = \
	libsystemd-shared.la

test_path_util_SOURCES = \
	src/test/test-path-util.c

test_path_util_LDADD = \
	libsystemd-shared.la

test_path_SOURCES = \
	src/test/test-path.c

test_path_CFLAGS = \
	$(MOUNT_CFLAGS)

test_path_LDADD = \
	libcore.la

test_execute_SOURCES = \
	src/test/test-execute.c

test_execute_CFLAGS = \
	$(MOUNT_CFLAGS)

test_execute_LDADD = \
	libcore.la

test_siphash24_SOURCES = \
	src/test/test-siphash24.c

test_siphash24_LDADD = \
	libsystemd-shared.la

test_strxcpyx_SOURCES = \
	src/test/test-strxcpyx.c

test_strxcpyx_LDADD = \
	libsystemd-shared.la

test_install_SOURCES = \
	src/test/test-install.c

test_install_LDADD = \
	libsystemd-shared.la

test_watchdog_SOURCES = \
	src/test/test-watchdog.c

test_watchdog_LDADD = \
	libsystemd-shared.la

test_sched_prio_SOURCES = \
	src/test/test-sched-prio.c

test_sched_prio_CPPFLAGS = \
	$(MOUNT_CFLAGS)

test_sched_prio_CFLAGS = \
	$(SECCOMP_CFLAGS)

test_sched_prio_LDADD = \
	libcore.la

test_conf_files_SOURCES = \
	src/test/test-conf-files.c

test_conf_files_LDADD = \
	libsystemd-shared.la

test_conf_parser_SOURCES = \
	src/test/test-conf-parser.c

test_conf_parser_LDADD = \
	libsystemd-shared.la

test_af_list_SOURCES = \
	src/test/test-af-list.c

test_af_list_LDADD = \
	libsystemd-shared.la

test_arphrd_list_SOURCES = \
	src/test/test-arphrd-list.c

test_arphrd_list_LDADD = \
	libsystemd-shared.la

include $(topsrcdir)/build-aux/Makefile.tail.mk
