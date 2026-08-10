#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=${1:?}
workflow=$root/.github/workflows/ci.yml
fail() { echo "ci-contract: $*" >&2; exit 1; }
for pair in \
  'libpkgsource v3.0.1' \
  'libpkgstate v3.1.0' \
  'libpkgcatalog v3.0.1' \
  'libpkgplan v0.3.1' \
  'libpkgsource-plan v1.1.0' \
  'libpkgapply v3.0.0' \
  'libpkgexec v1.4.0'
do
  set -- $pair
  repository=$1
  ref=$2
  [ "$(grep -F "repository: zeppe-lin/$repository, ref: $ref" "$workflow" | wc -l)" -eq 2 ] ||
    fail "$repository is not pinned to $ref in both hosted matrices"
done
! grep -F 'repository: zeppe-lin/libpkgapply, ref: v2.' "$workflow" >/dev/null ||
  fail 'obsolete libpkgapply generation remains in hosted qualification'
grep -F 'meson install -C "$build/product"' "$root/ci/configure-and-test.sh" >/dev/null ||
  fail 'product is not staged before installed-consumer qualification'
grep -F 'audit-shared-boundary.sh' "$root/ci/configure-and-test.sh" >/dev/null ||
  fail 'shared core SONAME boundary is not audited after staging'
grep -F 'pkg-config --static --libs libpkgapply-exec' "$root/ci/configure-and-test.sh" >/dev/null ||
  fail 'static installed pkg-config closure is not qualified'
grep -F 'pkgapply_exec::derive(application.install)' "$root/tests/installed/consumer.cpp" >/dev/null ||
  fail 'installed consumer does not execute lifecycle derivation'
if grep -E '&[[:space:]]*pkgapply_exec::|decltype\(&pkgapply_exec::' "$root/tests/installed/consumer.cpp" >/dev/null; then
  fail 'installed consumer regressed to function-address qualification'
fi
