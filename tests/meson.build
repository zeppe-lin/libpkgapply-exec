# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
fixture_test_deps = [
  libpkgapply_exec_dep,
  dependency('libpkgbuild-image', version: ['>=1.0.0', '<2.0.0'], required: true, static: want_static),
  dependency('libpkgbuild-plan', version: ['>=1.0.0', '<2.0.0'], required: true, static: want_static),
  dependency('libpkgcatalog', version: ['>=3.0.0', '<4.0.0'], required: true, static: want_static),
  dependency('libpkgresolve', version: ['>=2.0.0', '<3.0.0'], required: true, static: want_static),
  dependency('libpkgstate', version: ['>=3.0.0', '<4.0.0'], required: true, static: want_static),
]

lifecycle_test = executable('lifecycle-test', 'lifecycle_test.cpp', dependencies: fixture_test_deps)
test('lifecycle authority', lifecycle_test)

codec_test = executable(
  'result-codec-test',
  'result_codec_test.cpp',
  dependencies: fixture_test_deps,
)
test('durable lifecycle evidence codec', codec_test)

headers_test = executable('public-headers-test', 'public_headers.cpp', dependencies: libpkgapply_exec_dep)
test('public headers', headers_test)
test('authority contract', find_program('check_authority_contract.sh'), args: [meson.project_source_root()])
test('release metadata', find_program('check_release_metadata.sh'), args: [meson.project_source_root(), meson.project_version()])

test(
  'codec contract',
  find_program('check_codec_contract.sh'),
  args: [meson.project_source_root()],
)

test(
  'Meson source paths',
  find_program('check_meson_sources.sh'),
  args: [meson.project_source_root()],
)
