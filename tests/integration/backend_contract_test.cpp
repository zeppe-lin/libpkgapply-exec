// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../fixtures/backend.h"
#include "../support/test.h"
#include "../../src/execution_request.h"

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

void prove_capability_failure_precedes_mutation()
{
  const auto application = requests();
  const auto nodes = pkgapply_exec::derive(application.install);
  const auto& selected = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);

  for (const auto mode : {
           backend_mode::throw_capabilities,
           backend_mode::throw_capabilities_nonstandard,
       }) {
    const auto base = temporary_base("capability-failure") /
        std::to_string(static_cast<int>(mode));
    prepare_roots(base);
    const auto admitted = session(application.install, selected, base);
    fixture_backend backend(mode);
    pkgapply_exec_test::expect_error(
        pkgapply_exec::error_code::backend_contract_violation, [&] {
          (void)pkgapply_exec::execute(admitted, backend);
        });
    TEST_CHECK(!fs::exists(base / "session"));
    fs::remove_all(base);
  }
}

void prove_execute_contract_refusals()
{
  const auto application = requests();
  const auto nodes = pkgapply_exec::derive(application.install);
  const auto& selected = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);

  for (const auto mode : {
           backend_mode::throw_execute,
           backend_mode::throw_execute_nonstandard,
           backend_mode::wrong_backend,
       }) {
    const auto base = temporary_base("backend-refusal") /
        std::to_string(static_cast<int>(mode));
    prepare_roots(base);
    const auto admitted = session(application.install, selected, base);
    fixture_backend backend(mode);
    pkgapply_exec_test::expect_error(
        pkgapply_exec::error_code::backend_contract_violation, [&] {
          (void)pkgapply_exec::execute(admitted, backend);
        });
    fs::remove_all(base);
  }
}

void prove_foreign_request_evidence_is_refused()
{
  const auto application = requests();
  const auto nodes = pkgapply_exec::derive(application.install);
  const auto& pre = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);
  const auto& post = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::post_install);

  const auto base = temporary_base("foreign-request");
  prepare_roots(base);
  const auto admitted = session(application.install, pre, base);

  const auto foreign_base = temporary_base("foreign-request-other");
  prepare_roots(foreign_base);
  const auto foreign_session = session(application.install, post, foreign_base);
  const auto foreign_request =
      pkgapply_exec::detail::lifecycle_execution_request(foreign_session);
  fs::remove_all(foreign_base);

  fixture_backend backend(backend_mode::wrong_request, foreign_request);
  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::backend_contract_violation, [&] {
        (void)pkgapply_exec::execute(admitted, backend);
      });
  fs::remove_all(base);
}
}

int main()
{
  try {
    prove_capability_failure_precedes_mutation();
    prove_execute_contract_refusals();
    prove_foreign_request_evidence_is_refused();
    return 0;
  } catch (const std::exception& value) {
    std::cerr << value.what() << '\n';
    return 1;
  }
}
