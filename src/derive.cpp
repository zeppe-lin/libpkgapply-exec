// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgapply-exec/derive.h>
#include <libpkgapply-exec/error.h>
#include "model_access.h"
#include <algorithm>
#include <optional>
#include <string>
#include <utility>
namespace pkgapply_exec {
namespace {
const pkgapply::lifecycle_executor_identity& executor(const pkgapply::package_application_request& request) {
  if (!request.target().lifecycle_executor())
    throw error(error_code::missing_lifecycle_executor,"application target does not bind a lifecycle executor");
  return *request.target().lifecycle_executor();
}
void append_incoming(std::vector<lifecycle_node>& nodes,
                     const pkgapply::package_application_request& request,
                     const pkgapply::incoming_package_authority& incoming,
                     const pkgplan::package_release& release) {
  const auto& source=incoming.build().request().source();
  for (const auto action : {pkgsource::lifecycle_action::pre_install,pkgsource::lifecycle_action::post_install}) {
    const auto* lifecycle=source.recipe().lifecycle(action); if (!lifecycle) continue;
    nodes.push_back(detail::model_access::node(request.identity(),request.plan(),request.kind(),
      request.target().identity(),executor(request),lifecycle_subject::incoming,release,action,
      lifecycle->value(),source.identity(),std::nullopt));
  }
}
pkgsource::lifecycle_action action(pkgplan::removal_lifecycle_phase phase) {
  switch (phase) { case pkgplan::removal_lifecycle_phase::pre_remove:return pkgsource::lifecycle_action::pre_remove;
                   case pkgplan::removal_lifecycle_phase::post_remove:return pkgsource::lifecycle_action::post_remove; }
  throw error(error_code::unsupported_program_format,"unknown removal lifecycle phase");
}
void append_installed(std::vector<lifecycle_node>& nodes,
                      const pkgapply::package_application_request& request,
                      const pkgplan::package_release& release,
                      const pkgplan::installed_control_identity& control_identity,
                      const pkgplan::installed_control_projection& control) {
  if (!pkgplan::is_known(control.completeness().removal_lifecycle))
    throw error(error_code::historical_control_unavailable,"historical removal lifecycle control is unavailable");
  for (const auto& declaration : control.removal_lifecycle()) {
    if (declaration.format()!="text/x-posix-shell")
      throw error(error_code::unsupported_program_format,"installed lifecycle declaration is not text/x-posix-shell");
    const auto selected=action(declaration.phase());
    nodes.push_back(detail::model_access::node(request.identity(),request.plan(),request.kind(),
      request.target().identity(),executor(request),lifecycle_subject::installed,release,selected,
      pkgsource::program(pkgsource::program_language::posix_shell,declaration.material()),
      std::nullopt,control_identity));
  }
}
}
lifecycle_node_set derive(const pkgapply::package_application_request& request) {
  static_cast<void>(executor(request));
  std::vector<lifecycle_node> nodes;
  if (const auto* install=request.installation()) {
    append_incoming(nodes,request,install->incoming(),install->plan().release());
  } else if (const auto* upgrade=request.upgrade()) {
    append_installed(nodes,request,upgrade->plan().old_release(),upgrade->plan().inputs().old_control(),upgrade->plan().old_control());
    append_incoming(nodes,request,upgrade->incoming(),upgrade->plan().release());
  } else if (const auto* removal=request.removal()) {
    append_installed(nodes,request,removal->plan().release(),removal->plan().inputs().control(),removal->plan().control());
  }
  return detail::model_access::set(request.identity(),std::move(nodes));
}
} // namespace pkgapply_exec
