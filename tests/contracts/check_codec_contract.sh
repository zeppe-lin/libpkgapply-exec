#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

set -eu
root=${1:?}

for path in \
  include/libpkgapply-exec/result_codec.h \
  src/execution_request.h \
  src/result_codec.cpp \
  tests/protocol/result_codec_roundtrip_test.cpp
 do
  test -f "$root/$path"
 done

grep -Fq 'encode_lifecycle_execution_result' \
  "$root/include/libpkgapply-exec/result_codec.h"
grep -Fq 'decode_lifecycle_execution_result' \
  "$root/include/libpkgapply-exec/result_codec.h"
grep -Fq 'admitted_lifecycle_session session' \
  "$root/include/libpkgapply-exec/result_codec.h"
grep -Fq 'pkgexec::decode_backend_capability_profile' "$root/src/result_codec.cpp"
grep -Fq 'pkgexec::decode_execution_result' "$root/src/result_codec.cpp"
grep -Fq 'detail::lifecycle_execution_request(session)' \
  "$root/src/result_codec.cpp"
grep -Fq 'detail::model_access::result' "$root/src/result_codec.cpp"
grep -Fq "'protocol/result_codec_roundtrip_test.cpp'" "$root/tests/meson.build"
grep -Fq "'protocol/result_codec_refusal_test.cpp'" "$root/tests/meson.build"
grep -Fq "version: ['>=2.1.0', '<3.0.0']" "$root/meson.build"
grep -Fq 'maximum_lifecycle_execution_result_encoding_size' \
  "$root/include/libpkgapply-exec/result_codec.h"
grep -Fq 'lifecycle_execution_result_encoding_version = 1' \
  "$root/include/libpkgapply-exec/result_codec.h"
grep -Fq 'execution-result encoding version 1' "$root/DESIGN.md"
! grep -Fq 'libpkgexec 1.4' "$root/DESIGN.md"

! grep -Eq 'prepare\(|execution_backend|execution_resources|filesystem::path|create_directory|chown\(' \
  "$root/src/result_codec.cpp"

! grep -Fq 'pkgexec::backend_capability_profile backend' \
  "$root/include/libpkgapply-exec/result_codec.h"
