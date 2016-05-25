#!/usr/bin/env bash

in_array() {
	local needle=$1; shift
	local item
	for item in "$@"; do
		[[ $item = $needle ]] && return 0 # Found
	done
	return 1 # Not Found
}

(
        set -e
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
        done < Makefile.am
        rm .tmp.move.all
)
