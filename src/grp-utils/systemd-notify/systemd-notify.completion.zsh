#compdef systemd-notify

local curcontext="$curcontext" state lstate line
_arguments \
    {-h,--help}'[Show this help]' \
    '--version[Show package version]' \
    '--ready[Inform the init system about service start-up completion.]' \
    '--pid=[Inform the init system about the main PID of the daemon]:daemon main PID:_pids' \
    '--status=[Send a free-form status string for the daemon to the init systemd]:status string:' \
    '--booted[Returns 0 if the system was booted up with systemd]'

#vim: set ft=zsh sw=4 ts=4 et
