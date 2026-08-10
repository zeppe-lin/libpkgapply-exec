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
void require_directory(const fs::path& path,std::string_view name) {
  std::error_code ec; const auto status=fs::symlink_status(path,ec);
  if (ec || !fs::is_directory(status) || fs::is_symlink(status))
    throw error(error_code::resource_preparation_failed,std::string(name)+" is not a real directory");
}
void own(const fs::path& path,const lifecycle_execution_identity& identity) {
  if (::chown(path.c_str(),static_cast<uid_t>(identity.user_id),static_cast<gid_t>(identity.group_id))!=0)
    throw error(error_code::resource_preparation_failed,"cannot assign lifecycle writable resource ownership: "+std::string(std::strerror(errno)));
}
pkgexec::backend_capability_profile backend_capabilities(
    pkgexec::execution_backend& backend) {
  try {
    return backend.capabilities();
  } catch (const std::exception& value) {
    throw error(error_code::backend_contract_violation,
                std::string("execution backend capability query threw: ") +
                    value.what());
  } catch (...) {
    throw error(error_code::backend_contract_violation,
                "execution backend capability query threw a non-standard exception");
  }
}

pkgexec::execution_result invoke_backend(
    pkgexec::execution_backend& backend,
    const prepared_execution& prepared) {
  try {
    return backend.execute(prepared.request, prepared.resources);
  } catch (const std::exception& value) {
    throw error(error_code::backend_contract_violation,
                std::string("execution backend threw instead of returning evidence: ") +
                    value.what());
  } catch (...) {
    throw error(error_code::backend_contract_violation,
                "execution backend threw a non-standard exception instead of returning evidence");
  }
}
}
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
  const auto advertised_backend=backend_capabilities(backend);
  auto prepared=prepare(session);
  auto evidence=invoke_backend(backend,prepared);
  if (evidence.request()!=prepared.request)
    throw error(error_code::backend_contract_violation,"execution backend returned evidence for another request");
  if (evidence.backend()!=advertised_backend)
    throw error(error_code::backend_contract_violation,"execution backend returned evidence for another backend profile");
  return detail::executor_access::make(session.node(),std::move(evidence));
}
} // namespace pkgapply_exec
