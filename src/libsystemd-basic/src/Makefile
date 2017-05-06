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

noinst_LTLIBRARIES += \
	libsystemd-basic.la

libsystemd_basic_la_SOURCES = \
	src/basic/missing.h \
	src/basic/missing_syscall.h \
	src/basic/raw-clone.h \
	src/basic/capability-util.c \
	src/basic/capability-util.h \
	src/basic/conf-files.c \
	src/basic/conf-files.h \
	src/basic/stdio-util.h \
	src/basic/hostname-util.h \
	src/basic/hostname-util.c \
	src/basic/unit-name.c \
	src/basic/unit-name.h \
	src/basic/ioprio.h \
	src/basic/securebits.h \
	src/basic/special.h \
	src/basic/list.h \
	src/basic/unaligned.h \
	src/basic/macro.h \
	src/basic/def.h \
	src/basic/sparse-endian.h \
	src/basic/refcnt.h \
	src/basic/util.c \
	src/basic/util.h \
	src/basic/io-util.c \
	src/basic/io-util.h \
	src/basic/string-util.c \
	src/basic/string-util.h \
	src/basic/fd-util.c \
	src/basic/fd-util.h \
	src/basic/parse-util.c \
	src/basic/parse-util.h \
	src/basic/user-util.c \
	src/basic/user-util.h \
	src/basic/rlimit-util.c \
	src/basic/rlimit-util.h \
	src/basic/dirent-util.c \
	src/basic/dirent-util.h \
	src/basic/xattr-util.c \
	src/basic/xattr-util.h \
	src/basic/chattr-util.c \
	src/basic/chattr-util.h \
	src/basic/proc-cmdline.c \
	src/basic/proc-cmdline.h \
	src/basic/fs-util.c \
	src/basic/fs-util.h \
	src/basic/syslog-util.c \
	src/basic/syslog-util.h \
	src/basic/stat-util.c \
	src/basic/stat-util.h \
	src/basic/mount-util.c \
	src/basic/mount-util.h \
	src/basic/hexdecoct.c \
	src/basic/hexdecoct.h \
	src/basic/glob-util.h \
	src/basic/glob-util.c \
	src/basic/extract-word.c \
	src/basic/extract-word.h \
	src/basic/escape.c \
	src/basic/escape.h \
	src/basic/cpu-set-util.c \
	src/basic/cpu-set-util.h \
	src/basic/lockfile-util.c \
	src/basic/lockfile-util.h \
	src/basic/path-util.c \
	src/basic/path-util.h \
	src/basic/time-util.c \
	src/basic/time-util.h \
	src/basic/locale-util.c \
	src/basic/locale-util.h \
	src/basic/umask-util.h \
	src/basic/signal-util.c \
	src/basic/signal-util.h \
	src/basic/string-table.c \
	src/basic/string-table.h \
	src/basic/mempool.c \
	src/basic/mempool.h \
	src/basic/hashmap.c \
	src/basic/hashmap.h \
	src/basic/hash-funcs.c \
	src/basic/hash-funcs.h \
	src/basic/siphash24.c \
	src/basic/siphash24.h \
	src/basic/set.h \
	src/basic/ordered-set.h \
	src/basic/ordered-set.c \
	src/basic/bitmap.c \
	src/basic/bitmap.h \
	src/basic/prioq.c \
	src/basic/prioq.h \
	src/basic/web-util.c \
	src/basic/web-util.h \
	src/basic/strv.c \
	src/basic/strv.h \
	src/basic/env-util.c \
	src/basic/env-util.h \
	src/basic/strbuf.c \
	src/basic/strbuf.h \
	src/basic/strxcpyx.c \
	src/basic/strxcpyx.h \
	src/basic/log.c \
	src/basic/log.h \
	src/basic/bus-label.c \
	src/basic/bus-label.h \
	src/basic/ratelimit.h \
	src/basic/ratelimit.c \
	src/basic/exit-status.c \
	src/basic/exit-status.h \
	src/basic/virt.c \
	src/basic/virt.h \
	src/basic/architecture.c \
	src/basic/architecture.h \
	src/basic/smack-util.c \
	src/basic/smack-util.h \
	src/basic/device-nodes.c \
	src/basic/device-nodes.h \
	src/basic/utf8.c \
	src/basic/utf8.h \
	src/basic/gunicode.c \
	src/basic/gunicode.h \
	src/basic/socket-util.c \
	src/basic/socket-util.h \
	src/basic/in-addr-util.c \
	src/basic/in-addr-util.h \
	src/basic/ether-addr-util.h \
	src/basic/ether-addr-util.c \
	src/basic/replace-var.c \
	src/basic/replace-var.h \
	src/basic/clock-util.c \
	src/basic/clock-util.h \
	src/basic/calendarspec.c \
	src/basic/calendarspec.h \
	src/basic/fileio.c \
	src/basic/fileio.h \
	src/basic/MurmurHash2.c \
	src/basic/MurmurHash2.h \
	src/basic/mkdir.c \
	src/basic/mkdir.h \
	src/basic/cgroup-util.c \
	src/basic/cgroup-util.h \
	src/basic/errno-list.c \
	src/basic/errno-list.h \
	src/basic/af-list.c \
	src/basic/af-list.h \
	src/basic/arphrd-list.c \
	src/basic/arphrd-list.h \
	src/basic/terminal-util.c \
	src/basic/terminal-util.h \
	src/basic/login-util.h \
	src/basic/login-util.c \
	src/basic/cap-list.c \
	src/basic/cap-list.h \
	src/basic/audit-util.c \
	src/basic/audit-util.h \
	src/basic/xml.c \
	src/basic/xml.h \
	src/basic/barrier.c \
	src/basic/barrier.h \
	src/basic/async.c \
	src/basic/async.h \
	src/basic/memfd-util.c \
	src/basic/memfd-util.h \
	src/basic/process-util.c \
	src/basic/process-util.h \
	src/basic/random-util.c \
	src/basic/random-util.h \
	src/basic/verbs.c \
	src/basic/verbs.h \
	src/basic/sigbus.c \
	src/basic/sigbus.h \
	src/basic/build.h \
	src/basic/socket-label.c \
	src/basic/label.c \
	src/basic/label.h \
	src/basic/btrfs-util.c \
	src/basic/btrfs-util.h \
	src/basic/btrfs-ctree.h \
	src/basic/selinux-util.c \
	src/basic/selinux-util.h \
	src/basic/mkdir-label.c \
	src/basic/fileio-label.c \
	src/basic/fileio-label.h \
	src/basic/rm-rf.c \
	src/basic/rm-rf.h \
	src/basic/copy.c \
	src/basic/copy.h \
	src/basic/alloc-util.h \
	src/basic/alloc-util.c \
	src/basic/formats-util.h \
	src/basic/nss-util.h

nodist_libsystemd_basic_la_SOURCES = \
	src/basic/errno-from-name.h \
	src/basic/errno-to-name.h \
	src/basic/af-from-name.h \
	src/basic/af-to-name.h \
	src/basic/arphrd-from-name.h \
	src/basic/arphrd-to-name.h \
	src/basic/cap-from-name.h \
	src/basic/cap-to-name.h

libsystemd_basic_la_CFLAGS = \
	$(SELINUX_CFLAGS) \
	$(CAP_CFLAGS) \
	-pthread

libsystemd_basic_la_LIBADD = \
	$(SELINUX_LIBS) \
	$(CAP_LIBS) \
	-lrt \
	-lm

$(outdir)/errno-list.txt:
	$(AM_V_GEN)$(CPP) $(sd.ALL_CPPFLAGS) -dM -include errno.h - </dev/null | $(AWK) '/^#define[ \t]+E[^ _]+[ \t]+/ { print $$2; }' >$@

$(outdir)/errno-to-name.h: $(outdir)/errno-list.txt
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "static const char* const errno_names[] = { "} !/EDEADLOCK/ && !/EWOULDBLOCK/ && !/ENOTSUP/ { printf "[%s] = \"%s\",\n", $$1, $$1 } END{print "};"}' <$< >$@

$(outdir)/af-list.txt:
	$(AM_V_GEN)$(CPP) $(sd.ALL_CPPFLAGS) -dM -include sys/socket.h - </dev/null | grep -v AF_UNSPEC | grep -v AF_MAX | $(AWK) '/^#define[ \t]+AF_[^ \t]+[ \t]+PF_[^ \t]/ { print $$2; }' >$@

$(outdir)/af-to-name.h: $(outdir)/af-list.txt
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "static const char* const af_names[] = { "} !/AF_FILE/ && !/AF_ROUTE/ && !/AF_LOCAL/ { printf "[%s] = \"%s\",\n", $$1, $$1 } END{print "};"}' <$< >$@

$(outdir)/arphrd-list.txt:
	$(AM_V_GEN)$(CPP) $(sd.ALL_CPPFLAGS) -dM -include net/if_arp.h - </dev/null | $(AWK) '/^#define[ \t]+ARPHRD_[^ \t]+[ \t]+[^ \t]/ { print $$2; }' | sed -e 's/ARPHRD_//' >$@

$(outdir)/arphrd-to-name.h: $(outdir)/arphrd-list.txt
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "static const char* const arphrd_names[] = { "} !/CISCO/ { printf "[ARPHRD_%s] = \"%s\",\n", $$1, $$1 } END{print "};"}' <$< >$@

$(outdir)/arphrd-from-name.gperf: $(outdir)/arphrd-list.txt
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "struct arphrd_name { const char* name; int id; };"; print "%null-strings"; print "%%";} { printf "%s, ARPHRD_%s\n", $$1, $$1 }' <$< >$@

$(outdir)/cap-list.txt:
	$(AM_V_GEN)$(CPP) $(sd.ALL_CPPFLAGS) -dM -include linux/capability.h -include missing.h - </dev/null | $(AWK) '/^#define[ \t]+CAP_[A-Z_]+[ \t]+/ { print $$2; }' | grep -v CAP_LAST_CAP >$@

$(outdir)/cap-to-name.h: $(outdir)/cap-list.txt
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "static const char* const capability_names[] = { "} { printf "[%s] = \"%s\",\n", $$1, tolower($$1) } END{print "};"}' <$< >$@

$(outdir)/cap-from-name.gperf: $(outdir)/cap-list.txt
	$(AM_V_GEN)$(AWK) 'BEGIN{ print "struct capability_name { const char* name; int id; };"; print "%null-strings"; print "%%";} { printf "%s, %s\n", $$1, $$1 }' <$< >$@

$(outdir)/cap-from-name.h: $(outdir)/cap-from-name.gperf
	$(AM_V_GPERF)$(GPERF) -L ANSI-C -t --ignore-case -N lookup_capability -H hash_capability_name -p -C <$< >$@

include $(topsrcdir)/build-aux/Makefile.tail.mk
