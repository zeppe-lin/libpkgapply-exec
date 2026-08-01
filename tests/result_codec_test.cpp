// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgapply-exec/libpkgapply-exec.h>

#include "fixture.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;
namespace fixture = pkgapply::test::fixture;

namespace {

void check(bool value, std::string_view message)
{
  if (!value) {
    std::cerr << message << '\n';
    std::exit(1);
  }
}

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

pkgplan::target_system_context_identity target_identity(std::uint8_t value)
{
  std::array<std::uint8_t, 32> bytes{};
  bytes.fill(value);
  return pkgplan::target_system_context_identity::from_sha256(bytes);
}

pkgapply::application_target_context target()
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
      apply_identity<pkgapply::lifecycle_executor_identity>(12));
}

pkgapply::application_execution_control control()
{
  return pkgapply::application_execution_control::make(
      pkgapply::application_recovery_requirement::exact_prior_state,
      pkgapply::application_durability_requirement::all_application_domains,
      pkgapply::application_cancellation_policy::recover_after_target_mutation,
      4096, 8192);
}

pkgapply::package_application_request request()
{
  auto context = target();
  fixture::planning_authorities authorities(context.target());
  return pkgapply::package_application_request(
      pkgapply::installation_application_request::make(
          fixture::ordinary_installation(authorities),
          fixture::ordinary_installation_incoming(), context, control()));
}

class backend final : public pkgexec::execution_backend {
public:
  enum class mode { success, failure };

  explicit backend(mode value) : mode_(value) {}

  pkgexec::backend_capability_profile capabilities() const override
  {
    return pkgexec::backend_capability_profile::seal(
        exec_identity<pkgexec::backend_identity>(1), guarantees_);
  }

  pkgexec::execution_result execute(
      const pkgexec::execution_request& request,
      const pkgexec::execution_resources&) override
  {
    guarantees_ = request.required_guarantees();
    auto profile = pkgexec::backend_capability_profile::seal(
        exec_identity<pkgexec::backend_identity>(1), guarantees_);
    if (mode_ == mode::failure)
      return pkgexec::execution_result::failed_before_start(
          request, std::move(profile),
          pkgexec::execution_failure_kind::backend_unsupported, {},
          "fixture failure");
    return pkgexec::execution_result::succeeded(
        request, std::move(profile), request.interpreter(),
        pkgexec::stream_capture::retained("ok\n"),
        pkgexec::stream_capture::retained(""),
        request.required_guarantees(), "fixture success");
  }

private:
  mode mode_;
  mutable std::vector<pkgexec::execution_guarantee> guarantees_;
};

pkgapply_exec::admitted_lifecycle_session make_session(
    const pkgapply::package_application_request& application,
    const pkgapply_exec::lifecycle_node& node,
    const fs::path& base,
    std::uint8_t execution_root = 2,
    std::uint8_t interpreter = 3)
{
  pkgapply_exec::lifecycle_session_paths paths{
      exec_identity<pkgexec::root_view_identity>(execution_root),
      base / "exec", application.target().root_view(), base / "target",
      base / "session"};
  pkgapply_exec::lifecycle_execution_identity identity{
      exec_identity<pkgexec::interpreter_identity>(interpreter),
      static_cast<std::uint64_t>(::geteuid()),
      static_cast<std::uint64_t>(::getegid()), {66, 77}};
  return pkgapply_exec::admitted_lifecycle_session::admit(
      application, node, std::move(paths), std::move(identity));
}

void prepare_directories(const fs::path& base)
{
  fs::remove_all(base);
  fs::create_directories(base / "exec");
  fs::create_directories(base / "target");
}

bool fails_with(const pkgapply_exec::lifecycle_execution_result_encoding& bytes,
                pkgapply_exec::admitted_lifecycle_session session,
                pkgexec::backend_capability_profile backend,
                pkgapply_exec::error_code expected)
{
  try {
    static_cast<void>(pkgapply_exec::decode_lifecycle_execution_result(
        bytes, std::move(session), std::move(backend)));
  } catch (const pkgapply_exec::error& problem) {
    return problem.code() == expected;
  }
  return false;
}

} // namespace

int main()
{
  const auto application = request();
  const auto nodes = pkgapply_exec::derive(application);
  const auto* pre = nodes.find(pkgapply_exec::lifecycle_subject::incoming,
                               pkgsource::lifecycle_action::pre_install);
  const auto* post = nodes.find(pkgapply_exec::lifecycle_subject::incoming,
                                pkgsource::lifecycle_action::post_install);
  check(pre != nullptr && post != nullptr,
        "installation lifecycle fixture is incomplete");

  const auto base = fs::temp_directory_path() /
      fs::path("libpkgapply-exec-codec-" + std::to_string(::getpid()));
  prepare_directories(base);
  const auto session = make_session(application, *pre, base);

  backend success_backend(backend::mode::success);
  const auto success = pkgapply_exec::execute(session, success_backend);
  const auto success_encoding =
      pkgapply_exec::encode_lifecycle_execution_result(success);
  check(success_encoding ==
            pkgapply_exec::encode_lifecycle_execution_result(success),
        "lifecycle codec is not deterministic");

  const auto success_profile = success.execution().backend();
  fs::remove_all(base);
  const auto decoded = pkgapply_exec::decode_lifecycle_execution_result(
      success_encoding, session, success_profile);
  check(!fs::exists(base), "lifecycle decoder touched the filesystem");
  check(decoded.identity() == success.identity(),
        "successful lifecycle result identity changed");
  check(decoded.node().identity() == success.node().identity(),
        "successful lifecycle node identity changed");
  check(decoded.execution().identity() == success.execution().identity(),
        "successful execution evidence identity changed");
  check(decoded.succeeded(), "successful lifecycle result became failed");
  check(pkgapply_exec::encode_lifecycle_execution_result(decoded) ==
            success_encoding,
        "successful lifecycle record did not round trip canonically");

  prepare_directories(base);
  backend failure_backend(backend::mode::failure);
  const auto failure = pkgapply_exec::execute(session, failure_backend);
  const auto failure_encoding =
      pkgapply_exec::encode_lifecycle_execution_result(failure);
  const auto failure_profile = failure.execution().backend();
  fs::remove_all(base);
  const auto decoded_failure =
      pkgapply_exec::decode_lifecycle_execution_result(
          failure_encoding, session, failure_profile);
  check(!decoded_failure.succeeded(),
        "failed lifecycle result became successful");
  check(decoded_failure.identity() == failure.identity(),
        "failed lifecycle result identity changed");

  auto corrupt = success_encoding;
  corrupt[corrupt.size() / 2U] ^= 0x01U;
  check(fails_with(corrupt, session, success_profile,
                   pkgapply_exec::error_code::corrupt_encoding),
        "checksum corruption was not rejected");

  auto truncated = success_encoding;
  truncated.pop_back();
  check(fails_with(truncated, session, success_profile,
                   pkgapply_exec::error_code::corrupt_encoding),
        "truncated lifecycle record was not rejected");

  const auto foreign_node_session = make_session(application, *post, base);
  check(fails_with(success_encoding, foreign_node_session, success_profile,
                   pkgapply_exec::error_code::authority_mismatch),
        "foreign lifecycle node was admitted");

  const auto foreign_execution_session =
      make_session(application, *pre, base, 9, 3);
  check(fails_with(success_encoding, foreign_execution_session,
                   success_profile,
                   pkgapply_exec::error_code::authority_mismatch),
        "foreign lifecycle execution session was admitted");

  const auto foreign_backend = pkgexec::backend_capability_profile::seal(
      exec_identity<pkgexec::backend_identity>(8),
      success.execution().established_guarantees());
  check(fails_with(success_encoding, session, foreign_backend,
                   pkgapply_exec::error_code::authority_mismatch),
        "foreign backend profile was admitted");

  fs::remove_all(base);
  return 0;
}
