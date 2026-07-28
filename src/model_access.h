// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <libpkgapply-exec/model.h>
namespace pkgapply_exec::detail {
class model_access final {
public:
  static lifecycle_node_identity node_identity(std::string hex);
  static lifecycle_node_set_identity set_identity(std::string hex);
  static lifecycle_execution_result_identity result_identity(std::string hex);
  static lifecycle_node node(
      pkgapply::application_request_identity request,
      pkgplan::operation_plan_identity plan,
      pkgplan::operation_kind operation,
      pkgapply::application_target_context_identity target,
      pkgapply::lifecycle_executor_identity executor,
      lifecycle_subject subject,
      pkgplan::package_release release,
      pkgsource::lifecycle_action action,
      pkgsource::program program,
      std::optional<pkgsource::source_snapshot_identity> source,
      std::optional<pkgplan::installed_control_identity> installed_control);
  static lifecycle_node_set set(
      pkgapply::application_request_identity request,
      std::vector<lifecycle_node> nodes);
  static lifecycle_execution_result result(
      lifecycle_node node, pkgexec::execution_result execution);
};
class executor_access final {
public:
  static lifecycle_execution_result make(
      lifecycle_node node, pkgexec::execution_result execution);
};
} // namespace pkgapply_exec::detail
