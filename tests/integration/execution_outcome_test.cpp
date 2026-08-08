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

pkgapply_exec::lifecycle_execution_result run(backend_mode mode,
                                              std::string_view suffix)
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
  auto result = pkgapply_exec::execute(admitted, backend);
  fs::remove_all(base);
  return result;
}

void prove_success_and_failure_are_retained_exactly()
{
  const auto success = run(backend_mode::succeed, "success");
  TEST_CHECK(success.succeeded());
  TEST_CHECK(success.execution().status() == pkgexec::execution_status::succeeded);
  TEST_CHECK(success.execution().established_guarantees() ==
             success.execution().request().required_guarantees());
  TEST_CHECK(success.identity().hex().size() == 64U);

  const auto before = run(backend_mode::fail_before_start, "before-start");
  TEST_CHECK(!before.succeeded());
  TEST_CHECK(before.execution().status() == pkgexec::execution_status::failed);
  TEST_CHECK(before.execution().start_state() ==
             pkgexec::execution_start_state::not_started);
  TEST_CHECK(before.execution().failure() ==
             pkgexec::execution_failure_kind::backend_unsupported);

  const auto after = run(backend_mode::fail_after_start, "after-start");
  TEST_CHECK(!after.succeeded());
  TEST_CHECK(after.execution().start_state() ==
             pkgexec::execution_start_state::started);
  TEST_CHECK(after.execution().failure() ==
             pkgexec::execution_failure_kind::program_exited_nonzero);
  TEST_CHECK(after.execution().termination()->kind() ==
             pkgexec::process_termination_kind::exited);
}
}

int main()
{
  try {
    prove_success_and_failure_are_retained_exactly();
    return 0;
  } catch (const std::exception& value) {
    std::cerr << value.what() << '\n';
    return 1;
  }
}
