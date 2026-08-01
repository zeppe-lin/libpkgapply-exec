# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
lifecycle_test = executable('lifecycle-test', 'lifecycle_test.cpp', dependencies: libpkgapply_exec_dep)
test('lifecycle authority', lifecycle_test)

codec_test = executable(
  'result-codec-test',
  'result_codec_test.cpp',
  dependencies: libpkgapply_exec_dep,
)
test('durable lifecycle evidence codec', codec_test)

headers_test = executable('public-headers-test', 'public_headers.cpp', dependencies: libpkgapply_exec_dep)
test('public headers', headers_test)
test('authority contract', find_program('check_authority_contract.sh'), args: [meson.project_source_root()])
test('release metadata', find_program('check_release_metadata.sh'), args: [meson.project_source_root()])
