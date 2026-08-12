#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=${1:?}
fail() { echo "abi-contract: $*" >&2; exit 1; }
manifest=$root/abi/libpkgapply-exec.exports
[ -s "$manifest" ] || fail 'reviewed ELF ABI manifest is absent'
[ "$(sed -n '/^_Z[A-Za-z0-9_]*$/p' "$manifest" | wc -l)" -eq 56 ] ||
  fail 'reviewed ELF ABI manifest must contain exactly 56 symbols'
[ "$(LC_ALL=C sort -u "$manifest" | wc -l)" -eq 56 ] ||
  fail 'reviewed ELF ABI manifest contains duplicate symbols'
! grep -F '_ZN13pkgapply_exec6detail' "$manifest" >/dev/null ||
  fail 'private detail namespace entered public ABI manifest'
grep -F "private_execution_request_test_support = static_library(" "$root/tests/meson.build" >/dev/null ||
  fail 'white-box execution-request tests lack private implementation support'
grep -F "'../src/execution_request.cpp'" "$root/tests/meson.build" >/dev/null ||
  fail 'white-box execution-request tests do not link the private request-projection leaf'
! grep -F "'../src/executor.cpp'" "$root/tests/meson.build" >/dev/null ||
  fail 'white-box execution-request tests link the non-leaf executor implementation'
grep -F "name == 'request-projection' or name == 'backend-contract'" "$root/tests/meson.build" >/dev/null ||
  fail 'private execution-request linkage is not scoped to the two white-box tests'
! grep -E '^_ZNSt|^_ZN9__gnu_cxx' "$manifest" >/dev/null ||
  fail 'standard-library implementation symbol entered public ABI manifest'
grep -F '_ZN13pkgapply_exec6deriveERKN8pkgapply27package_application_requestE' "$manifest" >/dev/null ||
  fail 'derive() is absent from reviewed ABI'
grep -F '_ZN13pkgapply_exec7executeERKNS_26admitted_lifecycle_sessionERN7pkgexec17execution_backendE' "$manifest" >/dev/null ||
  fail 'execute() is absent from reviewed ABI'
grep -F '_ZN13pkgapply_exec33decode_lifecycle_execution_result' "$manifest" >/dev/null ||
  fail 'durable decoder is absent from reviewed ABI'
grep -F 'soversion: '\''3'\''' "$root/src/meson.build" >/dev/null ||
  fail 'provider SONAME generation changed unexpectedly'
grep -F -- '--version-script=' "$root/src/meson.build" >/dev/null ||
  fail 'reviewed ELF export manifest is not linked'
grep -F "../abi/libpkgapply-exec.exports" "$root/src/meson.build" >/dev/null ||
  fail 'Meson does not consume reviewed ABI manifest'
grep -F '56-symbol' "$root/MAINTAINING.md" >/dev/null ||
  fail 'reviewed ABI inventory is undocumented'
