// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "lifecycle.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace pkgapply_exec_test {

inline pkgexec::backend_capability_profile backend_capabilities(char seed = 'a')
{
  return pkgexec::backend_capability_profile::seal(
      pkgexec::backend_identity::from_sha256(std::string(64U, seed)),
      {
          pkgexec::execution_guarantee::exact_interpreter,
          pkgexec::execution_guarantee::closed_environment,
          pkgexec::execution_guarantee::root_view,
          pkgexec::execution_guarantee::read_only_resources,
          pkgexec::execution_guarantee::writable_resources,
          pkgexec::execution_guarantee::fixed_credentials,
          pkgexec::execution_guarantee::network_denied,
          pkgexec::execution_guarantee::loopback_isolated,
          pkgexec::execution_guarantee::resource_limits,
          pkgexec::execution_guarantee::cancellation,
          pkgexec::execution_guarantee::complete_stdout_capture,
          pkgexec::execution_guarantee::complete_stderr_capture,
          pkgexec::execution_guarantee::cleanup_verified,
          pkgexec::execution_guarantee::cpu_time_limit,
          pkgexec::execution_guarantee::address_space_limit,
          pkgexec::execution_guarantee::file_size_limit,
          pkgexec::execution_guarantee::open_files_limit,
          pkgexec::execution_guarantee::process_count_limit,
      });
}

enum class backend_mode {
  succeed,
  fail_before_start,
  fail_after_start,
  wrong_request,
  wrong_backend,
  throw_execute,
  throw_execute_nonstandard,
  throw_capabilities,
  throw_capabilities_nonstandard,
};

class fixture_backend final : public pkgexec::execution_backend {
public:
  explicit fixture_backend(
      backend_mode mode,
      std::optional<pkgexec::execution_request> foreign = std::nullopt)
      : mode_(mode), foreign_(std::move(foreign))
  {
  }

  pkgexec::backend_capability_profile capabilities() const override
  {
    if (mode_ == backend_mode::throw_capabilities)
      throw std::runtime_error("fixture capability exception");
    if (mode_ == backend_mode::throw_capabilities_nonstandard)
      throw 17;
    return backend_capabilities();
  }

  pkgexec::execution_result execute(
      const pkgexec::execution_request& request,
      const pkgexec::execution_resources&) override
  {
    if (mode_ == backend_mode::throw_execute)
      throw std::runtime_error("fixture execution exception");
    if (mode_ == backend_mode::throw_execute_nonstandard)
      throw 23;

    auto evidence_request = request;
    if (mode_ == backend_mode::wrong_request) {
      if (!foreign_)
        throw std::logic_error("foreign request fixture is missing");
      evidence_request = *foreign_;
    }
    const auto evidence_backend = mode_ == backend_mode::wrong_backend
        ? backend_capabilities('b')
        : backend_capabilities();

    if (mode_ == backend_mode::fail_before_start) {
      return pkgexec::execution_result::failed_before_start(
          evidence_request, evidence_backend,
          pkgexec::execution_failure_kind::backend_unsupported, {},
          "fixture refusal");
    }
    if (mode_ == backend_mode::fail_after_start) {
      return pkgexec::execution_result::failed_after_start(
          evidence_request, evidence_backend, evidence_request.interpreter(),
          pkgexec::process_termination::exited(7),
          pkgexec::stream_capture::retained("fixture stdout\n"),
          pkgexec::stream_capture::retained("fixture stderr\n"),
          evidence_request.required_guarantees(),
          pkgexec::cleanup_outcome::verified,
          pkgexec::execution_failure_kind::program_exited_nonzero,
          "fixture program failure");
    }
    return pkgexec::execution_result::succeeded(
        evidence_request, evidence_backend, evidence_request.interpreter(),
        pkgexec::stream_capture::retained("ok\n"),
        pkgexec::stream_capture::retained(""),
        evidence_request.required_guarantees(), "fixture success");
  }

private:
  backend_mode mode_;
  std::optional<pkgexec::execution_request> foreign_;
};

} // namespace pkgapply_exec_test
