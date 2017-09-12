#compdef systemd-cgls

local curcontext="$curcontext" state lstate line
_arguments \
    {-h,--help}'[Show this help]' \
    '--version[Show package version]' \
    '--no-pager[Do not pipe output into a pager]' \
    {-a,--all}'[Show all groups, including empty]' \
    '-k[Include kernel threads in output]' \
    ':cgroups:(cpuset cpu cpuacct memory devices freezer blkio)'

#vim: set ft=zsh sw=4 ts=4 et
