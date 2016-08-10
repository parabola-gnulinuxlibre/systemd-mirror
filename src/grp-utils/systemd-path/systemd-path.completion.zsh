#compdef systemd-path

__get_names() {
        systemd-path | { while IFS=: read -r a b; do echo " $a"; done; }
}

__names() {
        local -a _names
        _names=(${(fo)"$(__get_names)"})
        typeset -U _names
        _describe 'names' _names
}

_arguments \
        {-h,--help}'[Show help message]' \
        '--version[Show package version]' \
        '--host=[Sufix to append to paths]:suffix' \
	'*:name:__names'
