# systemd-mirror

<!--
  Copyright 2023  Luke T. Shumaker <lukeshu@parabola.nu>
  SPDX-License-Identifier: AGPL-3.0-or-later
  -->

This repository is a consolidated mirror of systemd and its various
downstreams.  This aims to solve the problem that it's very easy to
loose track of which fork a given tag came from.

Sources that we mirror:

 - `systemd` itself (of course), and `udev`: https://github.com/systemd/systemd.git
 - `sytemd-stable`: https://github.com/systemd/systemd-stable.git
 - `elogind`: https://github.com/elogind/elogind.git
 - `eudev`: https://github.com/gentoo/eudev.git
 - (TODO) `notsystemd`
 - (TODO) `systemd-parabola`: systemd plus whatever patches
   [Arch](https://archlinux.org) and [Parabola](https://parabola.nu) ship.

Refs in this repository:
 - `refs/heads/main`: This `main` branch that contains this README.md and the scripts for
   updating the other refs.
 - `refs/heads/${repo}/*`: Branches synced from each of the
   repositories listed above.
 - `refs/tags/${project}/*`: Tags synced from each of the repositories
   listed above; `${project}` is mostly the same as the repo name, but
   in the case of systemd.git, a tag may be classified into the
   `systemd` project or the `udev` project.
