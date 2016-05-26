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

ifeq ($(topsrcdir),)
topoutdir := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
abs_topsrcdir := /home/luke/src/parabola/systemd
topsrcdir = $(if $(am_path),$(call am_path,$(abs_topsrcdir)),$(abs_topsrcdir))

GZIP_ENV = --best
DIST_ARCHIVES = $(distdir).tar.xz
DIST_TARGETS = dist-xz
distuninstallcheck_listfiles = find . -type f -print
am__distuninstallcheck_listfiles = $(distuninstallcheck_listfiles) \
  | sed 's|^\./|$(prefix)/|' | grep -v '$(infodir)/dir$$'
distcleancheck_listfiles = find . -type f -print
pkgincludedir = $(includedir)/systemd
ACLOCAL = ${SHELL} /home/luke/src/3rd-party/systemd/build-aux/missing aclocal-1.15
ACL_LIBS = -lacl
ALL_LINGUAS = 
AMTAR = $${TAR-tar}
AM_DEFAULT_VERBOSITY = 0
APPARMOR_CFLAGS = 
APPARMOR_LIBS = 
AR = gcc-ar
AUDIT_LIBS = 
AUTOCONF = ${SHELL} /home/luke/src/3rd-party/systemd/build-aux/missing autoconf
AUTOHEADER = ${SHELL} /home/luke/src/3rd-party/systemd/build-aux/missing autoheader
AUTOMAKE = ${SHELL} /home/luke/src/3rd-party/systemd/build-aux/missing automake-1.15
AWK = gawk
BLKID_CFLAGS = -I/usr/include/blkid -I/usr/include/uuid
BLKID_LIBS = -lblkid
CAP_LIBS = -lcap 
CC = gcc
CCDEPMODE = depmode=gcc3
CERTIFICATEROOT = /etc/ssl
CFLAGS = -g -O2
CPP = gcc -E
CPPFLAGS = 
CYGPATH_W = echo
DBUS_CFLAGS = -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include
DBUS_LIBS = -ldbus-1
DEBUGTTY = /dev/tty9
DEFS = -DHAVE_CONFIG_H
DEPDIR = .deps
DLLTOOL = false
DNS_SERVERS = 8.8.8.8 8.8.4.4 2001:4860:4860::8888 2001:4860:4860::8844
DSYMUTIL = 
DUMPBIN = 
ECHO_C = 
ECHO_N = -n
ECHO_T = 
EFI_ARCH = x86_64
EFI_CC = gcc
EFI_INC_DIR = /usr/include
EFI_LDS_DIR = /usr/lib
EFI_LIB_DIR = /usr/lib
EFI_MACHINE_TYPE_NAME = x64
EGREP = /usr/bin/grep -E
ELFUTILS_LIBS = -lelf -ldw
EXEEXT = 
FGREP = /usr/bin/grep -F
GCRYPT_CFLAGS =  
GCRYPT_LIBS = -lgcrypt -lgpg-error -lgpg-error
GETTEXT_PACKAGE = systemd
GMSGFMT = /usr/bin/msgfmt
GNUTLS_CFLAGS = -I/usr/include/p11-kit-1
GNUTLS_LIBS = -lgnutls
GPERF = gperf
GPG_ERROR_CFLAGS = 
GPG_ERROR_CONFIG = /usr/bin/gpg-error-config
GPG_ERROR_LIBS = -lgpg-error
GPG_ERROR_MT_CFLAGS = 
GPG_ERROR_MT_LIBS = -lgpg-error -lpthread
GREP = /usr/bin/grep
INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_PROGRAM = ${INSTALL}
INSTALL_SCRIPT = ${INSTALL}
INSTALL_STRIP_PROGRAM = $(install_sh) -c -s
INTLTOOL_EXTRACT = /usr/bin/intltool-extract
INTLTOOL_MERGE = /usr/bin/intltool-merge
INTLTOOL_PERL = /usr/bin/perl
INTLTOOL_UPDATE = /usr/bin/intltool-update
INTLTOOL_V_MERGE = $(INTLTOOL__v_MERGE_$(V))
INTLTOOL_V_MERGE_OPTIONS = $(intltool__v_merge_options_$(V))
INTLTOOL__v_MERGE_ = $(INTLTOOL__v_MERGE_$(AM_DEFAULT_VERBOSITY))
INTLTOOL__v_MERGE_0 = @echo "  ITMRG " $@;
KBD_LOADKEYS = /usr/bin/loadkeys
KBD_SETFONT = /usr/bin/setfont
KEXEC = /usr/sbin/kexec
KILL = /usr/bin/kill
KMOD = /usr/bin/kmod
KMOD_CFLAGS = 
KMOD_LIBS = -lkmod
LD = /usr/bin/ld -m elf_x86_64
LDFLAGS = 
LIBCRYPTSETUP_CFLAGS = 
LIBCRYPTSETUP_LIBS = -lcryptsetup
LIBCURL_CFLAGS = 
LIBCURL_LIBS = -lcurl
LIBGCRYPT_CFLAGS = 
LIBGCRYPT_CONFIG = /usr/bin/libgcrypt-config
LIBGCRYPT_LIBS = -lgcrypt -lgpg-error
LIBIDN_CFLAGS = 
LIBIDN_LIBS = -lidn
LIBIPTC_CFLAGS = 
LIBIPTC_LIBS = -lip4tc -lip6tc
LIBOBJS = 
LIBS = 
LIBTOOL = $(SHELL) $(topoutdir)/libtool
LIPO = 
LN_S = ln -s
LTLIBOBJS = 
LT_SYS_LIBRARY_PATH = 
LZ4_CFLAGS = 
LZ4_LIBS = -llz4
M4 = /usr/bin/m4
M4_DEFINES =  -DHAVE_SECCOMP -DHAVE_PAM -DHAVE_ACL -DHAVE_SMACK -DHAVE_MICROHTTPD -DHAVE_LIBCURL -DHAVE_LIBIDN -DHAVE_LIBIPTC -DENABLE_TIMESYNCD -DENABLE_COREDUMP -DENABLE_RESOLVED -DENABLE_NETWORKD -DENABLE_KDBUS
MAINT = 
MAKEINFO = ${SHELL} /home/luke/src/3rd-party/systemd/build-aux/missing makeinfo
MANIFEST_TOOL = :
MICROHTTPD_CFLAGS = -I/usr/include/libmicrohttpd -I/usr/include/p11-kit-1
MICROHTTPD_LIBS = -lmicrohttpd
MKDIR_P = /usr/bin/mkdir -p
MOUNT_CFLAGS = -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/uuid
MOUNT_LIBS = -lmount
MOUNT_PATH = /usr/bin/mount
MSGFMT = /usr/bin/msgfmt
MSGMERGE = /usr/bin/msgmerge
NM = gcc-nm
NMEDIT = 
NTP_SERVERS = 0.arch.pool.ntp.org 1.arch.pool.ntp.org 2.arch.pool.ntp.org 3.arch.pool.ntp.org
OBJCOPY = objcopy
OBJDUMP = objdump
OBJEXT = o
OTOOL = 
OTOOL64 = 
OUR_CFLAGS =  -pipe -Wall -Wextra -Wundef -Wformat=2 -Wformat-security -Wformat-nonliteral -Wlogical-op -Wmissing-include-dirs -Wold-style-definition -Wpointer-arith -Winit-self -Wdeclaration-after-statement -Wfloat-equal -Wsuggest-attribute=noreturn -Werror=missing-prototypes -Werror=implicit-function-declaration -Werror=missing-declarations -Werror=return-type -Wstrict-prototypes -Wredundant-decls -Wmissing-noreturn -Wshadow -Wendif-labels -Wstrict-aliasing=2 -Wwrite-strings -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-result -Wno-format-signedness -Werror=overflow -Wdate-time -Wnested-externs -ffast-math -fno-common -fdiagnostics-show-option -fno-strict-aliasing -fvisibility=hidden -fstack-protector -fstack-protector-strong -fPIE --param=ssp-buffer-size=4 -Werror=shadow -flto -ffunction-sections -fdata-sections  
OUR_CPPFLAGS =  -Wp,-D_FORTIFY_SOURCE=2  
OUR_LDFLAGS =  -Wl,--gc-sections -Wl,--as-needed -Wl,--no-undefined -Wl,-z,relro -Wl,-z,now -pie -Wl,-fuse-ld=gold  
PACKAGE = systemd
PACKAGE_BUGREPORT = http://github.com/systemd/systemd/issues
PACKAGE_NAME = systemd
PACKAGE_STRING = systemd 229
PACKAGE_TARNAME = systemd
PACKAGE_URL = http://www.freedesktop.org/wiki/Software/systemd
PACKAGE_VERSION = 229
PAM_LIBS = -lpam -lpam_misc
PATH_SEPARATOR = :
PKG_CONFIG = /usr/bin/pkg-config
PKG_CONFIG_LIBDIR = 
PKG_CONFIG_PATH = /home/luke/.prefix/lib/pkgconfig
PYTHON = /usr/bin/python
PYTHON_EXEC_PREFIX = ${exec_prefix}
PYTHON_PLATFORM = linux
PYTHON_PREFIX = ${prefix}
PYTHON_VERSION = 3.5
QEMU = /usr/bin/qemu-system-x86_64
QEMU_BIOS = 
QRENCODE_CFLAGS = 
QRENCODE_LIBS = 
QUOTACHECK = /usr/bin/quotacheck
QUOTAON = /usr/bin/quotaon
RANLIB = gcc-ranlib
RC_LOCAL_SCRIPT_PATH_START = /etc/rc.local
RC_LOCAL_SCRIPT_PATH_STOP = /usr/sbin/halt.local
SECCOMP_CFLAGS = 
SECCOMP_LIBS = -lseccomp
SED = /usr/bin/sed
SELINUX_CFLAGS = 
SELINUX_LIBS = 
SETCAP = /usr/bin/setcap
SET_MAKE = 
SHELL = /bin/sh
STRINGS = strings
STRIP = strip
SULOGIN = /usr/bin/sulogin
SUSHELL = /bin/sh
SYSTEM_GID_MAX = 999
SYSTEM_SYSVINIT_PATH = 
SYSTEM_SYSVRCND_PATH = 
SYSTEM_UID_MAX = 999
TELINIT = /lib/sysvinit/telinit
TTY_GID = 5
UMOUNT_PATH = /usr/bin/umount
USE_NLS = yes
VERSION = 229
XGETTEXT = /usr/bin/xgettext
XKBCOMMON_CFLAGS = 
XKBCOMMON_LIBS = -lxkbcommon
XSLTPROC = /usr/bin/xsltproc
XZ_CFLAGS = 
XZ_LIBS = -llzma
ZLIB_CFLAGS = 
ZLIB_LIBS = -lz
abs_builddir = /home/luke/src/3rd-party/systemd
abs_srcdir = /home/luke/src/3rd-party/systemd
abs_top_builddir = /home/luke/src/3rd-party/systemd
abs_top_srcdir = /home/luke/src/3rd-party/systemd
ac_ct_AR = gcc-ar
ac_ct_CC = gcc
ac_ct_DUMPBIN = 
ac_ct_NM = gcc-nm
ac_ct_RANLIB = gcc-ranlib
am__include = include
am__leading_dot = .
am__quote = 
am__tar = tar --format=posix -chf - "$$tardir"
am__untar = tar -xf -
bashcompletiondir = /usr/share/bash-completion/completions
bindir = ${exec_prefix}/bin
build = x86_64-unknown-linux-gnu
build_alias = 
build_cpu = x86_64
build_os = linux-gnu
build_vendor = unknown
builddir = .
datadir = ${datarootdir}
datarootdir = ${prefix}/share

# Dirs of external packages
dbuspolicydir=${prefix}/etc/dbus-1/system.d
dbussessionservicedir=${datarootdir}/dbus-1/services
dbussystemservicedir=${datarootdir}/dbus-1/system-services
pamlibdir=${exec_prefix}/lib/security
pamconfdir=${prefix}/etc/pam.d
pkgconfigdatadir=$(datadir)/pkgconfig
pkgconfiglibdir=$(libdir)/pkgconfig
polkitpolicydir=$(datadir)/polkit-1/actions
bashcompletiondir=/usr/share/bash-completion/completions
zshcompletiondir=${datarootdir}/zsh/site-functions
rpmmacrosdir=$(prefix)/lib/rpm/macros.d
sysvinitdir=$(SYSTEM_SYSVINIT_PATH)
sysvrcnddir=$(SYSTEM_SYSVRCND_PATH)
varlogdir=$(localstatedir)/log
systemdstatedir=$(localstatedir)/lib/systemd
catalogstatedir=$(systemdstatedir)/catalog
xinitrcdir=$(sysconfdir)/X11/xinit/xinitrc.d

# Our own, non-special dirs
pkgsysconfdir=$(sysconfdir)/systemd
userunitdir=$(prefix)/lib/systemd/user
userpresetdir=$(prefix)/lib/systemd/user-preset
tmpfilesdir=$(prefix)/lib/tmpfiles.d
sysusersdir=$(prefix)/lib/sysusers.d
sysctldir=$(prefix)/lib/sysctl.d
binfmtdir=$(prefix)/lib/binfmt.d
modulesloaddir=$(prefix)/lib/modules-load.d
networkdir=$(rootprefix)/lib/systemd/network
pkgincludedir=$(includedir)/systemd
systemgeneratordir=$(rootlibexecdir)/system-generators
usergeneratordir=$(prefix)/lib/systemd/user-generators
systemshutdowndir=$(rootlibexecdir)/system-shutdown
systemsleepdir=$(rootlibexecdir)/system-sleep
systemunitdir=$(rootprefix)/lib/systemd/system
systempresetdir=$(rootprefix)/lib/systemd/system-preset
udevlibexecdir=$(rootprefix)/lib/udev
udevhomedir=$(udevlibexecdir)
udevrulesdir=$(udevlibexecdir)/rules.d
udevhwdbdir=$(udevlibexecdir)/hwdb.d
catalogdir=$(prefix)/lib/systemd/catalog
kernelinstalldir = $(prefix)/lib/kernel/install.d
factory_etcdir = $(datadir)/factory/etc
factory_pamdir = $(datadir)/factory/etc/pam.d
bootlibdir = $(prefix)/lib/systemd/boot/efi

# And these are the special ones for /
rootprefix=/usr
rootbindir=$(rootprefix)/bin
rootlibexecdir=$(rootprefix)/lib/systemd

AM_DEFAULT_VERBOSITY = 0

endif
