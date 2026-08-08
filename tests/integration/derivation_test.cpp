// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../fixtures/lifecycle.h"
#include "../support/test.h"

#include <iostream>

namespace {
using pkgapply_exec_test::application_fixture::historical_control;
using pkgapply_exec_test::application_fixture::planning_authorities;
using pkgapply_exec_test::application_fixture::removal_with_control;
using pkgapply_exec_test::node;
using pkgapply_exec_test::requests;
using pkgapply_exec_test::target;

void prove_install_upgrade_remove_sets()
{
  const auto application = requests();
  const auto install = pkgapply_exec::derive(application.install);
  const auto upgrade = pkgapply_exec::derive(application.upgrade);
  const auto remove = pkgapply_exec::derive(application.remove);

  TEST_CHECK(install.nodes().size() == 2U);
  TEST_CHECK(upgrade.nodes().size() == 4U);
  TEST_CHECK(remove.nodes().size() == 2U);

  const auto& install_pre = node(
      install, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);
  TEST_CHECK(install_pre.program().material() == "echo pre-install\n");
  TEST_CHECK(install_pre.source().has_value());
  TEST_CHECK(!install_pre.installed_control().has_value());

  const auto& upgrade_remove = node(
      upgrade, pkgapply_exec::lifecycle_subject::installed,
      pkgsource::lifecycle_action::pre_remove);
  TEST_CHECK(upgrade_remove.program().material() == "echo old-pre-remove\n");
  TEST_CHECK(!upgrade_remove.source().has_value());
  TEST_CHECK(upgrade_remove.installed_control().has_value());

  TEST_CHECK(install.find(pkgapply_exec::lifecycle_subject::incoming,
                          pkgsource::lifecycle_action::pre_remove) == nullptr);
  TEST_CHECK(upgrade.find(pkgapply_exec::lifecycle_subject::incoming,
                          pkgsource::lifecycle_action::pre_remove) == nullptr);
  TEST_CHECK(remove.find(pkgapply_exec::lifecycle_subject::installed,
                         pkgsource::lifecycle_action::pre_install) == nullptr);

  TEST_CHECK(std::is_sorted(install.nodes().begin(), install.nodes().end()));
  TEST_CHECK(std::is_sorted(upgrade.nodes().begin(), upgrade.nodes().end()));
}

void prove_derivation_refusals()
{
  auto without_executor = requests(false);
  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::missing_lifecycle_executor, [&] {
        (void)pkgapply_exec::derive(without_executor.install);
      });

  auto context = target(true);
  planning_authorities authorities(context.target());
  auto unavailable = pkgapply::package_application_request(
      pkgapply::removal_application_request::make(
          removal_with_control(
              authorities,
              pkgplan::installed_control_projection::historically_unavailable()),
          context, pkgapply_exec_test::control()));
  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::historical_control_unavailable, [&] {
        (void)pkgapply_exec::derive(unavailable);
      });

  context = target(true);
  planning_authorities foreign_authorities(context.target());
  auto foreign = pkgapply::package_application_request(
      pkgapply::removal_application_request::make(
          removal_with_control(
              foreign_authorities,
              historical_control("application/x-foreign")),
          context, pkgapply_exec_test::control()));
  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::unsupported_program_format, [&] {
        (void)pkgapply_exec::derive(foreign);
      });
}
}

int main()
{
  try {
    prove_install_upgrade_remove_sets();
    prove_derivation_refusals();
    return 0;
  } catch (const std::exception& value) {
    std::cerr << value.what() << '\n';
    return 1;
  }
}
