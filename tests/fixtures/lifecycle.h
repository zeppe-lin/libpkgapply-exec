// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "application.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <sys/types.h>
#include <unistd.h>

#include <libpkgapply-exec/libpkgapply-exec.h>

namespace pkgapply_exec_test {
namespace fs = std::filesystem;
namespace application_fixture = pkgapply::test::fixture;

template<class Identity>
Identity apply_identity(std::uint8_t value)
{
  std::string text = "v1:sha256:";
  constexpr char digits[] = "0123456789abcdef";
  for (std::size_t index = 0; index < 32U; ++index) {
    const auto current = static_cast<std::uint8_t>(value + index);
    text.push_back(digits[current >> 4U]);
    text.push_back(digits[current & 0x0fU]);
  }
  return Identity::parse(text);
}

template<class Identity>
Identity exec_identity(std::uint8_t value)
{
  return Identity::from_sha256(
      std::string(64U, "0123456789abcdef"[value & 0x0fU]));
}

inline pkgplan::target_system_context_identity target_identity(std::uint8_t value)
{
  std::array<std::uint8_t, 32> bytes{};
  bytes.fill(value);
  return pkgplan::target_system_context_identity::from_sha256(bytes);
}

inline pkgapply::application_target_context target(bool with_executor = true)
{
  return pkgapply::application_target_context::make(
      target_identity(7),
      apply_identity<pkgapply::managed_target_identity>(2),
      apply_identity<pkgapply::root_view_identity>(3),
      apply_identity<pkgapply::observation_backend_identity>(4),
      apply_identity<pkgapply::mutation_backend_identity>(5),
      apply_identity<pkgapply::mutation_exclusion_domain_identity>(6),
      apply_identity<pkgapply::active_object_namespace_identity>(7),
      apply_identity<pkgapply::rejected_object_store_identity>(8),
      apply_identity<pkgapply::staging_namespace_identity>(9),
      apply_identity<pkgapply::journal_namespace_identity>(10),
      apply_identity<pkgapply::execution_capability_profile_identity>(11),
      with_executor
          ? std::optional<pkgapply::lifecycle_executor_identity>(
                apply_identity<pkgapply::lifecycle_executor_identity>(12))
          : std::nullopt);
}

inline pkgapply::application_execution_control control()
{
  return pkgapply::application_execution_control::make(
      pkgapply::application_recovery_requirement::exact_prior_state,
      pkgapply::application_durability_requirement::all_application_domains,
      pkgapply::application_cancellation_policy::recover_after_target_mutation,
      4096, 8192);
}

struct application_requests final {
  pkgapply::package_application_request install;
  pkgapply::package_application_request upgrade;
  pkgapply::package_application_request remove;
};

inline application_requests requests(bool with_executor = true)
{
  auto context = target(with_executor);
  application_fixture::planning_authorities authorities(context.target());
  return {
      pkgapply::package_application_request(
          pkgapply::installation_application_request::make(
              application_fixture::ordinary_installation(authorities),
              application_fixture::ordinary_installation_incoming(), context,
              control())),
      pkgapply::package_application_request(
          pkgapply::upgrade_application_request::make(
              application_fixture::ordinary_upgrade(authorities),
              application_fixture::ordinary_upgrade_incoming(), context,
              control())),
      pkgapply::package_application_request(
          pkgapply::removal_application_request::make(
              application_fixture::ordinary_removal(authorities), context,
              control()))};
}

inline const pkgapply_exec::lifecycle_node& node(
    const pkgapply_exec::lifecycle_node_set& nodes,
    pkgapply_exec::lifecycle_subject subject,
    pkgsource::lifecycle_action action)
{
  const auto* value = nodes.find(subject, action);
  if (value == nullptr)
    throw std::runtime_error("fixture lifecycle node is missing");
  return *value;
}

inline fs::path temporary_base(std::string_view name)
{
  return fs::temp_directory_path() /
      fs::path(std::string("libpkgapply-exec-") + std::string(name) + "-" +
               std::to_string(::getpid()));
}

inline void prepare_roots(const fs::path& base)
{
  fs::remove_all(base);
  fs::create_directories(base / "exec");
  fs::create_directories(base / "target");
}

inline pkgapply_exec::lifecycle_session_paths paths(
    const pkgapply::package_application_request& application,
    const fs::path& base,
    std::uint8_t execution_root_seed = 2)
{
  return {
      exec_identity<pkgexec::root_view_identity>(execution_root_seed),
      base / "exec", application.target().root_view(), base / "target",
      base / "session"};
}

inline pkgapply_exec::lifecycle_execution_identity execution_identity(
    std::uint8_t interpreter_seed = 3)
{
  return {
      exec_identity<pkgexec::interpreter_identity>(interpreter_seed),
      static_cast<std::uint64_t>(::geteuid()),
      static_cast<std::uint64_t>(::getegid()), {77, 66}};
}

inline pkgapply_exec::admitted_lifecycle_session session(
    const pkgapply::package_application_request& application,
    const pkgapply_exec::lifecycle_node& selected,
    const fs::path& base,
    std::uint8_t execution_root_seed = 2,
    std::uint8_t interpreter_seed = 3)
{
  return pkgapply_exec::admitted_lifecycle_session::admit(
      application, selected, paths(application, base, execution_root_seed),
      execution_identity(interpreter_seed));
}

} // namespace pkgapply_exec_test
