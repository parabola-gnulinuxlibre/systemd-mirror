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
# ------------------------------------------------------------------------------
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
else
EFI_FORMAT = --target=efi-app-$(EFI_ARCH)
endif # ARCH_AARCH64

$(outdir)/%.o: $(srcdir)/%.c
	$(AM_V_EFI_CC)$(EFI_CC) $(efi_cppflags) $(efi_cflags) -c $< -o $@

$(outdir)/%.so:
	@if test $(words $^) = 0; then echo 'Cannot link EFI library with no dependencies: $@' >&2; exit 1; fi
	$(AM_V_EFI_CCLD)$(LD) $(efi_ldflags) $^ -o $@ -lefi -lgnuefi $(shell $(CC) -print-libgcc-file-name)
	$(AM_V_at)! { nm -D -u $@ | grep ' U '; }

# These next 2 are the same rule
$(outdir)/%$(EFI_MACHINE_TYPE_NAME).efi     : $(outdir)/%.so; $(AM_V_GEN)$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc $(EFI_FORMAT) $< $@
$(outdir)/%$(EFI_MACHINE_TYPE_NAME).efi.stub: $(outdir)/%.so; $(AM_V_GEN)$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc $(EFI_FORMAT) $< $@
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

systemd_boot = systemd-boot$(EFI_MACHINE_TYPE_NAME).efi
bootlib_DATA += $(systemd_boot)
$(outdir)/systemd-boot.so: $(addprefix $(outdir)/,$(notdir $(systemd_boot_sources:.c=.o)))
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

stub = linux$(EFI_MACHINE_TYPE_NAME).efi.stub
bootlib_DATA += $(stub)
$(outdir)/linux.so: $(addprefix $(outdir)/,$(notdir $(stub_sources:.c=.o)))
# ------------------------------------------------------------------------------
$(outdir)/test-efi-disk.img: $(outdir)/$(systemd_boot) $(outdir)/$(stub) $(srcdir)/test-efi-create-disk.sh
	$(AM_V_GEN)$(@D)/test-efi-create-disk.sh

test-efi: $(outdir)/test-efi-disk.img
	$(QEMU) -machine accel=kvm -m 1024 -bios $(QEMU_BIOS) -snapshot test-efi-disk.img

files.out.int += test-efi-disk.img
# ------------------------------------------------------------------------------
endif # HAVE_GNUEFI
endif # ENABLE_EFI
include $(topsrcdir)/build-aux/Makefile.tail.mk
