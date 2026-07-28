// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <libpkgapply/request.h>
#include <libpkgexec/libpkgexec.h>
namespace pkgapply_exec {
namespace detail { class model_access; class executor_access; }
inline constexpr std::uint16_t lifecycle_node_schema_version = 1;
inline constexpr std::uint16_t lifecycle_node_set_schema_version = 1;
inline constexpr std::uint16_t lifecycle_execution_result_schema_version = 1;

enum class lifecycle_subject : std::uint8_t { incoming = 1, installed = 2 };
[[nodiscard]] std::string_view to_string(lifecycle_subject value) noexcept;

class lifecycle_node_identity final {
public:
  [[nodiscard]] const std::string& hex() const noexcept;
  friend bool operator==(const lifecycle_node_identity&, const lifecycle_node_identity&) noexcept;
  friend bool operator!=(const lifecycle_node_identity&, const lifecycle_node_identity&) noexcept;
  friend bool operator<(const lifecycle_node_identity&, const lifecycle_node_identity&) noexcept;
private:
  explicit lifecycle_node_identity(std::string hex);
  std::string hex_;
  friend class detail::model_access;
};
class lifecycle_node_set_identity final {
public:
  [[nodiscard]] const std::string& hex() const noexcept;
  friend bool operator==(const lifecycle_node_set_identity&, const lifecycle_node_set_identity&) noexcept;
  friend bool operator!=(const lifecycle_node_set_identity&, const lifecycle_node_set_identity&) noexcept;
private:
  explicit lifecycle_node_set_identity(std::string hex);
  std::string hex_;
  friend class detail::model_access;
};
class lifecycle_execution_result_identity final {
public:
  [[nodiscard]] const std::string& hex() const noexcept;
  friend bool operator==(const lifecycle_execution_result_identity&, const lifecycle_execution_result_identity&) noexcept;
  friend bool operator!=(const lifecycle_execution_result_identity&, const lifecycle_execution_result_identity&) noexcept;
private:
  explicit lifecycle_execution_result_identity(std::string hex);
  std::string hex_;
  friend class detail::model_access;
};

class lifecycle_node final {
public:
  [[nodiscard]] std::uint16_t schema_version() const noexcept;
  [[nodiscard]] const pkgapply::application_request_identity& application_request() const noexcept;
  [[nodiscard]] const pkgplan::operation_plan_identity& plan() const noexcept;
  [[nodiscard]] pkgplan::operation_kind operation() const noexcept;
  [[nodiscard]] const pkgapply::application_target_context_identity& target() const noexcept;
  [[nodiscard]] const pkgapply::lifecycle_executor_identity& executor() const noexcept;
  [[nodiscard]] lifecycle_subject subject() const noexcept;
  [[nodiscard]] const pkgplan::package_release& release() const noexcept;
  [[nodiscard]] pkgsource::lifecycle_action action() const noexcept;
  [[nodiscard]] const pkgsource::program& program() const noexcept;
  [[nodiscard]] const std::optional<pkgsource::source_snapshot_identity>& source() const noexcept;
  [[nodiscard]] const std::optional<pkgplan::installed_control_identity>& installed_control() const noexcept;
  [[nodiscard]] const lifecycle_node_identity& identity() const noexcept;
  friend bool operator==(const lifecycle_node&, const lifecycle_node&) noexcept;
  friend bool operator!=(const lifecycle_node&, const lifecycle_node&) noexcept;
  friend bool operator<(const lifecycle_node&, const lifecycle_node&) noexcept;
private:
  lifecycle_node(pkgapply::application_request_identity request,
                 pkgplan::operation_plan_identity plan,
                 pkgplan::operation_kind operation,
                 pkgapply::application_target_context_identity target,
                 pkgapply::lifecycle_executor_identity executor,
                 lifecycle_subject subject,
                 pkgplan::package_release release,
                 pkgsource::lifecycle_action action,
                 pkgsource::program program,
                 std::optional<pkgsource::source_snapshot_identity> source,
                 std::optional<pkgplan::installed_control_identity> installed_control,
                 lifecycle_node_identity identity);
  std::uint16_t schema_version_ = lifecycle_node_schema_version;
  pkgapply::application_request_identity request_;
  pkgplan::operation_plan_identity plan_;
  pkgplan::operation_kind operation_;
  pkgapply::application_target_context_identity target_;
  pkgapply::lifecycle_executor_identity executor_;
  lifecycle_subject subject_;
  pkgplan::package_release release_;
  pkgsource::lifecycle_action action_;
  pkgsource::program program_;
  std::optional<pkgsource::source_snapshot_identity> source_;
  std::optional<pkgplan::installed_control_identity> installed_control_;
  lifecycle_node_identity identity_;
  friend class detail::model_access;
};

class lifecycle_node_set final {
public:
  [[nodiscard]] std::uint16_t schema_version() const noexcept;
  [[nodiscard]] const pkgapply::application_request_identity& application_request() const noexcept;
  [[nodiscard]] const std::vector<lifecycle_node>& nodes() const noexcept;
  [[nodiscard]] const lifecycle_node* find(lifecycle_subject subject, pkgsource::lifecycle_action action) const noexcept;
  [[nodiscard]] const lifecycle_node_set_identity& identity() const noexcept;
private:
  lifecycle_node_set(pkgapply::application_request_identity request,
                     std::vector<lifecycle_node> nodes,
                     lifecycle_node_set_identity identity);
  std::uint16_t schema_version_ = lifecycle_node_set_schema_version;
  pkgapply::application_request_identity request_;
  std::vector<lifecycle_node> nodes_;
  lifecycle_node_set_identity identity_;
  friend class detail::model_access;
};

struct lifecycle_session_paths final {
  pkgexec::root_view_identity execution_root;
  std::filesystem::path execution_root_path;
  pkgapply::root_view_identity target_root;
  std::filesystem::path target_root_path;
  std::filesystem::path session_root;
};
struct lifecycle_execution_identity final {
  pkgexec::interpreter_identity interpreter;
  std::uint64_t user_id = 0;
  std::uint64_t group_id = 0;
  std::vector<std::uint64_t> supplementary_groups;
};

class admitted_lifecycle_session final {
public:
  [[nodiscard]] static admitted_lifecycle_session admit(
      pkgapply::package_application_request request,
      lifecycle_node node,
      lifecycle_session_paths paths,
      lifecycle_execution_identity identity);
  [[nodiscard]] const pkgapply::package_application_request& request() const noexcept;
  [[nodiscard]] const lifecycle_node& node() const noexcept;
  [[nodiscard]] const lifecycle_session_paths& paths() const noexcept;
  [[nodiscard]] const lifecycle_execution_identity& execution_identity() const noexcept;
private:
  admitted_lifecycle_session(pkgapply::package_application_request request,
                             lifecycle_node node,
                             lifecycle_session_paths paths,
                             lifecycle_execution_identity identity);
  pkgapply::package_application_request request_;
  lifecycle_node node_;
  lifecycle_session_paths paths_;
  lifecycle_execution_identity identity_;
};

struct prepared_execution final {
  pkgexec::execution_request request;
  pkgexec::execution_resources resources;
  std::filesystem::path temporary_root;
};

class lifecycle_execution_result final {
public:
  [[nodiscard]] std::uint16_t schema_version() const noexcept;
  [[nodiscard]] const lifecycle_node& node() const noexcept;
  [[nodiscard]] const pkgexec::execution_result& execution() const noexcept;
  [[nodiscard]] bool succeeded() const noexcept;
  [[nodiscard]] const lifecycle_execution_result_identity& identity() const noexcept;
private:
  lifecycle_execution_result(lifecycle_node node,
                             pkgexec::execution_result execution,
                             lifecycle_execution_result_identity identity);
  std::uint16_t schema_version_ = lifecycle_execution_result_schema_version;
  lifecycle_node node_;
  pkgexec::execution_result execution_;
  lifecycle_execution_result_identity identity_;
  friend class detail::executor_access;
  friend class detail::model_access;
};
} // namespace pkgapply_exec
