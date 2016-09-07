#!/usr/bin/env bash
set -e
git checkout notsystemd/premove
git branch -D tmp/postmove || true
git checkout .
git clean -xdf
