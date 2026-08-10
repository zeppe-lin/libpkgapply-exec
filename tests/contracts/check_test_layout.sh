#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=$1
meson=$root/tests/meson.build

for directory in contracts fixtures header installed integration protocol support unit; do
  test -d "$root/tests/$directory" || {
    echo "missing test role directory: $directory" >&2
    exit 1
  }
done

for suite in unit integration protocol header contract; do
  grep -F "suite: '$suite'" "$meson" >/dev/null || {
    echo "missing Meson test suite: $suite" >&2
    exit 1
  }
done

grep -F "test('header-' + header.underscorify(), header_test, suite: 'header')" "$meson" >/dev/null
[ -f "$root/tests/installed/consumer.cpp" ] || { echo 'missing installed consumer' >&2; exit 1; }
if grep -F "test('header:'" "$meson" >/dev/null; then
  echo 'deprecated colon remains in generated header test name' >&2
  exit 1
fi

for source in \
  integration/derivation_test.cpp \
  integration/session_admission_test.cpp \
  integration/request_projection_test.cpp \
  integration/preparation_test.cpp \
  integration/backend_contract_test.cpp \
  integration/execution_outcome_test.cpp \
  protocol/result_codec_roundtrip_test.cpp \
  protocol/result_codec_refusal_test.cpp
 do
  grep -F "'$source'" "$meson" >/dev/null || {
    echo "missing focused test registration: $source" >&2
    exit 1
  }
done

for obsolete in lifecycle_test.cpp result_codec_test.cpp public_headers.cpp fixture.h; do
  test ! -e "$root/tests/$obsolete" || {
    echo "flat omnibus test remains: $obsolete" >&2
    exit 1
  }
done
