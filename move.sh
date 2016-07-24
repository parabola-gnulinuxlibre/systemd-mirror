#!/usr/bin/env bash

in_array() {
	local needle=$1; shift
	local item
	for item in "$@"; do
		[[ $item = $needle ]] && return 0 # Found
	done
	return 1 # Not Found
}

move_files() (
	for d in libsystemd libudev; do
		mkdir src/$d-new
		mv -T src/$d src/$d-new/src
		mv -T src/$d-new src/$d
	done

	for d in basic core shared; do
		mv -T src/{,lib}$d
	done

	pfix=(
		dbus1-generator
		debug-generator
		fstab-generator
		getty-generator
		gpt-auto-generator
		rc-local-generator
		system-update-generator
		sysv-generator

		ac-power
		activate
		analyze
		ask-password
		backlight
		binfmt
		cgls
		cgroups-agent
		cgtop
		coredump
		cryptsetup
		delta
		detect-virt
		escape
		firstboot
		fsck
		hibernate-resume
		hwdb
		initctl
		machine-id-setup
		modules-load
		notify
		nspawn
		path
		quotacheck
		random-seed
		remount-fs
		reply-password
		rfkill
		run
		sleep
		socket-proxy
		stdio-bridge
		sysctl
		sysusers
		timesync
		tmpfiles
		tty-ask-password-agent
		update-done
		update-utmp
		user-sessions
	)
	for d in "${pfix[@]}"; do
		mv -T src/{,systemd-}$d
	done
	mv -T src/vconsole src/systemd-vconsole-setup

	dmon=(
		systemd-socket-proxy
		systemd-timesync
	)
	for d in "${dmon[@]}"; do
		mv -T "src/$d"{,d}
	done

	mv -T {shell-completion/bash/,src/kernel-install/bash-completion_}kernel-install
	mv -T {shell-completion/zsh/_,src/kernel-install/zsh-completion_}kernel-install
	mv -T {man,src/kernel-install}/kernel-install.xml

	mv -T src/lib{shared,core}/linux

	mkdir src/libsystemd/include
	mv -T src/{,libsystemd/include}/systemd

	mkdir src/grp-machine
	mv -T src/machine src/grp-machine/libmachine-core
	mkdir src/grp-machine/systemd-machined
	mv -T src/grp-machine/{libmachine-core,systemd-machined}/machined.c
	mkdir src/grp-machine/machinectl
	mv -T src/grp-machine/{libmachine-core,machinectl}/machinectl.c
	mv -T src/{,grp-machine}/nss-mymachines

	mkdir src/grp-resolve
	mv -T src/{resolve,grp-resolve/systemd-resolved}
	mv -T src/{,grp-resolve}/nss-resolve

	mkdir src/grp-system
	mv -T src/{,grp-system}/systemctl

	mkdir src/libfirewall
	mv -T src/lib{shared,firewall}/firewall-util.c
	mv -T src/lib{shared,firewall}/firewall-util.h

	mkdir src/grp-system/systemd
	mv -T src/{libcore,grp-system/systemd}/main.c
	mv -T src/{libcore,grp-system/systemd}/macros.systemd.in
	mv -T src/{libcore,grp-system/systemd}/org.freedesktop.systemd1.conf
	mv -T src/{libcore,grp-system/systemd}/org.freedesktop.systemd1.policy.in.in
	mv -T src/{libcore,grp-system/systemd}/org.freedesktop.systemd1.service
	mv -T src/{libcore,grp-system/systemd}/system.conf
	mv -T src/{libcore,grp-system/systemd}/systemd.pc.in
	mv -T src/{libcore,grp-system/systemd}/triggers.systemd.in
	mv -T src/{libcore,grp-system/systemd}/user.conf

	mkdir src/libudev/include
	mv -T src/libudev/{src,include}/libudev.h

	mv -T src/libsystemd/{src,}/libsystemd.pc.in
	mv -T src/libsystemd/{src,}/libsystemd.sym
	mv -T src/libsystemd/{src,}/.gitignore
	mv -T src/libsystemd/{src,libsystemd-internal}

	mkdir src/systemd-shutdown

	mkdir src/grp-coredump
	mv -T src/{,grp-coredump}/systemd-coredump
	mkdir src/grp-coredump/coredumpctl
	mv -T src/grp-coredump/{systemd-coredump,coredumpctl}/coredumpctl.c

	mkdir src/grp-boot
	mv -T src/boot/efi src/grp-boot/systemd-boot
	mv -T src/boot src/grp-boot/bootctl
	mv -T {test,src/grp-boot/systemd-boot}/test-efi-create-disk.sh

	mkdir build-aux
	mkdir build-aux/Makefile.{once,each}.{head,tail}
	touch build-aux/Makefile.{once,each}.{head,tail}/.gitignore

	mkdir src/libsystemd/libsystemd-journal-internal

	libsystemd_journal_files=(
		audit-type.c
		audit-type.h
		catalog.c
		catalog.h
		compress.c
		compress.h
		fsprg.c
		fsprg.h
		journal-authenticate.c
		journal-authenticate.h
		journal-def.h
		journal-file.c
		journal-file.h
		journal-internal.h
		journal-send.c
		journal-vacuum.c
		journal-vacuum.h
		journal-verify.c
		journal-verify.h
		lookup3.c
		lookup3.h
		mmap-cache.c
		mmap-cache.h
		sd-journal.c
	)
	for file in "${libsystemd_journal_files[@]}"; do
		mv -T src/{journal,libsystemd/libsystemd-journal-internal}/$file
	done

	mkdir src/busctl
	mv src/libsystemd/libsystemd-internal/sd-bus/busctl* src/busctl

	mv -T src/{udev,libudev/src}/udev.h

	mkdir src/grp-timedate
	mv -T src/timedate src/grp-timedate/systemd-timedated
	mkdir src/grp-timedate/timedatectl
	mv -T src/grp-timedate/{systemd-timedated,timedatectl}/timedatectl.c

	mv -T src/{libsystemd/libsystemd-internal/sd-netlink,libshared}/local-addresses.c
	mv -T src/{libsystemd/libsystemd-internal/sd-netlink,libshared}/local-addresses.h
	mv -T src/{libsystemd/libsystemd-internal/sd-netlink,libshared}/test-local-addresses.c

	mv -T src/{journal,grp-journal}
	mv -T {,src/grp-journal/}catalog
	mkdir src/grp-journal/{systemd-journald,journalctl,libjournal-core}
	mv -T src/grp-journal/{,systemd-journald}/journald.c
	mv -T src/grp-journal/{,journalctl}/journalctl.c
	mv    src/grp-journal/*.* src/grp-journal/libjournal-core/

	mv -T src/{,grp-}journal-remote
	local suffix
	for suffix in gatewayd remote upload; do
		mkdir src/grp-journal-remote/systemd-journal-$suffix
		mv src/grp-journal-remote/journal-$suffix* src/grp-journal-remote/systemd-journal-$suffix/
	done

	mv -T src/{,grp-}hostname
	mv -T src/{,grp-}locale
	mv -T src/{,grp-}udev

	mv -T src/{,grp-}import
	mkdir src/grp-import/systemd-importd
	mv src/grp-import/{,systemd-importd}/importd.c
	mkdir src/grp-import/systemd-pull
	mv src/grp-import/pull* src/grp-import/systemd-pull
	mkdir src/grp-import/systemd-import
	mv src/grp-import/import* src/grp-import/systemd-import
	mkdir src/grp-import/systemd-export
	mv src/grp-import/export* src/grp-import/systemd-export
	
	mv -T src/{,grp-}network
	mkdir src/grp-network/systemd-networkd
	mv src/grp-network/{,systemd-networkd}/networkd.c
	mkdir src/grp-network/systemd-networkd-wait-online
	mv src/grp-network/networkd-wait-online* src/grp-network/systemd-networkd-wait-online
	mkdir src/grp-network/networkctl
	mv src/grp-network/{,networkctl}/networkctl.c
	mkdir src/grp-network/libnetworkd-core
	mv src/grp-network/networkd* src/grp-network/libnetworkd-core

	mv -T src/{,grp-}login
	mkdir src/grp-login/systemd-logind
	mv -T src/grp-login/{,systemd-logind}/logind.c
	mv -T src/grp-login/{,systemd-logind}/logind.h
	mkdir src/grp-login/liblogind-core
	mv src/grp-login/logind-* src/grp-login/liblogind-core
	mkdir src/grp-login/loginctl
	mv -T src/grp-login/{,loginctl}/loginctl.c
	mv -T src/grp-login/{,loginctl}/sysfs-show.h
	mv -T src/grp-login/{,loginctl}/sysfs-show.c
	mkdir src/grp-login/systemd-inhibit
	mv -T src/grp-login/{,systemd-inhibit}/inhibit.c
	mkdir src/grp-login/pam_systemd
	mv -T src/grp-login/{,pam_systemd}/pam_systemd.c
	mv -T src/grp-login/{,pam_systemd}/pam_systemd.sym

	mkdir src/grp-udev/d-udevadm
	mv src/grp-udev/udevadm* src/grp-udev/d-udevadm/
	mkdir src/grp-udev/systemd-udevd
	mv -T src/grp-udev/{,systemd-udevd}/udevd.c
	mkdir src/grp-udev/libudev-core
	mv src/grp-udev/udev* src/grp-udev/libudev-core/
	mv -T src/grp-udev/{,libudev-core}/net
	mv -T src/grp-udev/{d-,}udevadm

	mv -T src/{libcore,systemd-shutdown}/shutdown.c
	mv -T src/{libcore,systemd-shutdown}/umount.c
	mv -T src/{libcore,systemd-shutdown}/umount.h

	mkdir src/grp-helperunits
	helperunits=(
		backlight
		binfmt
		detect-virt
		quotacheck
		random-seed
		rfkill
		sleep
		vconsole-setup
		user-sessions
	)
	for hu in "${helperunits[@]}"; do
		mv -T src/systemd-"$hu" src/grp-helperunits/systemd-"$hu"
	done
)

breakup_makefile() (
        find . \( -name Makefile -o -name '*.mk' \) -delete

        touch .tmp.move.all
        files=(.tmp.move.all)
        file=/dev/null
        IFS=''
        while read -r line; do
                if [[ $line = '#@'* ]]; then
                        file="${line#'#@'}"
                        file="${file%% *}"
                elif [[ $file = all ]]; then
                        printf '%s\n' "$line" | tee -a "${files[@]}" >/dev/null
                else
			if ! in_array "$file" "${files[@]}"; then
				cat .tmp.move.all > "$file"
				files+=("$file")
			fi
                        printf '%s\n' "$line" >> "$file"
                fi
        done < <(fixup_makefile <Makefile.am)
        rm .tmp.move.all
)

fixup_includes() (
	find src \( -name '*.h' -o -name '*.c' \) \
	     -exec grep '#include ["<]sd-' -l -- {} + |
	    xargs -d $'\n' sed -ri \
		  -e 's|#include "(sd-[^"]*)"|#include <systemd/\1>|' \
		  -e 's|#include <(sd-[^>]*)>|#include <systemd/\1>|'
)

fixup_makefile() {
	sed -r \
	    -e '/^[^#	]*:/ { s|^(\s*)\S+/|\1$(outdir)/| }' \
	    -e 's|^if (.*)|ifneq ($(\1),)|' \
	    -e 's|--version-script=.*/([^/]+)\.sym|--version-script=$(srcdir)/\1.sym|g'
}

fixup_makefiles() (
	sed -ri \
	    -e '/^	\$\(AM_V_at\)\$\(MKDIR_P\) \$\(dir \$@\)/d' \
	    -e 's/ \$\(CFLAGS\) / /g' \
	    -e 's/ \$\(CPPFLAGS\) / /g' \
	    -e 's/ \$\(AM_CPPFLAGS\) / $(ALL_CPPFLAGS) /g' \
	    -e '/^[^#	]*:/ { s|\S+/|$(outdir)/|g }' \
	    src/libbasic/Makefile \
	    src/libsystemd/libsystemd-journal-internal/Makefile \
	    src/grp-udev/libudev-core/Makefile
	find -type f -name Makefile|while read -r filename; do
		sed -r -i "s|(/\.\.)*/config.mk|/$(realpath -ms --relative-to="${filename%/*}" config.mk)|" "$filename"
	done
)

move() (
	>&2 echo ' => move_files'
	move_files
	>&2 echo ' => breakup_makefile'
	breakup_makefile
	>&2 echo ' => fixup_includes'
	fixup_includes
	>&2 echo ' => fixup_makefiles'
	fixup_makefiles
)

main() {
	set -e

	if [[ -n "$(git status -s)" ]] || [[ -n "$(git clean -xdn)" ]]; then
		echo 'There are changes in the current directory.' >&2
		exit 1
	fi

	git checkout -b postmove

	move

	git add .
	git commit -m './move.sh'
	git merge -s ours lukeshu/postmove
	git checkout lukeshu/postmove
	git merge postmove
	git branch -d postmove
}

main "$@"
