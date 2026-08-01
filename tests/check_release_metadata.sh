#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

set -eu
root=${1:?}
version=${2:?}

grep -Fq "version: '$version'" "$root/meson.build"
grep -Fq "# libpkgapply-exec $version" "$root/README.md"
grep -Eq "^## $version — [0-9]{4}-[0-9]{2}-[0-9]{2}$" "$root/HISTORY.md"
grep -Fq "soversion: '1'" "$root/src/meson.build"
grep -Fq "libpkgapply >= 2.0.0" "$root/src/meson.build"
grep -Fq "libpkgexec >= 1.4.0" "$root/src/meson.build"
grep -Fq 'libpkgapply_exec_result_codec.3' "$root/man/meson.build"
grep -Fq '../include/libpkgapply-exec/result_codec.h' \
  "$root/src/meson.build"
