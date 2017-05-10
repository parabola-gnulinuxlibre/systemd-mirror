#compdef systemd-cat

local curcontext="$curcontext" state lstate line
_arguments \
    {-h,--help}'[Show this help]' \
    '--version[Show package version.]' \
    {-t+,--identifier=}'[Set syslog identifier.]:syslog identifier:' \
    {-p+,--priority=}'[Set priority value.]:value:({0..7})' \
    '--level-prefix=[Control whether level prefix shall be parsed.]:boolean:(1 0)' \
    ':Message'

#vim: set ft=zsh sw=4 ts=4 et
