#compdef systemd-detect-virt

local curcontext="$curcontext" state lstate line
_arguments \
    {-h,--help}'[Show this help]' \
    '--version[Show package version]' \
    {-c,--container}'[Only detect whether we are run in a container]' \
    {-v,--vm}'[Only detect whether we are run in a VM]' \
    {-q,--quiet}"[Don't output anything, just set return value]"

#vim: set ft=zsh sw=4 ts=4 et
