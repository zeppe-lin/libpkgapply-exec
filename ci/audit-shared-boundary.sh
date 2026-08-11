#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu

[ "$#" -eq 1 ] || {
  echo "usage: $0 INSTALLED-LIBRARY" >&2
  exit 2
}
library=$1
[ -s "$library" ] || {
  echo "shared-boundary-audit: missing library: $library" >&2
  exit 1
}

output=$(readelf -d "$library")
printf '%s\n' "$output"
printf '%s\n' "$output" | grep -F \
  'Library soname: [libpkgapply-exec.so.2]' >/dev/null || {
  echo 'shared-boundary-audit: wrong provider SONAME' >&2
  exit 1
}
needed=$(printf '%s\n' "$output" | grep 'Shared library:' || true)
printf '%s\n' "$needed" | grep -F \
  'Shared library: [libpkgapply.so.3]' >/dev/null || {
  echo 'shared-boundary-audit: generation-3 semantic core is absent' >&2
  exit 1
}
printf '%s\n' "$needed" | grep -F \
  'Shared library: [libpkgexec.so.2]' >/dev/null || {
  echo 'shared-boundary-audit: execution core is absent' >&2
  exit 1
}
if printf '%s\n' "$needed" | grep -E 'libpkgexec\.so\.[01]' >/dev/null; then
  echo 'shared-boundary-audit: obsolete execution-core SONAME remains' >&2
  exit 1
fi
if printf '%s\n' "$needed" | grep -F 'libpkgapply.so.2' >/dev/null; then
  echo 'shared-boundary-audit: obsolete semantic-core SONAME remains' >&2
  exit 1
fi
