// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgapply-exec/error.h>
#include <utility>
namespace pkgapply_exec {
std::string_view to_string(error_code value) noexcept {
  switch (value) {
    case error_code::missing_lifecycle_executor: return "missing-lifecycle-executor";
    case error_code::historical_control_unavailable: return "historical-control-unavailable";
    case error_code::unsupported_program_format: return "unsupported-program-format";
    case error_code::request_node_mismatch: return "request-node-mismatch";
    case error_code::target_binding_mismatch: return "target-binding-mismatch";
    case error_code::invalid_effect_coordinate: return "invalid-effect-coordinate";
    case error_code::invalid_execution_identity: return "invalid-execution-identity";
    case error_code::resource_preparation_failed: return "resource-preparation-failed";
    case error_code::backend_contract_violation: return "backend-contract-violation";
    case error_code::inconsistent_result: return "inconsistent-result";
    case error_code::corrupt_encoding: return "corrupt-encoding";
    case error_code::authority_mismatch: return "authority-mismatch";
  }
  return "unknown";
}
error::error(error_code code, std::string message) : std::runtime_error(std::move(message)), code_(code) {}
error_code error::code() const noexcept { return code_; }
} // namespace pkgapply_exec
