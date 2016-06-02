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
	for d in libsystemd libudev machine resolve; do
		mkdir src/$d-new
		mv -T src/$d src/$d-new/src
		mv -T src/$d-new src/$d
	done

	for d in basic core shared; do
		mv -T src/{,lib}$d
	done

	pfix=(
		activate
		analyze
		ask-password
		backlight
		binfmt
		bootchart
		cgls
		cgroups-agent
		cgtop
		cryptsetup
		delta
		escape
		notify
		nspawn
		path
		quotacheck
		random-seed
		remount-fs
		reply-password
		rfkill
		run
		timedate
		timesync
		tmpfiles
		tty-ask-password-agent
		update-done
		update-utmp
		user-sessions
		vconsole
	)
	for d in "${pfix[@]}"; do
		mv -T src/{,systemd-}$d
	done


	mv -T {,src/journal/}catalog

	mv -T {shell-completion/bash/,src/kernel-install/bash-completion_}kernel-install
	mv -T {shell-completion/zsh/_,src/kernel-install/zsh-completion_}kernel-install
	mv -T {man,src/kernel-install}/kernel-install.xml

	mv -T src/lib{shared,core}/linux

	mv -T src/{,libsystemd/}/compat-libs
	mkdir src/libsystemd/include
	mv -T src/{,libsystemd/include}/systemd

	mv -T src/{,machine}/nss-mymachines
	mv -T src/{,resolve}/nss-resolve

	mkdir src/system
	mv -T src/{,system}/systemctl

	mkdir src/libfirewall
	mv -T src/lib{shared,firewall}/firewall-util.c
	mv -T src/lib{shared,firewall}/firewall-util.h

	mkdir src/system/systemd
	mv -T src/{libcore,system/systemd}/main.c
	mv -T src/{libcore,system/systemd}/macros.systemd.in
	mv -T src/{libcore,system/systemd}/org.freedesktop.systemd1.conf
	mv -T src/{libcore,system/systemd}/org.freedesktop.systemd1.policy.in.in
	mv -T src/{libcore,system/systemd}/org.freedesktop.systemd1.service
	mv -T src/{libcore,system/systemd}/system.conf
	mv -T src/{libcore,system/systemd}/systemd.pc.in
	mv -T src/{libcore,system/systemd}/triggers.systemd.in
	mv -T src/{libcore,system/systemd}/user.conf

	mkdir src/libudev/include
	mv -T src/libudev/{src,include}/libudev.h

	mv -T {man,src/systemd-activate}/systemd-activate.xml

	mv -T src/libsystemd/{src,}/libsystemd.pc.in
	mv -T src/libsystemd/{src,}/libsystemd.sym
	mv -T src/libsystemd/{src,}/.gitignore
	mv -T src/libsystemd/{src,libsystemd-internal}

	mkdir src/systemd-shutdown

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
	     -exec grep '#include "sd-' -l -- {} + |
	    xargs -d $'\n' sed -ri 's|#include "(sd-[^"]*)"|#include <systemd/\1>|'
)

fixup_makefile() {
	sed -r \
	    -e '/^[^#	]*:/ { s|^(\s*)\S+/|\1$(outdir)/| }' \
	    -e 's|^if (.*)|ifneq ($(\1),)|' \
	    -e 's|rootprefix|prefix|g' \
	    -e 's|rootbin|bin|g' \
	    -e 's|rootlib|lib|g'
}

fixup_makefiles() (
	sed -ri \
	    -e '/^	\$\(AM_V_at\)\$\(MKDIR_P\) \$\(dir \$@\)/d' \
	    -e 's/ \$\(CFLAGS\) / /g' \
	    -e 's/ \$\(CPPFLAGS\) / /g' \
	    -e 's/ \$\(AM_CPPFLAGS\) / $(ALL_CPPFLAGS) /g' \
	    -e '/^[^#	]*:/ { s|\S+/|$(outdir)/|g }' \
	    src/libbasic/Makefile
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
