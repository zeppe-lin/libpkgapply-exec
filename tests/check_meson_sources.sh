#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

set -eu
root=${1:?}

for path in \
  include/libpkgapply-exec/error.h \
  include/libpkgapply-exec/model.h \
  include/libpkgapply-exec/derive.h \
  include/libpkgapply-exec/executor.h \
  include/libpkgapply-exec/result_codec.h \
  include/libpkgapply-exec/libpkgapply-exec.h \
  src/error.cpp \
  src/model.cpp \
  src/derive.cpp \
  src/executor.cpp \
  src/execution_request.h \
  src/result_codec.cpp \
  tests/lifecycle_test.cpp \
  tests/result_codec_test.cpp \
  tests/public_headers.cpp \
  tests/check_authority_contract.sh \
  tests/check_codec_contract.sh \
  tests/check_meson_sources.sh \
  tests/check_release_metadata.sh \
  man/libpkgapply-exec.7.scdoc \
  man/libpkgapply_exec_result_codec.3.scdoc
 do
  test -f "$root/$path" || {
    echo "missing Meson input: $path" >&2
    exit 1
  }
 done
