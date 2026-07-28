// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgapply-exec/model.h>
#include <libpkgapply-exec/error.h>
#include "model_access.h"
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <sys/types.h>
#include <utility>
namespace pkgapply_exec {
namespace {
class record final {
public:
  explicit record(std::string_view domain) { text(domain); }
  void number(std::uint64_t value) {
    for (int shift = 56; shift >= 0; shift -= 8)
      bytes_.push_back(static_cast<char>((value >> shift) & 0xffU));
  }
  void text(std::string_view value) { number(value.size()); bytes_.append(value.data(), value.size()); }
  void present(bool value) { number(value ? 1 : 0); }
  [[nodiscard]] std::string finish() const { return pkgexec::sha256_digest::of_bytes(bytes_).hex(); }
private:
  std::string bytes_;
};
bool absolute_normal(const std::filesystem::path& path) {
  return path.is_absolute() && path == path.lexically_normal();
}
bool contains(const std::filesystem::path& parent, const std::filesystem::path& child) {
  auto p=parent.begin(); auto c=child.begin();
  for (; p!=parent.end(); ++p,++c) if (c==child.end() || *p!=*c) return false;
  return true;
}
}
namespace detail {
lifecycle_node_identity model_access::node_identity(std::string hex) { return lifecycle_node_identity(std::move(hex)); }
lifecycle_node_set_identity model_access::set_identity(std::string hex) { return lifecycle_node_set_identity(std::move(hex)); }
lifecycle_execution_result_identity model_access::result_identity(std::string hex) { return lifecycle_execution_result_identity(std::move(hex)); }
lifecycle_node model_access::node(pkgapply::application_request_identity request,
      pkgplan::operation_plan_identity plan, pkgplan::operation_kind operation,
      pkgapply::application_target_context_identity target,
      pkgapply::lifecycle_executor_identity executor, lifecycle_subject subject,
      pkgplan::package_release release, pkgsource::lifecycle_action action,
      pkgsource::program program,
      std::optional<pkgsource::source_snapshot_identity> source,
      std::optional<pkgplan::installed_control_identity> installed_control) {
    record value("pkgapply-exec/lifecycle-node/v1");
    value.number(lifecycle_node_schema_version);
    value.text(request.string()); value.text(plan.string());
    value.number(static_cast<std::uint64_t>(operation)); value.text(target.string());
    value.text(executor.string()); value.number(static_cast<std::uint64_t>(subject));
    value.text(release.identity().string()); value.text(release.name());
    value.text(release.version()); value.text(release.release());
    value.number(static_cast<std::uint64_t>(action));
    value.number(static_cast<std::uint64_t>(program.language()));
    value.text(program.material()); value.text(program.content_digest().hex());
    value.present(source.has_value()); if (source) value.text(source->hex());
    value.present(installed_control.has_value()); if (installed_control) value.text(installed_control->string());
    return lifecycle_node(std::move(request), std::move(plan), operation,
      std::move(target), std::move(executor), subject, std::move(release), action,
      std::move(program), std::move(source), std::move(installed_control),
      node_identity(value.finish()));
}
lifecycle_node_set model_access::set(pkgapply::application_request_identity request,
                                std::vector<lifecycle_node> nodes) {
    std::sort(nodes.begin(), nodes.end());
    if (std::adjacent_find(nodes.begin(), nodes.end()) != nodes.end())
      throw std::invalid_argument("duplicate lifecycle node identity");
    record value("pkgapply-exec/lifecycle-node-set/v1");
    value.number(lifecycle_node_set_schema_version); value.text(request.string());
    value.number(nodes.size()); for (const auto& node : nodes) value.text(node.identity().hex());
    return lifecycle_node_set(std::move(request), std::move(nodes), set_identity(value.finish()));
}
lifecycle_execution_result model_access::result(lifecycle_node node, pkgexec::execution_result execution) {
    record value("pkgapply-exec/lifecycle-execution-result/v1");
    value.number(lifecycle_execution_result_schema_version);
    value.text(node.identity().hex()); value.text(execution.identity().hex());
    return lifecycle_execution_result(std::move(node), std::move(execution), result_identity(value.finish()));
}
lifecycle_execution_result executor_access::make(lifecycle_node node, pkgexec::execution_result execution) {
    return model_access::result(std::move(node), std::move(execution));
}
} // namespace detail

std::string_view to_string(lifecycle_subject value) noexcept {
  switch (value) { case lifecycle_subject::incoming: return "incoming"; case lifecycle_subject::installed: return "installed"; }
  return "unknown";
}
#define ID_IMPL(Type) \
Type::Type(std::string hex):hex_(std::move(hex)){} \
const std::string& Type::hex() const noexcept{return hex_;} \
bool operator==(const Type& a,const Type& b) noexcept{return a.hex_==b.hex_;} \
bool operator!=(const Type& a,const Type& b) noexcept{return !(a==b);}
ID_IMPL(lifecycle_node_identity)
bool operator<(const lifecycle_node_identity& a,const lifecycle_node_identity& b) noexcept{return a.hex_<b.hex_;}
ID_IMPL(lifecycle_node_set_identity)
ID_IMPL(lifecycle_execution_result_identity)
#undef ID_IMPL

lifecycle_node::lifecycle_node(pkgapply::application_request_identity request,
    pkgplan::operation_plan_identity plan, pkgplan::operation_kind operation,
    pkgapply::application_target_context_identity target,
    pkgapply::lifecycle_executor_identity executor, lifecycle_subject subject,
    pkgplan::package_release release, pkgsource::lifecycle_action action,
    pkgsource::program program, std::optional<pkgsource::source_snapshot_identity> source,
    std::optional<pkgplan::installed_control_identity> installed_control,
    lifecycle_node_identity identity)
  : request_(std::move(request)), plan_(std::move(plan)), operation_(operation),
    target_(std::move(target)), executor_(std::move(executor)), subject_(subject),
    release_(std::move(release)), action_(action), program_(std::move(program)),
    source_(std::move(source)), installed_control_(std::move(installed_control)),
    identity_(std::move(identity)) {}
std::uint16_t lifecycle_node::schema_version() const noexcept{return schema_version_;}
const pkgapply::application_request_identity& lifecycle_node::application_request() const noexcept{return request_;}
const pkgplan::operation_plan_identity& lifecycle_node::plan() const noexcept{return plan_;}
pkgplan::operation_kind lifecycle_node::operation() const noexcept{return operation_;}
const pkgapply::application_target_context_identity& lifecycle_node::target() const noexcept{return target_;}
const pkgapply::lifecycle_executor_identity& lifecycle_node::executor() const noexcept{return executor_;}
lifecycle_subject lifecycle_node::subject() const noexcept{return subject_;}
const pkgplan::package_release& lifecycle_node::release() const noexcept{return release_;}
pkgsource::lifecycle_action lifecycle_node::action() const noexcept{return action_;}
const pkgsource::program& lifecycle_node::program() const noexcept{return program_;}
const std::optional<pkgsource::source_snapshot_identity>& lifecycle_node::source() const noexcept{return source_;}
const std::optional<pkgplan::installed_control_identity>& lifecycle_node::installed_control() const noexcept{return installed_control_;}
const lifecycle_node_identity& lifecycle_node::identity() const noexcept{return identity_;}
bool operator==(const lifecycle_node& a,const lifecycle_node& b) noexcept{return a.identity_==b.identity_;}
bool operator!=(const lifecycle_node& a,const lifecycle_node& b) noexcept{return !(a==b);}
bool operator<(const lifecycle_node& a,const lifecycle_node& b) noexcept{return a.identity_<b.identity_;}

lifecycle_node_set::lifecycle_node_set(pkgapply::application_request_identity request,
  std::vector<lifecycle_node> nodes,lifecycle_node_set_identity identity)
  : request_(std::move(request)),nodes_(std::move(nodes)),identity_(std::move(identity)){}
std::uint16_t lifecycle_node_set::schema_version() const noexcept{return schema_version_;}
const pkgapply::application_request_identity& lifecycle_node_set::application_request() const noexcept{return request_;}
const std::vector<lifecycle_node>& lifecycle_node_set::nodes() const noexcept{return nodes_;}
const lifecycle_node* lifecycle_node_set::find(lifecycle_subject subject,pkgsource::lifecycle_action action) const noexcept {
  const auto i=std::find_if(nodes_.begin(),nodes_.end(),[&](const auto& n){return n.subject()==subject&&n.action()==action;});
  return i==nodes_.end()?nullptr:&*i;
}
const lifecycle_node_set_identity& lifecycle_node_set::identity() const noexcept{return identity_;}

admitted_lifecycle_session admitted_lifecycle_session::admit(
    pkgapply::package_application_request request,lifecycle_node node,
    lifecycle_session_paths paths,lifecycle_execution_identity identity) {
  if (request.identity()!=node.application_request() || request.plan()!=node.plan() ||
      request.kind()!=node.operation() || request.target().identity()!=node.target())
    throw error(error_code::request_node_mismatch,"lifecycle node does not belong to the supplied application request");
  if (!request.target().lifecycle_executor() || *request.target().lifecycle_executor()!=node.executor())
    throw error(error_code::target_binding_mismatch,"application target does not bind the lifecycle executor carried by the node");
  if (request.target().root_view()!=paths.target_root)
    throw error(error_code::target_binding_mismatch,"target root-view identity differs from the application request");
  if (identity.user_id >= static_cast<std::uint64_t>(std::numeric_limits<uid_t>::max()) ||
      identity.group_id >= static_cast<std::uint64_t>(std::numeric_limits<gid_t>::max()) ||
      std::any_of(identity.supplementary_groups.begin(),identity.supplementary_groups.end(),
          [](std::uint64_t value) {
            return value >= static_cast<std::uint64_t>(std::numeric_limits<gid_t>::max());
          }))
    throw error(error_code::invalid_execution_identity,"lifecycle credentials exceed native numeric identifier bounds");
  std::sort(identity.supplementary_groups.begin(),identity.supplementary_groups.end());
  if (std::adjacent_find(identity.supplementary_groups.begin(),identity.supplementary_groups.end()) !=
      identity.supplementary_groups.end())
    throw error(error_code::invalid_execution_identity,"supplementary lifecycle groups must be unique");
  if (std::binary_search(identity.supplementary_groups.begin(),identity.supplementary_groups.end(),identity.group_id))
    throw error(error_code::invalid_execution_identity,"primary lifecycle group cannot be supplementary");
  for (const auto* p : {&paths.execution_root_path,&paths.target_root_path,&paths.session_root})
    if (!absolute_normal(*p)) throw error(error_code::invalid_effect_coordinate,"lifecycle effect paths must be absolute and normalized");
  if (paths.execution_root_path==paths.target_root_path ||
      contains(paths.execution_root_path,paths.target_root_path) || contains(paths.target_root_path,paths.execution_root_path) ||
      contains(paths.execution_root_path,paths.session_root) || contains(paths.session_root,paths.execution_root_path) ||
      contains(paths.target_root_path,paths.session_root) || contains(paths.session_root,paths.target_root_path))
    throw error(error_code::invalid_effect_coordinate,"execution root, target root, and session root must not overlap");
  return admitted_lifecycle_session(std::move(request),std::move(node),std::move(paths),std::move(identity));
}
admitted_lifecycle_session::admitted_lifecycle_session(pkgapply::package_application_request request,lifecycle_node node,lifecycle_session_paths paths,lifecycle_execution_identity identity)
 :request_(std::move(request)),node_(std::move(node)),paths_(std::move(paths)),identity_(std::move(identity)){}
const pkgapply::package_application_request& admitted_lifecycle_session::request() const noexcept{return request_;}
const lifecycle_node& admitted_lifecycle_session::node() const noexcept{return node_;}
const lifecycle_session_paths& admitted_lifecycle_session::paths() const noexcept{return paths_;}
const lifecycle_execution_identity& admitted_lifecycle_session::execution_identity() const noexcept{return identity_;}

lifecycle_execution_result::lifecycle_execution_result(lifecycle_node node,pkgexec::execution_result execution,lifecycle_execution_result_identity identity)
 :node_(std::move(node)),execution_(std::move(execution)),identity_(std::move(identity)){}
std::uint16_t lifecycle_execution_result::schema_version() const noexcept{return schema_version_;}
const lifecycle_node& lifecycle_execution_result::node() const noexcept{return node_;}
const pkgexec::execution_result& lifecycle_execution_result::execution() const noexcept{return execution_;}
bool lifecycle_execution_result::succeeded() const noexcept{return execution_.status()==pkgexec::execution_status::succeeded;}
const lifecycle_execution_result_identity& lifecycle_execution_result::identity() const noexcept{return identity_;}
} // namespace pkgapply_exec
