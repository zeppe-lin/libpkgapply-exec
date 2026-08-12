// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../fixtures/backend.h"
#include "../support/test.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace {
using pkgapply_exec_test::backend_mode;
using pkgapply_exec_test::fixture_backend;
using pkgapply_exec_test::node;
using pkgapply_exec_test::prepare_roots;
using pkgapply_exec_test::requests;
using pkgapply_exec_test::session;
using pkgapply_exec_test::temporary_base;

void prove_roundtrip(backend_mode mode, std::string_view suffix)
{
  const auto application = requests();
  const auto nodes = pkgapply_exec::derive(application.install);
  const auto& selected = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);
  const auto base = temporary_base(suffix);
  prepare_roots(base);
  const auto admitted = session(application.install, selected, base);
  fixture_backend backend(mode);
  const auto original = pkgapply_exec::execute(admitted, backend);
  const auto encoding = pkgapply_exec::encode_lifecycle_execution_result(original);
  const auto profile = original.execution().backend();
  fs::remove_all(base);

  const auto decoded = pkgapply_exec::decode_lifecycle_execution_result(
      encoding, admitted);
  TEST_CHECK(decoded.execution().backend() == profile);
  TEST_CHECK(!fs::exists(base));
  TEST_CHECK(decoded.identity() == original.identity());
  TEST_CHECK(decoded.node().identity() == original.node().identity());
  TEST_CHECK(decoded.execution().identity() == original.execution().identity());
  TEST_CHECK(pkgapply_exec::encode_lifecycle_execution_result(decoded) == encoding);
}
}

int main()
{
  try {
    prove_roundtrip(backend_mode::succeed, "codec-success");
    prove_roundtrip(backend_mode::fail_before_start, "codec-before");
    prove_roundtrip(backend_mode::fail_after_start, "codec-after");
    return 0;
  } catch (const std::exception& value) {
    std::cerr << value.what() << '\n';
    return 1;
  }
}
