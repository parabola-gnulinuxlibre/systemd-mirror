# systemd-resolve(1) completion                             -*- shell-script -*-
#
# This file is part of systemd.
#
# Copyright 2016 Zbigniew Jędrzejewski-Szmek
#
# systemd is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2.1 of the License, or
# (at your option) any later version.
#
# systemd is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with systemd; If not, see <http://www.gnu.org/licenses/>.

__contains_word () {
        local w word=$1; shift
        for w in "$@"; do
                [[ $w = "$word" ]] && return
        done
}

__get_interfaces(){
        { cd /sys/class/net && echo *; } | \
        while read -d' ' -r name; do
                [[ "$name" != "lo" ]] && echo "$name"
        done
}

_systemd-resolve() {
        local i comps
        local cur=${COMP_WORDS[COMP_CWORD]} prev=${COMP_WORDS[COMP_CWORD-1]}
        local -A OPTS=(
               [STANDALONE]='-h --help --version -4 -6
                             --service --openpgp --tlsa --status --statistics
                             --reset-statistics --service-address=no --service-txt=no
                             --cname=no --search=no --legend=no'
                      [ARG]='-i --interface -p --protocol -t --type -c --class'
        )

        if __contains_word "$prev" ${OPTS[ARG]}; then
                case $prev in
                        --interface|-i)
                                comps=$( __get_interfaces )
                                ;;
                        --protocol|-p|--type|-t|--class|-c)
                                comps=$( systemd-resolve --legend=no "$prev" help; echo help )
                                ;;
                esac
                COMPREPLY=( $(compgen -W '$comps' -- "$cur") )
                return 0
        fi

        if [[ "$cur" = -* ]]; then
                COMPREPLY=( $(compgen -W '${OPTS[*]}' -- "$cur") )
                return 0
        fi
}

complete -F _systemd-resolve systemd-resolve
