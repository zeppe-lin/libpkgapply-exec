// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "execution_request.h"
#include <string>
#include <utility>

namespace pkgapply_exec {
namespace {

pkgexec::resource_identity resource_identity(
    std::string_view domain,
    std::string_view node) {
  std::string material(domain);
  material.push_back('\0');
  material.append(node);
  return pkgexec::resource_identity::from_sha256(
      pkgexec::sha256_digest::of_bytes(material).hex());
}

std::string action_text(pkgsource::lifecycle_action action) {
  return std::string(pkgsource::to_string(action));
}

} // namespace

namespace detail {

pkgexec::resource_identity managed_target_resource_identity(
    const lifecycle_node& node) {
  return resource_identity(
      "pkgapply-exec/managed-target-root/v1", node.identity().hex());
}

pkgexec::resource_identity temporary_resource_identity(
    const lifecycle_node& node) {
  return resource_identity(
      "pkgapply-exec/private-temporary-root/v1", node.identity().hex());
}

pkgexec::execution_request lifecycle_execution_request(
    const admitted_lifecycle_session& session) {
  const auto target_slot =
      pkgexec::resource_slot::singleton(pkgexec::resource_role::managed_target_root);
  const auto target_identity = managed_target_resource_identity(session.node());
  const auto temporary_slot =
      pkgexec::resource_slot::singleton(pkgexec::resource_role::private_temporary_root);
  const auto temporary_identity = temporary_resource_identity(session.node());

  std::vector<pkgexec::resource_binding> bindings;
  bindings.emplace_back(
      target_slot,
      target_identity,
      pkgexec::resource_access::writable,
      pkgexec::logical_path::parse("/target"));
  bindings.emplace_back(
      temporary_slot,
      temporary_identity,
      pkgexec::resource_access::writable,
      pkgexec::logical_path::parse("/tmp"));
  auto layout = pkgexec::resource_layout::seal(std::move(bindings), target_slot);

  const auto& release = session.node().release();
  std::vector<pkgexec::environment_variable> variables;
  variables.emplace_back("ZEPPE_LIN_TARGET_ROOT", "/target");
  variables.emplace_back("ZEPPE_LIN_PACKAGE_NAME", release.name());
  variables.emplace_back("ZEPPE_LIN_PACKAGE_VERSION", release.version());
  variables.emplace_back("ZEPPE_LIN_PACKAGE_RELEASE", release.release());
  variables.emplace_back(
      "ZEPPE_LIN_LIFECYCLE_ACTION", action_text(session.node().action()));
  variables.emplace_back(
      "ZEPPE_LIN_LIFECYCLE_SUBJECT",
      std::string(to_string(session.node().subject())));

  auto environment = pkgexec::environment_policy::hermetic(
      {pkgexec::logical_path::parse("/usr/bin"),
       pkgexec::logical_path::parse("/bin")},
      pkgexec::logical_path::parse("/tmp/home"),
      pkgexec::logical_path::parse("/tmp"),
      1,
      0022,
      std::nullopt,
      pkgexec::network_policy::denied,
      pkgexec::stdin_policy::closed,
      pkgexec::stream_policy::capture_complete,
      pkgexec::stream_policy::capture_complete,
      std::move(variables));

  return pkgexec::execution_request::seal(
      session.node().program(),
      pkgexec::execution_purpose::lifecycle(session.node().action()),
      session.execution_identity().interpreter,
      session.paths().execution_root,
      std::move(layout),
      std::move(environment),
      pkgexec::credential_policy::fixed(
          session.execution_identity().user_id,
          session.execution_identity().group_id,
          session.execution_identity().supplementary_groups,
          true),
      pkgexec::resource_limits::make(),
      pkgexec::cancellation_policy::disabled());
}

} // namespace detail
} // namespace pkgapply_exec
