#!/usr/bin/env bash
set -e
git checkout lukeshu/premove
git branch -D postmove || true
git checkout .
git clean -xdf
