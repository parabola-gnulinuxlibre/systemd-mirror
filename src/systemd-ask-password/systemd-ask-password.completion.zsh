#compdef systemd-ask-password

local curcontext="$curcontext" state lstate line
_arguments \
    {-h,--help}'[Show this help]' \
    '--icon=[Icon name]:icon name:' \
    '--timeout=[Timeout in sec]:timeout (seconds):' \
    '--no-tty[Ask question via agent even on TTY]' \
    '--accept-cached[Accept cached passwords]' \
    '--multiple[List multiple passwords if available]'

#vim: set ft=zsh sw=4 ts=4 et
