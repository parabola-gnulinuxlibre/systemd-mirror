#compdef systemd-cgtop

local curcontext="$curcontext" state lstate line
_arguments \
    {-h,--help}'[Show this help]' \
    '--version[Print version and exit]' \
    '(-c -m -i -t)-p[Order by path]' \
    '(-c -p -m -i)-t[Order by number of tasks]' \
    '(-m -p -i -t)-c[Order by CPU load]' \
    '(-c -p -i -t)-m[Order by memory load]' \
    '(-c -m -p -t)-i[Order by IO load]' \
    {-d+,--delay=}'[Specify delay]:delay:' \
    {-n+,--iterations=}'[Run for N iterations before exiting]:number of iterations:' \
    {-b,--batch}'[Run in batch mode, accepting no input]' \
    '--depth=[Maximum traversal depth]:maximum depth:'

#vim: set ft=zsh sw=4 ts=4 et
