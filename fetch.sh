#!/usr/bin/env bash
# Copyright 2023  Luke T. Shumaker <lukeshu@parabola.nu>
# SPDX-License-Identifier: AGPL-3.0-or-later

# I used to manually maintain a horribly complicated .git/config to
# get `git fetch` to do the right things.  Now, instead of doing that,
# I use this more robust script instead of using `git fetch` directly.

set -euE

declare -A urls=(
	[systemd]=https://github.com/systemd/systemd.git
	[systemd-stable]=https://github.com/systemd/systemd-stable.git
	[elogind]=https://github.com/elogind/elogind.git
	[eudev]=https://github.com/gentoo/eudev.git
)

# tmpdir=$(mktemp -d)
# trap "rm -rf -- ${tmpdir@Q}" EXIT
# pushd "$tmpdir" >/dev/null

# 0: List remote tags ##########################################################

libremessages msg 'Listing remote tags...'

for name in "${!urls[@]}"; do
	git ls-remote --tags --refs "${urls[$name]}" >"$name.0.txt"
done

# 1: Filter out the ones we don't want to fetch ################################

libremessages msg 'Remapping tags...'

cp systemd.{0,1}.txt

cat systemd-stable.0.txt |
	# This repo contains several duplicates within itself:
	#  - udev-{XXX} -> {XXX}
	#  - sytemd-v{XXX} -> v{XXX}
	#  - show -> v1
	# Filter those out.
	grep -vFx -f <(<systemd-stable.0.txt sed -nE -e 's,(.*)refs/tags/v1$,\1refs/tags/show\n\1refs/tags/systemd-v1,p' -e 's,refs/tags/v,refs/tags/systemd-v,p' -e 's,refs/tags/([01]),refs/tags/udev-\1,p') |
	# Filter out tags copied from systemd.git
	grep -vFx -f systemd.0.txt |
	# Done.
	cat >systemd-stable.1.txt

cat elogind.0.txt |
	# Filter out tags copied from systemd.git and systemd-stable.git
	grep -vFx -f systemd.0.txt |
	# Done.
	cat >elogind.1.txt

cat eudev.0.txt |
	# Filter out the renamed tags from systemd.git
	grep -vFx -f <(<systemd.0.txt sed -nE -e 's,refs/tags/v,refs/tags/systemd-v,p' -e 's,refs/tags/([01]),refs/tags/udev-\1,p') |
	# Prune out other tags that exist as both 'vXXX' and just 'XXX'.
	grep -vFx -f <(<eudev.0.txt sed -n 's,^refs/tags/v,refs/tags/,p') |
	# 'v3.1.4' and '3.1.4' both point at the same commit, but
	# 'v3.1.4' is an annotated tag while '3.1.4' is a lightweight
	# tag.
	grep -vFx -e $'305f0eef4de67c09598271caf1d19c5436cd40d7\trefs/tags/3.1.4' |
	# Done.
	cat >eudev.1.txt

# 2: Rename them ###############################################################

# There are two tag formats coming from systemd.git:
#  - vXXX[.Y]: systemd versions
#  - XXX: (001-182) udev versions, from before the
#    systemd/udev merge
<systemd.1.txt awk '{print $2}' |
	sed -E \
	    -e 's,refs/tags/v,refs/tags/systemd/v,' \
	    -e 's,refs/tags/([01]),refs/tags/udev/v\1,' \
	    >systemd.2.txt

<systemd-stable.1.txt awk '{print $2}' |
	sed -E \
	    -e 's,refs/tags/,refs/tags/systemd-stable/,' \
	    >systemd-stable.2.txt

<elogind.1.txt awk '{print $2}' |
	sed -E \
	    -e 's,refs/tags/,refs/tags/elogind/,' \
	    >elogind.2.txt

<eudev.1.txt awk '{print $2}' |
	sed -E \
	    -e 's,refs/tags/v,refs/tags/eudev/v,' \
	    -e 's,refs/tags/([01]),refs/tags/eudev/v\1,' \
	    >eudev.2.txt

# Sanity check; make sure that all tags got renamed.
if cat *.2.txt|grep -vE 'refs/tags/(udev|systemd|systemd-stable|elogind|eudev)/v'; then
	libremessages error 'Failed to rename some tags appropriately'
	exit 1
fi

# 3: Turn those into refspecs ##################################################

for name in "${!urls[@]}"; do
	{
		echo "+refs/heads/*:refs/heads/${name}/*"
		paste --delimiter=: <(<"$name.1.txt" awk '{print "+" $2}') "$name.2.txt"
	} >"$name.3.txt"
done

# 3: Actually fetch ############################################################

exit 0

popd >/dev/null
libremessages msg 'Actually fetching...'

for name in "${!urls[@]}"; do
	libremessages msg2 '%s -> %s' "${urls[$name]}" "r-${name}"
	tag_globs=()
	case "$name" in
		systemd) tag_globs=('refs/tags/systemd/*' 'refs/tags/udev/*');;
		*) tag_globs=("refs/tags/$name/*");;
	esac
	git for-each-ref --format='%(refname)' "${tag_globs[@]}" | grep -vFx -f "$tmpdir/$name.2.txt" | sed 's/^/delete /' | tee /dev/stderr | git update-ref --stdin
	xargs -d $'\n' -a "$tmpdir/$name.3.txt" git fetch --prune --no-tags -- "${urls[$name]}"
done
