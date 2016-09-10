#!/usr/bin/env bash

for lib in basic shared systemd-network systemd; do
        pushd src/lib${lib}/include/${lib}
        find . -type f -exec sed -ri -e "s|$lib/||" -- {} +
        popd
done

find src \( -name '*.h' -o -name '*.c' \) -type f -exec ./fixup_includes {} \;
