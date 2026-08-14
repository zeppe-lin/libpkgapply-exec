#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=${1:?}
workflow=$root/.github/workflows/ci.yml
fail() { echo "ci-contract: $*" >&2; exit 1; }
for expected in \
  'zeppe-lin/libpkgsource, ref: v4.1.0' \
  'zeppe-lin/libpkgstate, ref: f74df278b47b48e798c3de01c922c59b58319d13' \
  'zeppe-lin/libpkgimage, ref: 284324996dce673e1a96d73f8adb90b29dbb79f5' \
  'zeppe-lin/libpkgcatalog, ref: v4.0.0' \
  'zeppe-lin/libpkgresolve, ref: v4.0.0' \
  'zeppe-lin/libpkgbuild, ref: v3.0.1' \
  'zeppe-lin/libpkgplan, ref: 2c6e5394749ffa0ad76fbdd1918d5a6793c5d0ec' \
  'zeppe-lin/libpkgbuild-image, ref: v1.0.1' \
  'zeppe-lin/libpkgsource-plan, ref: v2.0.0' \
  'zeppe-lin/libpkgbuild-plan, ref: v1.1.0' \
  'zeppe-lin/libpkgapply, ref: v3.0.1' \
  'zeppe-lin/libpkgexec, ref: v2.1.1'
do
  count=$(grep -F "$expected" "$workflow" | wc -l)
  test "$count" -eq 2 || fail "current authority checkout is not pinned in both hosted matrices: $expected"
done
! grep -F 'repository: zeppe-lin/libpkgresolve, ref: v2.0.0' "$workflow" >/dev/null ||
  fail 'obsolete resolver2 checkout remains in hosted qualification'
! grep -F 'repository: zeppe-lin/libpkgexec, ref: v1.' "$workflow" >/dev/null ||
  fail 'obsolete exec1 checkout remains in hosted qualification'
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
