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

ifneq ($(ENABLE_EFI),)
ifneq ($(HAVE_GNUEFI),)
efi_cppflags = \
	$(EFI_CPPFLAGS) \
	-I$(top_builddir) -include config.h \
	-I$(EFI_INC_DIR)/efi \
	-I$(EFI_INC_DIR)/efi/$(EFI_ARCH) \
	-DEFI_MACHINE_TYPE_NAME=\"$(EFI_MACHINE_TYPE_NAME)\"

efi_cflags = \
	$(EFI_CFLAGS) \
	-Wall \
	-Wextra \
	-std=gnu90 \
	-nostdinc \
	-ggdb -O0 \
	-fpic \
	-fshort-wchar \
	-nostdinc \
	-ffreestanding \
	-fno-strict-aliasing \
	-fno-stack-protector \
	-Wsign-compare \
	-Wno-missing-field-initializers

ifneq ($(ARCH_X86_64),)
efi_cflags += \
	-mno-red-zone \
	-mno-sse \
	-mno-mmx \
	-DEFI_FUNCTION_WRAPPER \
	-DGNU_EFI_USE_MS_ABI
endif # ARCH_X86_64

ifneq ($(ARCH_IA32),)
efi_cflags += \
	-mno-sse \
	-mno-mmx
endif # ARCH_IA32

efi_ldflags = \
	$(EFI_LDFLAGS) \
	-T $(EFI_LDS_DIR)/elf_$(EFI_ARCH)_efi.lds \
	-shared \
	-Bsymbolic \
	-nostdlib \
	-znocombreloc \
	-L $(EFI_LIB_DIR) \
	$(EFI_LDS_DIR)/crt0-efi-$(EFI_ARCH).o

# Aarch64 and ARM32 don't have an EFI capable objcopy. Use 'binary' instead,
# and add required symbols manually.
ifneq ($(ARCH_AARCH64),)
efi_ldflags += --defsym=EFI_SUBSYSTEM=0xa
EFI_FORMAT = -O binary
else # ARCH_AARCH64
EFI_FORMAT = --target=efi-app-$(EFI_ARCH)
endif # ARCH_AARCH64
endif # HAVE_GNUEFI
endif # ENABLE_EFI

# ------------------------------------------------------------------------------
systemd_boot_headers = \
	src/boot/efi/util.h \
	src/boot/efi/console.h \
	src/boot/efi/graphics.h \
	src/boot/efi/pefile.h \
	src/boot/efi/measure.h \
	src/boot/efi/disk.h

systemd_boot_sources = \
	src/boot/efi/util.c \
	src/boot/efi/console.c \
	src/boot/efi/graphics.c \
	src/boot/efi/pefile.c \
	src/boot/efi/disk.c \
	src/boot/efi/measure.c \
	src/boot/efi/boot.c

EXTRA_DIST += $(systemd_boot_sources) $(systemd_boot_headers)

systemd_boot_objects = $(addprefix $(top_builddir)/,$(systemd_boot_sources:.c=.o))
systemd_boot_solib = $(top_builddir)/src/boot/efi/systemd_boot.so
systemd_boot = systemd-boot$(EFI_MACHINE_TYPE_NAME).efi

ifneq ($(ENABLE_EFI),)
ifneq ($(HAVE_GNUEFI),)
bootlib_DATA = $(systemd_boot)

$(outdir)/%.o: $(top_srcdir)/src/boot/efi/%.c $(addprefix $(top_srcdir)/,$(systemd_boot_headers))
	@$(MKDIR_P) $(top_builddir)/src/boot/efi/
	$(AM_V_CC)$(EFI_CC) $(efi_cppflags) $(efi_cflags) -c $< -o $@

$(systemd_boot_solib): $(systemd_boot_objects)
	$(AM_V_CCLD)$(LD) $(efi_ldflags) $(systemd_boot_objects) \
		-o $@ -lefi -lgnuefi $(shell $(CC) -print-libgcc-file-name); \
	nm -D -u $@ | grep ' U ' && exit 1 || :

$(systemd_boot): $(systemd_boot_solib)
	$(AM_V_GEN)$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic \
	  -j .dynsym -j .rel -j .rela -j .reloc $(EFI_FORMAT) $< $@
endif # HAVE_GNUEFI
endif # ENABLE_EFI

CLEANFILES += $(systemd_boot_objects) $(systemd_boot_solib) $(systemd_boot)

# ------------------------------------------------------------------------------
stub_headers = \
	src/boot/efi/util.h \
	src/boot/efi/pefile.h \
	src/boot/efi/disk.h \
	src/boot/efi/graphics.h \
	src/boot/efi/splash.h \
	src/boot/efi/measure.h \
	src/boot/efi/linux.h

stub_sources = \
	src/boot/efi/util.c \
	src/boot/efi/pefile.c \
	src/boot/efi/disk.c \
	src/boot/efi/graphics.c \
	src/boot/efi/splash.c \
	src/boot/efi/linux.c \
	src/boot/efi/measure.c \
	src/boot/efi/stub.c

EXTRA_DIST += \
	$(stub_sources) \
	$(stub_headers) \
	test/splash.bmp

stub_objects = $(addprefix $(top_builddir)/,$(stub_sources:.c=.o))
stub_solib = $(top_builddir)/src/boot/efi/stub.so
stub = linux$(EFI_MACHINE_TYPE_NAME).efi.stub

ifneq ($(ENABLE_EFI),)
ifneq ($(HAVE_GNUEFI),)
bootlib_DATA += $(stub)

$(outdir)/%.o: $(top_srcdir)/src/boot/efi/%.c $(addprefix $(top_srcdir)/,$(stub_headers))
	@$(MKDIR_P) $(top_builddir)/src/boot/efi/
	$(AM_V_CC)$(EFI_CC) $(efi_cppflags) $(efi_cflags) -c $< -o $@

$(stub_solib): $(stub_objects)
	$(AM_V_CCLD)$(LD) $(efi_ldflags) $(stub_objects) \
		-o $@ -lefi -lgnuefi $(shell $(CC) -print-libgcc-file-name); \
	nm -D -u $@ | grep ' U ' && exit 1 || :

$(stub): $(stub_solib)
	$(AM_V_GEN)$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic \
	  -j .dynsym -j .rel -j .rela -j .reloc $(EFI_FORMAT) $< $@
endif # HAVE_GNUEFI
endif # ENABLE_EFI

CLEANFILES += $(stub_objects) $(stub_solib) $(stub)

# ------------------------------------------------------------------------------
CLEANFILES += test-efi-disk.img

test-efi-disk.img: $(systemd_boot) $(stub) test/test-efi-create-disk.sh
	$(AM_V_GEN)test/test-efi-create-disk.sh

test-efi: test-efi-disk.img
	$(QEMU) -machine accel=kvm -m 1024 -bios $(QEMU_BIOS) -snapshot test-efi-disk.img

EXTRA_DIST += test/test-efi-create-disk.sh

include $(topsrcdir)/build-aux/Makefile.tail.mk
