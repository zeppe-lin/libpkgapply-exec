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
  tests/fixtures/application.h \
  tests/fixtures/lifecycle.h \
  tests/fixtures/backend.h \
  tests/support/test.h \
  tests/unit/error_value_test.cpp \
  tests/integration/derivation_test.cpp \
  tests/integration/session_admission_test.cpp \
  tests/integration/request_projection_test.cpp \
  tests/integration/preparation_test.cpp \
  tests/integration/backend_contract_test.cpp \
  tests/integration/execution_outcome_test.cpp \
  tests/protocol/result_codec_roundtrip_test.cpp \
  tests/protocol/result_codec_refusal_test.cpp \
  tests/header/public_header_test.cpp \
  tests/contracts/check_authority_contract.sh \
  tests/contracts/check_codec_contract.sh \
  tests/contracts/check_meson_sources.sh \
  tests/contracts/check_release_metadata.sh \
  tests/contracts/check_test_layout.sh \
  ci/qualify.sh \
  man/libpkgapply-exec.7.scdoc \
  man/libpkgapply_exec_result_codec.3.scdoc
 do
  test -f "$root/$path" || {
    echo "missing Meson input: $path" >&2
    exit 1
  }
 done
