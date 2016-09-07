#!/usr/bin/env bash

in_array() {
	local needle=$1; shift
	local item
	for item in "$@"; do
		[[ $item = $needle ]] && return 0 # Found
	done
	return 1 # Not Found
}

split_lib() {
	local d=$1

	mkdir "$d/test"
	mv "$d"/test-* -t "$d/test"

	mkdir "$d/src"
	mv "$d"/*.c -t "$d/src"

	local h=${d##*/lib}
	mkdir "$d/include"
	mkdir "$d/include/$h"
	mv "$d"/*.h -t "$d/include/$h"
}

grp() {
	local grp=$1
	shift
	mkdir "$grp"
	mv "$@" -t "$grp"
}

move_files() (
	# first focus on getting directories to the right names.
	mv -T src/{,systemd-}dbus1-generator
	mv -T src/{,systemd-}debug-generator
	mv -T src/{,systemd-}fstab-generator
	mv -T src/{,systemd-}getty-generator
	mv -T src/{,systemd-}gpt-auto-generator
	mv -T src/{,systemd-}rc-local-generator
	mv -T src/{,systemd-}system-update-generator
	mv -T src/{,systemd-}sysv-generator

	mv -T src/{,systemd-}ac-power
	mv -T src/{,systemd-}activate
	mv -T src/{,systemd-}analyze
	mv -T src/{,systemd-}ask-password
	mv -T src/{,systemd-}backlight
	mv -T src/{,systemd-}binfmt
	mv -T src/{,systemd-}cgls
	mv -T src/{,systemd-}cgroups-agent
	mv -T src/{,systemd-}cgtop
	mv -T src/{,systemd-}coredump
	mv -T src/{,systemd-}cryptsetup
	mv -T src/{,systemd-}delta
	mv -T src/{,systemd-}detect-virt
	mv -T src/{,systemd-}escape
	mv -T src/{,systemd-}firstboot
	mv -T src/{,systemd-}fsck
	mv -T src/{,systemd-}hibernate-resume
	mv -T src/{,systemd-}hwdb
	mv -T src/{,systemd-}initctl
	mv -T src/{,systemd-}machine-id-setup
	mv -T src/{,systemd-}modules-load
	mv -T src/{,systemd-}notify
	mv -T src/{,systemd-}nspawn
	mv -T src/{,systemd-}path
	mv -T src/{,systemd-}quotacheck
	mv -T src/{,systemd-}random-seed
	mv -T src/{,systemd-}remount-fs
	mv -T src/{,systemd-}reply-password
	mv -T src/{,systemd-}rfkill
	mv -T src/{,systemd-}run
	mv -T src/{,systemd-}sleep
	mv -T src/{,systemd-}stdio-bridge
	mv -T src/{,systemd-}sysctl
	mv -T src/{,systemd-}sysusers
	mv -T src/{,systemd-}tmpfiles
	mv -T src/{,systemd-}tty-ask-password-agent
	mv -T src/{,systemd-}update-done
	mv -T src/{,systemd-}update-utmp
	mv -T src/{,systemd-}user-sessions
	mv -T src/vconsole     src/systemd-vconsole-setup
	mv -T src/socket-proxy src/systemd-socket-proxyd
	mv -T src/timesync     src/systemd-timesyncd

	mv src/udev/*_id    -t src
	mv src/udev/collect -t src

	mv -T src/boot/efi src/systemd-boot
	mv -T src/boot     src/bootctl

	mkdir src/busctl
	mv src/libsystemd/src/sd-bus/busctl* -t src/busctl

	mkdir src/coredumpctl
	mv src/coredump{,ctl}/coredumpctl.c
	
	mv -T src/{,lib}basic
	mv -T src/{,lib}core
	mv -T src/{,lib}shared

	mv -T src/lib{shared,core}/linux

	mkdir src/libfirewall
	mv -T src/lib{shared,firewall}/firewall-util.c
	mv -T src/lib{shared,firewall}/firewall-util.h

	split_lib src/libbasic
	split_lib src/libshared
	split_lib src/libsystemd-network

	mkdir src/libsystemd/include
	mv -T src/{,libsystemd/include}/systemd

	# src/resolve => src/{libbasic-dns,resolve,resolved}
	mkdir src/libbasic-dns
	mv -t src/libbasic-dns \
	   src/resolve/dns-type.{c,h} \
	   src/resolve/resolved-dns-{anser,dnssec,packet,question,rr}.{c,h} \
	   src/resolve/test-*
	mkdir src/systemd-resolve
	mv -t src/systemd-resolve \
	   src/resolve/resolve-tool.c
	mkdir src/systemd-resolved
	mv -t src/systemd-resolved \
	   src/resolve/.gitignore \
	   src/resolve/*
	rmdir src/resolve

	# src/import => src/{libimport,systemd-{export,importd,import}}
	mkdir src/libimport
	mv -t src/libimport \
	   src/import/import-common.{c,h} \
	   src/import/import-compress.{c,h} \
	   src/import/qcow2-util.{c,h} \
	   src/import/test-qcow2.h
	mkdir src/systemd-export
	mv -t src/systemd-export \
	   src/import/export*
	mkdir src/systemd-importd
	mv -t src/systemd-importd \
	   src/import/.gitignore \
	   src/import/importd.c \
	   src/import/org.*
	mkdir src/systemd-import
	mv -t src/systemd-import \
	   src/import*

	# src/journal => src/..
	mkdir src/libjournal-core
	mv -t src/libjournal-core \
	   src/journal/journald-* \
	   src/journal/test-*
	mkdir src/systemd-cat
	mv -t src/systemd-cat \
	   src/journal/cat.c
	mkdir src/journalctl
	mv -t src/journalctl \
	   src/journalctl/journal-qrcode.{c,h} \
	   src/journalctl/journalctl.c
	mkdir src/journald
	mv -t src/journald \
	   src/journal/journald.*
	mkdir src/libsystemd/src/sd-journal
	mv -t src/libsystemd/src/sd-journal \
	   src/journal/audit-type.c \
	   src/journal/audit-type.h \
	   src/journal/catalog.c \
	   src/journal/catalog.h \
	   src/journal/compress.c \
	   src/journal/compress.h \
	   src/journal/fsprg.c \
	   src/journal/fsprg.h \
	   src/journal/journal-authenticate.c \
	   src/journal/journal-authenticate.h \
	   src/journal/journal-def.h \
	   src/journal/journal-file.c \
	   src/journal/journal-file.h \
	   src/journal/journal-internal.h \
	   src/journal/journal-send.c \
	   src/journal/journal-vacuum.c \
	   src/journal/journal-vacuum.h \
	   src/journal/journal-verify.c \
	   src/journal/journal-verify.h \
	   src/journal/lookup3.c \
	   src/journal/lookup3.h \
	   src/journal/mmap-cache.c \
	   src/journal/mmap-cache.h \
	   src/journal/sd-journal.c
	rmdir src/journal

	# src/network => src/...
	mkdir src/systemd-networkd-wait-online
	mv -t src/systemd-networkd-wait-online \
	   network/networkd-wait-online*
	mkdir src/libnetworkd-core
	mv -t src/libnetworkd-core \
	   network/.gitignore \
	   network/networkd-*
	mkdir src/networkctl
	mv -t src/networkctl \
	   network/networkctl.c
	mkdir src/systemd-networkd
	mv -t src/systemd-networkd \
	   network/networkd* \
	   network/org.*
	rmdir network

	# src/machine => src/{machinectl,systemd-machined}
	mkdir src/machinectl
	mv -t src/machinectl \
	   src/machine/machinectl*
	mkdir src/systemd-machined
	mv -t src/systemd-machined \
	   machine/.gitignore \
	   machine/*

	# auto-distribute the stuff
	(
		cd man
		for file in *.xml; do
			if [[ -d ../src/"${file%.xml}" ]]; then
				mv "$file" -t ../src/"${file%.xml}"
			fi
		done
	)
	(
		cd units
		for file in *; do
			if [[ -d ../src/"${file%%.*}" ]]; then
				mv "$file" -t ../src/"${file%%.*}"
			elif [[ -d ../src/"${file%%@*}" ]]; then
				mv "$file" -t ../src/"${file%%@*}"
			fi
		done
	)
	(
		cd shell-completion/bash
		for file in *; do
			if [[ -d ../src/"$file" ]]; then
				mv -T "$file" "../src/$file/$file.completion.bash"
			fi
		done
	)
	(
		cd shell-completion/zsh
		for file in _*; do
			if [[ -d ../src/"${file#_}" ]]; then
				mv -T "$file" "../src/${file#_}/${file#_}.completion.zsh"
			fi
		done
	)
	
	# categorize
	grp src/grp-boot \
	    src/bootctl \
	    src/kernel-install \
	    src/systemd-boot
	grp src/grp-coredump \
	    src/coredumpctl \
	    src/systemd-coredump
	grp src/grp-hostname \
	    src/hostnamectl \
	    src/systemd-hostnamed
	grp src/grp-initprogs \
	    src/systemd-backlight \
	    src/systemd-binfmt \
	    src/systemd-detect-virt \
	    src/systemd-firstboot \
	    src/systemd-fsck \
	    src/systemd-modules-load \
	    src/systemd-quotacheck \
	    src/systemd-random-seed \
	    src/systemd-rfkill \
	    src/systemd-sysctl \
	    src/systemd-sysusers \
	    src/systemd-tmpfiles \
	    src/systemd-update-done \
	    src/systemd-update-utmp \
	    src/systemd-user-sessions \
	    src/systemd-vconsole-setup
	grp src/grp-initprogs/grp-sleep \
	    src/systemd-hibernate-resume \
	    src/systemd-hibernate-resume-generator \
	    src/systemd-sleep
	grp src/grp-journal \
	    catalog \
	    src/journalctl \
	    src/libjournal-core \
	    src/systemd-cat \
	    src/systemd-journald
	grp src/grp-journal/grp-remote \
	    src/systemd-journal-gatewayd \
	    src/systemd-journal-remote \
	    src/systemd-journal-upload
	grp src/grp-locale \
	    src/localectl \
	    src/systemd-localed
	grp src/grp-login \
	    src/loginctl \
	    src/pam_systemd \
	    src/systemd-inhibit \
	    src/systemd-logind
	grp src/grp-machine \
	    src/machinectl \
	    src/nss-mymachines \
	    src/systemd-machined
	grp src/grp-machine/grp-import \
	    src/libimport \
	    src/systemd-export \
	    src/systemd-import \
	    src/systemd-importd \
	    src/systemd-pull
	grp src/grp-network \
	    network \
	    src/libnetworkd-core \
	    src/networkctl \
	    src/systemd-networkd \
	    src/systemd-networkd-wait-online
	grp src/grp-resolve \
	    src/libbasic-dns \
	    src/nss-resolve \
	    src/systemd-resolve \
	    src/systemd-resolved
	grp src/systemd \
	    src/libcore \
	    src/systemctl \
	    src/systemd
	grp src/systemd/grp-utils \
	    src/systemd-analyze \
	    src/systemd-delta \
	    src/systemd-fstab-generator \
	    src/systemd-run \
	    src/systemd-sysv-generator
	grp src/grp-timedate \
	    src/systemd-timedated \
	    src/timedatectl
	grp src/grp-udev \
	    rules \
	    src/*_id \
	    src/systemd-hwdb \
	    src/systemd-udevd \
	    src/systemd-udevadm
	grp src/grp-utils \
	    src/systemd-ac-power \
	    src/systemd-escape \
	    src/systemd-notify \
	    src/systemd-path \
	    src/systemd-socket-activate
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
	    -e 's/ \$\(AM_CPPFLAGS\) / $(sd.ALL_CPPFLAGS) /g' \
	    -e '/^[^#	]*:/ { s|\S+/|$(outdir)/|g }' \
	    src/libbasic/include/basic/Makefile \
	    src/libsystemd/src/Makefile \
	    src/libsystemd/src/sd-journal/Makefile \
	    src/grp-udev/libudev-core/Makefile
	find -type f -name Makefile|while read -r filename; do
		sed -r -i "s|(/\.\.)*/config.mk|/$(realpath -ms --relative-to="${filename%/*}" config.mk)|" "$filename"
	done
)

breakup_zshcompletion() (
	sed_expr='
		1 {
			i #compdef %s
			d
		}
		/^case/,/^esac/ {
			/^    %s)/,/^    ;;/ {
				s/^        //p
			}
			d
		}
	'

	cd shell-completion/zsh
	read -r _ cmds < _systemd
	for cmd in $cmds; do
		printf -v cmd_sed_expr "$sed_expr" $cmd $cmd
		sed -e "$cmd_sed_expr" < _systemd > _$cmd
	done
	rm _systemd
)

move() (
	>&2 echo ' => breakup_zshcompletion'
	breakup_zshcompletion
	>&2 echo ' => move_files'
	move_files
	>&2 echo ' => breakup_makefile'
	#breakup_makefile
	>&2 echo ' => fixup_includes'
	fixup_includes
	>&2 echo ' => fixup_makefiles'
	#fixup_makefiles
)

main() {
	set -e

	if [[ -n "$(git status -s)" ]] || [[ -n "$(git clean -xdn)" ]]; then
		echo 'There are changes in the current directory.' >&2
		exit 1
	fi

	git checkout -b tmp/postmove

	move

	git add .
	git commit -m './move.sh'
	git merge -s ours notsystemd/postmove
	git checkout notsystemd/postmove
	git merge tmp/postmove
	git branch -d tmp/postmove
}

main "$@"
