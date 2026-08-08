// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../fixtures/lifecycle.h"
#include "../support/test.h"

#include <iostream>
#include <limits>

#include <sys/types.h>

namespace {
using pkgapply_exec_test::execution_identity;
using pkgapply_exec_test::node;
using pkgapply_exec_test::paths;
using pkgapply_exec_test::requests;
using pkgapply_exec_test::temporary_base;

void prove_exact_binding_and_canonical_credentials()
{
  const auto application = requests();
  const auto install = pkgapply_exec::derive(application.install);
  const auto& selected = node(
      install, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);
  const auto base = temporary_base("session");
  auto identity = execution_identity();
  auto admitted = pkgapply_exec::admitted_lifecycle_session::admit(
      application.install, selected, paths(application.install, base), identity);
  TEST_CHECK(admitted.request().identity() == application.install.identity());
  TEST_CHECK(admitted.node().identity() == selected.identity());
  TEST_CHECK(admitted.execution_identity().supplementary_groups ==
             std::vector<std::uint64_t>({66, 77}));
}

void prove_foreign_node_and_target_are_refused()
{
  const auto application = requests();
  const auto install = pkgapply_exec::derive(application.install);
  const auto upgrade = pkgapply_exec::derive(application.upgrade);
  const auto& install_pre = node(
      install, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);
  const auto& foreign = node(
      upgrade, pkgapply_exec::lifecycle_subject::installed,
      pkgsource::lifecycle_action::pre_remove);
  const auto base = temporary_base("binding");

  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::request_node_mismatch, [&] {
        (void)pkgapply_exec::admitted_lifecycle_session::admit(
            application.install, foreign, paths(application.install, base),
            execution_identity());
      });

  auto wrong_root = paths(application.install, base);
  wrong_root.target_root = pkgapply_exec_test::apply_identity<
      pkgapply::root_view_identity>(55);
  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::target_binding_mismatch, [&] {
        (void)pkgapply_exec::admitted_lifecycle_session::admit(
            application.install, install_pre, wrong_root,
            execution_identity());
      });
}

void prove_effect_coordinates_are_disjoint_and_normal()
{
  const auto application = requests();
  const auto nodes = pkgapply_exec::derive(application.install);
  const auto& selected = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);
  const auto base = temporary_base("coordinates");

  auto overlap = paths(application.install, base);
  overlap.session_root = base / "target" / "session";
  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::invalid_effect_coordinate, [&] {
        (void)pkgapply_exec::admitted_lifecycle_session::admit(
            application.install, selected, overlap, execution_identity());
      });

  auto relative = paths(application.install, base);
  relative.session_root = "relative/session";
  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::invalid_effect_coordinate, [&] {
        (void)pkgapply_exec::admitted_lifecycle_session::admit(
            application.install, selected, relative, execution_identity());
      });
}

void prove_numeric_identity_refusals()
{
  const auto application = requests();
  const auto nodes = pkgapply_exec::derive(application.install);
  const auto& selected = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);
  const auto base = temporary_base("identity");

  auto invalid = execution_identity();
  invalid.user_id = static_cast<std::uint64_t>(std::numeric_limits<uid_t>::max());
  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::invalid_execution_identity, [&] {
        (void)pkgapply_exec::admitted_lifecycle_session::admit(
            application.install, selected, paths(application.install, base),
            invalid);
      });

  auto duplicate = execution_identity();
  duplicate.supplementary_groups = {66, 66};
  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::invalid_execution_identity, [&] {
        (void)pkgapply_exec::admitted_lifecycle_session::admit(
            application.install, selected, paths(application.install, base),
            duplicate);
      });

  auto primary = execution_identity();
  primary.supplementary_groups = {primary.group_id};
  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::invalid_execution_identity, [&] {
        (void)pkgapply_exec::admitted_lifecycle_session::admit(
            application.install, selected, paths(application.install, base),
            primary);
      });
}
}

int main()
{
  try {
    prove_exact_binding_and_canonical_credentials();
    prove_foreign_node_and_target_are_refused();
    prove_effect_coordinates_are_disjoint_and_normal();
    prove_numeric_identity_refusals();
    return 0;
  } catch (const std::exception& value) {
    std::cerr << value.what() << '\n';
    return 1;
  }
}
