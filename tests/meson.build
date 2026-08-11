# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

private_execution_request_test_support = static_library(
  'pkgapply-exec-test-execution-request',
  '../src/execution_request.cpp',
  include_directories: public_inc,
  dependencies: public_deps,
)

fixture_test_deps = [
  libpkgapply_exec_dep,
  dependency('libpkgbuild-image', version: ['>=1.0.0', '<2.0.0'], required: true, static: want_static),
  dependency('libpkgbuild-plan', version: ['>=1.0.0', '<2.0.0'], required: true, static: want_static),
  dependency('libpkgcatalog', version: ['>=3.0.0', '<4.0.0'], required: true, static: want_static),
  dependency('libpkgresolve', version: ['>=3.0.0', '<4.0.0'], required: true, static: want_static),
  dependency('libpkgstate', version: ['>=3.0.0', '<4.0.0'], required: true, static: want_static),
]

unit_tests = {
  'error-value': 'unit/error_value_test.cpp',
}

integration_tests = {
  'derivation': 'integration/derivation_test.cpp',
  'session-admission': 'integration/session_admission_test.cpp',
  'request-projection': 'integration/request_projection_test.cpp',
  'preparation': 'integration/preparation_test.cpp',
  'backend-contract': 'integration/backend_contract_test.cpp',
  'execution-outcome': 'integration/execution_outcome_test.cpp',
}

protocol_tests = {
  'result-codec-roundtrip': 'protocol/result_codec_roundtrip_test.cpp',
  'result-codec-refusal': 'protocol/result_codec_refusal_test.cpp',
}

foreach name, source : unit_tests
  executable_test = executable(
    name + '-test',
    source,
    dependencies: libpkgapply_exec_dep,
  )
  test(name, executable_test, suite: 'unit')
endforeach

foreach name, source : integration_tests
  private_link = []
  if name == 'request-projection' or name == 'backend-contract'
    private_link += [private_execution_request_test_support]
  endif
  executable_test = executable(
    name + '-test',
    source,
    dependencies: fixture_test_deps,
    link_with: private_link,
  )
  test(name, executable_test, suite: 'integration')
endforeach

foreach name, source : protocol_tests
  executable_test = executable(
    name + '-test',
    source,
    dependencies: fixture_test_deps,
  )
  test(name, executable_test, suite: 'protocol')
endforeach

public_headers = [
  'error.h',
  'model.h',
  'derive.h',
  'executor.h',
  'result_codec.h',
  'libpkgapply-exec.h',
]
foreach header : public_headers
  header_test = executable(
    'header-' + header.underscorify() + '-test',
    'header/public_header_test.cpp',
    cpp_args: [
      '-DPKGAPPLY_EXEC_TEST_HEADER=<libpkgapply-exec/' + header + '>',
    ],
    dependencies: libpkgapply_exec_dep,
  )
  test('header-' + header.underscorify(), header_test, suite: 'header')
endforeach

contract_tests = {
  'authority-contract': 'contracts/check_authority_contract.sh',
  'codec-contract': 'contracts/check_codec_contract.sh',
  'release-metadata': 'contracts/check_release_metadata.sh',
  'meson-sources': 'contracts/check_meson_sources.sh',
  'test-layout': 'contracts/check_test_layout.sh',
  'abi-contract': 'contracts/check_abi_contract.sh',
  'ci-contract': 'contracts/check_ci_contract.sh',
}
foreach name, program : contract_tests
  test(
    name,
    find_program(program),
    args: [meson.project_source_root(), meson.project_version()],
    suite: 'contract',
  )
endforeach

pkgconfig_metadata = find_program('contracts/check_pkgconfig_metadata.sh')
test(
  'pkgconfig-metadata',
  pkgconfig_metadata,
  args: [meson.project_build_root()],
  suite: 'contract',
)

abi_layout = executable(
  'abi-layout-test',
  'contracts/abi_layout_test.cpp',
  dependencies: libpkgapply_exec_dep,
)
test('abi-layout', abi_layout, suite: 'contract')

if library_kind == 'shared'
  abi_surface = find_program('contracts/check_abi_surface.sh')
  test(
    'abi-surface',
    abi_surface,
    args: [libpkgapply_exec, meson.project_source_root() / 'abi/libpkgapply-exec.exports'],
    suite: 'contract',
  )

  exec_abi_generation = find_program('contracts/check_exec_abi_generation.sh')
  test(
    'exec-abi-generation',
    exec_abi_generation,
    args: [libpkgapply_exec],
    suite: 'contract',
  )
endif
