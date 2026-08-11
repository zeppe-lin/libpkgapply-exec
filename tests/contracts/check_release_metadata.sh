#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=${1:?}
version=${2:?}
fail() { echo "release-metadata-test: $*" >&2; exit 1; }
require()
{
  file=$1
  text=$2
  grep -F -- "$text" "$file" >/dev/null ||
    fail "missing in ${file#$root/}: $text"
}
require_dependency_range()
{
  variable=$1
  package=$2
  range=$3
  block=$(sed -n "/^${variable} = dependency(/,/^)/p" "$root/meson.build")
  printf '%s\n' "$block" | grep -F "  '$package'," >/dev/null ||
    fail "$variable does not name $package"
  printf '%s\n' "$block" | grep -F "version: $range" >/dev/null ||
    fail "$variable does not require $range"
}
require "$root/meson.build" "version: '$version'"
require_dependency_range libpkgapply_dep libpkgapply "['>=3.0.0', '<4.0.0']"
require_dependency_range libpkgexec_dep libpkgexec "['>=2.0.0', '<3.0.0']"
block=$(sed -n '/^public_deps = \[/,/^\]/p' "$root/src/meson.build")
for dependency in libpkgapply_dep libpkgexec_dep; do
  count=$(printf '%s\n' "$block" | grep -Fxc "  $dependency," || true)
  test "$count" -eq 1 ||
    fail "public_deps contains $count copies of $dependency, expected exactly one"
done
actual=$(printf '%s\n' "$block" | grep -Ec '^[[:space:]]+[A-Za-z0-9_]+_dep,$' || true)
test "$actual" -eq 2 || fail "public_deps contains $actual dependency objects, expected 2"
require "$root/src/meson.build" "soversion: '2'"
require "$root/src/meson.build" 'requires: public_deps,'
require "$root/README.md" "# libpkgapply-exec $version"
require "$root/HISTORY.md" "## $version —"
require "$root/HISTORY.md" 'libpkgexec 2.x'
require "$root/man/meson.build" 'libpkgapply_exec_result_codec.3'
require "$root/src/meson.build" '../include/libpkgapply-exec/result_codec.h'
