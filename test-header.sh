#!/bin/bash
gcc -c -o /dev/null -I/usr/include/efi -I/usr/include/efi/x86_64 -I/usr/include/libmount $(find src -type d |sed 's|^|-I&|') -include ./config.h -include "$(realpath -- "$1")" test-header.c
