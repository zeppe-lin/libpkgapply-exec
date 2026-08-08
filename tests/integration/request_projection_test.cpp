// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../fixtures/lifecycle.h"
#include "../support/test.h"
#include "../../src/execution_request.h"

#include <algorithm>
#include <iostream>

namespace {
using pkgapply_exec_test::node;
using pkgapply_exec_test::requests;
using pkgapply_exec_test::session;
using pkgapply_exec_test::temporary_base;

bool has_guarantee(const pkgexec::execution_request& request,
                   pkgexec::execution_guarantee value)
{
  return std::find(request.required_guarantees().begin(),
                   request.required_guarantees().end(), value) !=
         request.required_guarantees().end();
}

const pkgexec::environment_variable* variable(
    const pkgexec::environment_policy& environment,
    std::string_view name)
{
  const auto& values = environment.additional_variables();
  const auto item = std::find_if(values.begin(), values.end(), [&](const auto& value) {
    return value.name() == name;
  });
  return item == values.end() ? nullptr : &*item;
}

void prove_exact_projection()
{
  const auto application = requests();
  const auto nodes = pkgapply_exec::derive(application.install);
  const auto& selected = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);
  const auto admitted = session(
      application.install, selected, temporary_base("projection"));
  const auto request = pkgapply_exec::detail::lifecycle_execution_request(admitted);

  TEST_CHECK(request.program() == selected.program());
  TEST_CHECK(request.purpose().kind() == pkgexec::execution_purpose_kind::lifecycle);
  TEST_CHECK(request.purpose().action() == selected.action());
  TEST_CHECK(request.interpreter() == admitted.execution_identity().interpreter);
  TEST_CHECK(request.root_view() == admitted.paths().execution_root);
  TEST_CHECK(request.resources().working_directory() ==
             pkgexec::resource_slot::singleton(pkgexec::resource_role::managed_target_root));

  const auto target_slot = pkgexec::resource_slot::singleton(
      pkgexec::resource_role::managed_target_root);
  const auto temporary_slot = pkgexec::resource_slot::singleton(
      pkgexec::resource_role::private_temporary_root);
  const auto& target = request.resources().binding(target_slot);
  const auto& temporary = request.resources().binding(temporary_slot);
  TEST_CHECK(target.access() == pkgexec::resource_access::writable);
  TEST_CHECK(target.mount_point().string() == "/target");
  TEST_CHECK(temporary.access() == pkgexec::resource_access::writable);
  TEST_CHECK(temporary.mount_point().string() == "/tmp");
  TEST_CHECK(target.resource() != temporary.resource());

  const auto& environment = request.environment();
  TEST_CHECK(environment.network() == pkgexec::network_policy::denied);
  TEST_CHECK(environment.standard_input() == pkgexec::stdin_policy::closed);
  TEST_CHECK(environment.standard_output() == pkgexec::stream_policy::capture_complete);
  TEST_CHECK(environment.standard_error() == pkgexec::stream_policy::capture_complete);
  TEST_CHECK(environment.home_directory().string() == "/tmp/home");
  TEST_CHECK(environment.temporary_directory().string() == "/tmp");
  TEST_CHECK(environment.parallelism() == 1U);
  TEST_CHECK(environment.file_creation_mask() == 0022U);
  TEST_CHECK(environment.additional_variables().size() == 6U);
  TEST_CHECK(variable(environment, "ZEPPE_LIN_TARGET_ROOT")->value() == "/target");
  TEST_CHECK(variable(environment, "ZEPPE_LIN_PACKAGE_NAME")->value() == "tool");
  TEST_CHECK(variable(environment, "ZEPPE_LIN_PACKAGE_VERSION")->value() == "1.0");
  TEST_CHECK(variable(environment, "ZEPPE_LIN_PACKAGE_RELEASE")->value() == "1");
  TEST_CHECK(variable(environment, "ZEPPE_LIN_LIFECYCLE_ACTION")->value() == "pre-install");
  TEST_CHECK(variable(environment, "ZEPPE_LIN_LIFECYCLE_SUBJECT")->value() == "incoming");

  const auto& credentials = request.credentials();
  TEST_CHECK(credentials.user_id() == admitted.execution_identity().user_id);
  TEST_CHECK(credentials.group_id() == admitted.execution_identity().group_id);
  TEST_CHECK(credentials.supplementary_groups() == std::vector<std::uint64_t>({66, 77}));
  TEST_CHECK(credentials.no_new_privileges());
  TEST_CHECK(request.cancellation().mode() == pkgexec::cancellation_mode::disabled);
  TEST_CHECK(!has_guarantee(request, pkgexec::execution_guarantee::cancellation));
  TEST_CHECK(has_guarantee(request, pkgexec::execution_guarantee::network_denied));
  TEST_CHECK(has_guarantee(request, pkgexec::execution_guarantee::writable_resources));
}

void prove_host_coordinates_are_not_semantic()
{
  const auto application = requests();
  const auto nodes = pkgapply_exec::derive(application.install);
  const auto& selected = node(
      nodes, pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);
  const auto first = session(application.install, selected,
                             temporary_base("coordinate-a"));
  const auto second = session(application.install, selected,
                              temporary_base("coordinate-b"));
  TEST_CHECK(pkgapply_exec::detail::lifecycle_execution_request(first).identity() ==
             pkgapply_exec::detail::lifecycle_execution_request(second).identity());
}
}

int main()
{
  try {
    prove_exact_projection();
    prove_host_coordinates_are_not_semantic();
    return 0;
  } catch (const std::exception& value) {
    std::cerr << value.what() << '\n';
    return 1;
  }
}
