// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <stdexcept>
#include <string>
#include <string_view>
namespace pkgapply_exec {
enum class error_code {
  missing_lifecycle_executor,
  historical_control_unavailable,
  unsupported_program_format,
  request_node_mismatch,
  target_binding_mismatch,
  invalid_effect_coordinate,
  invalid_execution_identity,
  resource_preparation_failed,
  backend_contract_violation,
  inconsistent_result,
  corrupt_encoding,
  authority_mismatch,
};
[[nodiscard]] std::string_view to_string(error_code value) noexcept;
class error final : public std::runtime_error {
public:
  error(error_code code, std::string message);
  [[nodiscard]] error_code code() const noexcept;
private:
  error_code code_;
};
} // namespace pkgapply_exec
