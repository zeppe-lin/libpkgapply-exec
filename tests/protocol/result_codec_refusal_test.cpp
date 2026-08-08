// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../fixtures/backend.h"
#include "../support/test.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace {
using pkgapply_exec_test::backend_capabilities;
using pkgapply_exec_test::backend_mode;
using pkgapply_exec_test::fixture_backend;
using pkgapply_exec_test::node;
using pkgapply_exec_test::prepare_roots;
using pkgapply_exec_test::requests;
using pkgapply_exec_test::session;
using pkgapply_exec_test::temporary_base;

bool refused(const pkgapply_exec::lifecycle_execution_result_encoding& bytes,
             pkgapply_exec::admitted_lifecycle_session admitted,
             pkgexec::backend_capability_profile backend,
             pkgapply_exec::error_code expected)
{
  try {
    (void)pkgapply_exec::decode_lifecycle_execution_result(
        bytes, std::move(admitted), std::move(backend));
  } catch (const pkgapply_exec::error& value) {
    return value.code() == expected;
  }
  return false;
}

void prove_corruption_and_authority_refusal()
{
  const auto application = requests();
  const auto nodes = pkgapply_exec::derive(application.install);
  const auto& pre = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);
  const auto& post = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::post_install);
  const auto base = temporary_base("codec-refusal");
  prepare_roots(base);
  const auto admitted = session(application.install, pre, base);
  fixture_backend backend(backend_mode::succeed);
  const auto result = pkgapply_exec::execute(admitted, backend);
  const auto encoding = pkgapply_exec::encode_lifecycle_execution_result(result);
  const auto profile = result.execution().backend();
  fs::remove_all(base);

  auto corrupt = encoding;
  corrupt[corrupt.size() / 2U] ^= 0x01U;
  TEST_CHECK(refused(corrupt, admitted, profile,
                     pkgapply_exec::error_code::corrupt_encoding));

  auto truncated = encoding;
  truncated.pop_back();
  TEST_CHECK(refused(truncated, admitted, profile,
                     pkgapply_exec::error_code::corrupt_encoding));

  auto trailing = encoding;
  trailing.push_back(0U);
  TEST_CHECK(refused(trailing, admitted, profile,
                     pkgapply_exec::error_code::corrupt_encoding));

  const auto foreign_node = session(
      application.install, post, temporary_base("codec-foreign-node"));
  TEST_CHECK(refused(encoding, foreign_node, profile,
                     pkgapply_exec::error_code::authority_mismatch));

  const auto foreign_execution = session(
      application.install, pre, temporary_base("codec-foreign-execution"), 9, 3);
  TEST_CHECK(refused(encoding, foreign_execution, profile,
                     pkgapply_exec::error_code::authority_mismatch));

  TEST_CHECK(refused(encoding, admitted, backend_capabilities('b'),
                     pkgapply_exec::error_code::authority_mismatch));
}
}

int main()
{
  try {
    prove_corruption_and_authority_refusal();
    return 0;
  } catch (const std::exception& value) {
    std::cerr << value.what() << '\n';
    return 1;
  }
}
