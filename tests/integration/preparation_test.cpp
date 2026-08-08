// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../fixtures/lifecycle.h"
#include "../support/test.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

namespace fs = std::filesystem;

namespace {
using pkgapply_exec_test::node;
using pkgapply_exec_test::prepare_roots;
using pkgapply_exec_test::requests;
using pkgapply_exec_test::session;
using pkgapply_exec_test::temporary_base;

mode_t mode(const fs::path& path)
{
  struct stat value{};
  if (::stat(path.c_str(), &value) != 0)
    throw std::runtime_error("cannot stat fixture path");
  return static_cast<mode_t>(value.st_mode & 0777U);
}

void prove_preparation_materializes_exact_resources()
{
  const auto application = requests();
  const auto nodes = pkgapply_exec::derive(application.install);
  const auto& selected = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);
  const auto base = temporary_base("prepare");
  prepare_roots(base);
  const auto admitted = session(application.install, selected, base);
  const auto prepared = pkgapply_exec::prepare(admitted);

  TEST_CHECK(prepared.temporary_root == base / "session/tmp");
  TEST_CHECK(fs::is_directory(base / "session"));
  TEST_CHECK(fs::is_directory(base / "session/tmp"));
  TEST_CHECK(fs::is_directory(base / "session/tmp/home"));
  TEST_CHECK(mode(base / "session") == 0700U);
  TEST_CHECK(mode(base / "session/tmp") == 0700U);
  TEST_CHECK(mode(base / "session/tmp/home") == 0700U);

  const auto target_slot = pkgexec::resource_slot::singleton(
      pkgexec::resource_role::managed_target_root);
  const auto temporary_slot = pkgexec::resource_slot::singleton(
      pkgexec::resource_role::private_temporary_root);
  TEST_CHECK(prepared.resources.materialization(
                 prepared.request.resources().binding(target_slot).resource())
                 .host_path() == base / "target");
  TEST_CHECK(prepared.resources.materialization(
                 prepared.request.resources().binding(temporary_slot).resource())
                 .host_path() == base / "session/tmp");
  fs::remove_all(base);
}

void prove_invalid_roots_and_session_state_are_refused()
{
  const auto application = requests();
  const auto nodes = pkgapply_exec::derive(application.install);
  const auto& selected = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);

  auto base = temporary_base("missing-root");
  fs::remove_all(base);
  fs::create_directories(base / "target");
  auto admitted = session(application.install, selected, base);
  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::resource_preparation_failed, [&] {
        (void)pkgapply_exec::prepare(admitted);
      });
  TEST_CHECK(!fs::exists(base / "session"));

  base = temporary_base("nonempty-session");
  prepare_roots(base);
  fs::create_directories(base / "session");
  std::ofstream(base / "session/occupied") << "x";
  admitted = session(application.install, selected, base);
  pkgapply_exec_test::expect_error(
      pkgapply_exec::error_code::resource_preparation_failed, [&] {
        (void)pkgapply_exec::prepare(admitted);
      });
  fs::remove_all(base);
}
}

int main()
{
  try {
    prove_preparation_materializes_exact_resources();
    prove_invalid_roots_and_session_state_are_refused();
    return 0;
  } catch (const std::exception& value) {
    std::cerr << value.what() << '\n';
    return 1;
  }
}
