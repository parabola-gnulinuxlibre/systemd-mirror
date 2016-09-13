#compdef systemd-tty-ask-password-agent

local curcontext="$curcontext" state lstate line
_arguments \
    {-h,--help}'[Prints a short help text and exits.]' \
    '--version[Prints a short version string and exits.]' \
    '--list[Lists all currently pending system password requests.]' \
    '--query[Process all currently pending system password requests by querying the user on the calling TTY.]' \
    '--watch[Continuously process password requests.]' \
    '--wall[Forward password requests to wall(1).]' \
    '--plymouth[Ask question with plymouth(8).]' \
    '--console[Ask question on /dev/console.]'

#vim: set ft=zsh sw=4 ts=4 et
