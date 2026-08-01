// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgapply-exec/executor.h>
#include <libpkgapply-exec/error.h>
#include "execution_request.h"
#include "model_access.h"
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <string>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
namespace pkgapply_exec {
namespace {
namespace fs=std::filesystem;
pkgexec::resource_identity resource_identity(std::string_view domain,std::string_view node) {
  std::string material(domain); material.push_back('\0'); material.append(node);
  return pkgexec::resource_identity::from_sha256(pkgexec::sha256_digest::of_bytes(material).hex());
}
void require_directory(const fs::path& path,std::string_view name) {
  std::error_code ec; const auto status=fs::symlink_status(path,ec);
  if (ec || !fs::is_directory(status) || fs::is_symlink(status))
    throw error(error_code::resource_preparation_failed,std::string(name)+" is not a real directory");
}
void own(const fs::path& path,const lifecycle_execution_identity& identity) {
  if (::chown(path.c_str(),static_cast<uid_t>(identity.user_id),static_cast<gid_t>(identity.group_id))!=0)
    throw error(error_code::resource_preparation_failed,"cannot assign lifecycle writable resource ownership: "+std::string(std::strerror(errno)));
}
std::string action_text(pkgsource::lifecycle_action action) { return std::string(pkgsource::to_string(action)); }
}
namespace detail {
pkgexec::resource_identity managed_target_resource_identity(const lifecycle_node& node) {
  return resource_identity("pkgapply-exec/managed-target-root/v1", node.identity().hex());
}
pkgexec::resource_identity temporary_resource_identity(const lifecycle_node& node) {
  return resource_identity("pkgapply-exec/private-temporary-root/v1", node.identity().hex());
}
pkgexec::execution_request lifecycle_execution_request(const admitted_lifecycle_session& session) {
  const auto target_slot=pkgexec::resource_slot::singleton(pkgexec::resource_role::managed_target_root);
  const auto target_identity=managed_target_resource_identity(session.node());
  const auto temporary_slot=pkgexec::resource_slot::singleton(pkgexec::resource_role::private_temporary_root);
  const auto temporary_identity=temporary_resource_identity(session.node());
  std::vector<pkgexec::resource_binding> bindings;
  bindings.emplace_back(target_slot,target_identity,pkgexec::resource_access::writable,pkgexec::logical_path::parse("/target"));
  bindings.emplace_back(temporary_slot,temporary_identity,pkgexec::resource_access::writable,pkgexec::logical_path::parse("/tmp"));
  auto layout=pkgexec::resource_layout::seal(std::move(bindings),target_slot);
  const auto& release=session.node().release();
  std::vector<pkgexec::environment_variable> variables;
  variables.emplace_back("ZEPPE_LIN_TARGET_ROOT","/target");
  variables.emplace_back("ZEPPE_LIN_PACKAGE_NAME",release.name());
  variables.emplace_back("ZEPPE_LIN_PACKAGE_VERSION",release.version());
  variables.emplace_back("ZEPPE_LIN_PACKAGE_RELEASE",release.release());
  variables.emplace_back("ZEPPE_LIN_LIFECYCLE_ACTION",action_text(session.node().action()));
  variables.emplace_back("ZEPPE_LIN_LIFECYCLE_SUBJECT",std::string(to_string(session.node().subject())));
  auto environment=pkgexec::environment_policy::hermetic(
      {pkgexec::logical_path::parse("/usr/bin"),pkgexec::logical_path::parse("/bin")},
      pkgexec::logical_path::parse("/tmp/home"),pkgexec::logical_path::parse("/tmp"),1,0022,
      std::nullopt,pkgexec::network_policy::denied,pkgexec::stdin_policy::closed,
      pkgexec::stream_policy::capture_complete,pkgexec::stream_policy::capture_complete,std::move(variables));
  return pkgexec::execution_request::seal(
      session.node().program(),pkgexec::execution_purpose::lifecycle(session.node().action()),
      session.execution_identity().interpreter,session.paths().execution_root,std::move(layout),
      std::move(environment),pkgexec::credential_policy::fixed(
        session.execution_identity().user_id,session.execution_identity().group_id,
        session.execution_identity().supplementary_groups,true),
      pkgexec::resource_limits::make(),pkgexec::cancellation_policy::disabled());
}
} // namespace detail
prepared_execution prepare(const admitted_lifecycle_session& session) {
  require_directory(session.paths().execution_root_path,"execution root");
  require_directory(session.paths().target_root_path,"managed target root");
  require_directory(session.paths().session_root.parent_path(),"session-root parent");
  std::error_code ec;
  const auto session_status=fs::symlink_status(session.paths().session_root,ec);
  if (!ec && fs::exists(session_status)) {
    if (!fs::is_directory(session_status) || fs::is_symlink(session_status) ||
        !fs::is_empty(session.paths().session_root,ec) || ec)
      throw error(error_code::resource_preparation_failed,"session root must be absent or an empty real directory");
  } else {
    ec.clear();
    if (!fs::create_directory(session.paths().session_root,ec) || ec)
      throw error(error_code::resource_preparation_failed,"cannot create lifecycle session root");
  }
  if (::chmod(session.paths().session_root.c_str(),0700)!=0)
    throw error(error_code::resource_preparation_failed,"cannot protect lifecycle session root");
  const fs::path temporary=session.paths().session_root/"tmp";
  const fs::path home=temporary/"home";
  if (!fs::create_directory(temporary,ec) || ec ||
      !fs::create_directory(home,ec) || ec)
    throw error(error_code::resource_preparation_failed,"cannot create lifecycle temporary directories");
  if (::chmod(temporary.c_str(),0700)!=0 || ::chmod(home.c_str(),0700)!=0)
    throw error(error_code::resource_preparation_failed,"cannot protect lifecycle temporary directories");
  own(temporary,session.execution_identity()); own(home,session.execution_identity());

  const auto target_identity=detail::managed_target_resource_identity(session.node());
  const auto temporary_identity=detail::temporary_resource_identity(session.node());
  auto request=detail::lifecycle_execution_request(session);
  std::vector<pkgexec::resource_materialization> materializations;
  materializations.emplace_back(target_identity,session.paths().target_root_path);
  materializations.emplace_back(temporary_identity,temporary);
  auto resources=pkgexec::execution_resources::admit(request,session.paths().execution_root,
      session.paths().execution_root_path,std::move(materializations));
  return {std::move(request),std::move(resources),temporary};
}
lifecycle_execution_result execute(const admitted_lifecycle_session& session,pkgexec::execution_backend& backend) {
  auto prepared=prepare(session);
  pkgexec::execution_result evidence=[&] {
    try { return backend.execute(prepared.request,prepared.resources); }
    catch (const std::exception& value) {
      throw error(error_code::backend_contract_violation,std::string("execution backend threw instead of returning evidence: ")+value.what());
    }
  }();
  if (evidence.request()!=prepared.request)
    throw error(error_code::backend_contract_violation,"execution backend returned evidence for another request");
  return detail::executor_access::make(session.node(),std::move(evidence));
}
} // namespace pkgapply_exec
