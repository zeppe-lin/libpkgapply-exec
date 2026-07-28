// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgapply-exec/libpkgapply-exec.h>
#include "fixture.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <unistd.h>
namespace fs=std::filesystem;
namespace fixture=pkgapply::test::fixture;
namespace {
void check(bool value,std::string_view message){if(!value){std::cerr<<message<<'\n';std::exit(1);}}
template<class Identity> Identity apply_identity(std::uint8_t value){
 std::string text="v1:sha256:"; constexpr char h[]="0123456789abcdef";
 for(std::size_t i=0;i<32;++i){auto b=static_cast<std::uint8_t>(value+i);text.push_back(h[b>>4]);text.push_back(h[b&15]);}
 return Identity::parse(text);
}
template<class Identity> Identity exec_identity(std::uint8_t value){return Identity::from_sha256(std::string(64,"0123456789abcdef"[value&15]));}
pkgplan::target_system_context_identity target_id(std::uint8_t v){std::array<std::uint8_t,32>b{};b.fill(v);return pkgplan::target_system_context_identity::from_sha256(b);}
pkgapply::application_target_context context(bool with_executor=true){
 return pkgapply::application_target_context::make(target_id(7),
  apply_identity<pkgapply::managed_target_identity>(2),apply_identity<pkgapply::root_view_identity>(3),
  apply_identity<pkgapply::observation_backend_identity>(4),apply_identity<pkgapply::mutation_backend_identity>(5),
  apply_identity<pkgapply::mutation_exclusion_domain_identity>(6),apply_identity<pkgapply::active_object_namespace_identity>(7),
  apply_identity<pkgapply::rejected_object_store_identity>(8),apply_identity<pkgapply::staging_namespace_identity>(9),
  apply_identity<pkgapply::journal_namespace_identity>(10),apply_identity<pkgapply::execution_capability_profile_identity>(11),
  with_executor?std::optional<pkgapply::lifecycle_executor_identity>(apply_identity<pkgapply::lifecycle_executor_identity>(12)):std::nullopt);
}
pkgapply::application_execution_control control(){return pkgapply::application_execution_control::make(
 pkgapply::application_recovery_requirement::exact_prior_state,
 pkgapply::application_durability_requirement::all_application_domains,
 pkgapply::application_cancellation_policy::recover_after_target_mutation,4096,8192);}
struct requests { pkgapply::package_application_request install; pkgapply::package_application_request upgrade; pkgapply::package_application_request remove; };
requests make_requests(bool with_executor=true){
 auto target=context(with_executor); fixture::planning_authorities a(target.target());
 return {
  pkgapply::package_application_request(pkgapply::installation_application_request::make(
    fixture::ordinary_installation(a),fixture::ordinary_installation_incoming(),target,control())),
  pkgapply::package_application_request(pkgapply::upgrade_application_request::make(
    fixture::ordinary_upgrade(a),fixture::ordinary_upgrade_incoming(),target,control())),
  pkgapply::package_application_request(pkgapply::removal_application_request::make(
    fixture::ordinary_removal(a),target,control()))};
}
class backend final:public pkgexec::execution_backend{
public:
 enum class mode{success,failure,throws,foreign};
 explicit backend(mode m,std::optional<pkgexec::execution_request> foreign=std::nullopt):mode_(m),foreign_(std::move(foreign)){}
 pkgexec::backend_capability_profile capabilities()const override{return pkgexec::backend_capability_profile::seal(exec_identity<pkgexec::backend_identity>(1),last_guarantees_);}
 pkgexec::execution_result execute(const pkgexec::execution_request& request,const pkgexec::execution_resources&)override{
  if(mode_==mode::throws)throw std::runtime_error("fixture throw");
  const auto& evidence_request=mode_==mode::foreign?foreign_.value():request;
  last_guarantees_=evidence_request.required_guarantees(); auto profile=pkgexec::backend_capability_profile::seal(exec_identity<pkgexec::backend_identity>(1),last_guarantees_);
  if(mode_==mode::failure)return pkgexec::execution_result::failed_before_start(evidence_request,std::move(profile),pkgexec::execution_failure_kind::backend_unsupported,{},"fixture failure");
  return pkgexec::execution_result::succeeded(evidence_request,std::move(profile),evidence_request.interpreter(),
    pkgexec::stream_capture::retained("ok\n"),pkgexec::stream_capture::retained(""),evidence_request.required_guarantees(),"fixture success");
 }
private:mode mode_;std::optional<pkgexec::execution_request> foreign_;mutable std::vector<pkgexec::execution_guarantee> last_guarantees_;
};
}
int main(){
 auto req=make_requests();
 auto install=pkgapply_exec::derive(req.install); auto upgrade=pkgapply_exec::derive(req.upgrade); auto remove=pkgapply_exec::derive(req.remove);
 check(install.nodes().size()==2,"installation must derive two incoming lifecycle nodes");
 check(upgrade.nodes().size()==4,"upgrade must derive two old and two incoming nodes");
 check(remove.nodes().size()==2,"removal must derive two installed lifecycle nodes");
 const auto* install_pre=install.find(pkgapply_exec::lifecycle_subject::incoming,pkgsource::lifecycle_action::pre_install);
 const auto* remove_pre=remove.find(pkgapply_exec::lifecycle_subject::installed,pkgsource::lifecycle_action::pre_remove);
 check(install_pre&&install_pre->program().material()=="echo pre-install\n","incoming program bytes changed");
 check(remove_pre&&remove_pre->program().material()=="echo old-pre-remove\n","historical program bytes changed");
 check(install.nodes()[0].identity()<install.nodes()[1].identity(),"node vector is not canonical identity order");
 check(upgrade.find(pkgapply_exec::lifecycle_subject::installed,pkgsource::lifecycle_action::pre_remove)->installed_control().has_value(),"installed node lacks historical control authority");
 check(upgrade.find(pkgapply_exec::lifecycle_subject::incoming,pkgsource::lifecycle_action::pre_install)->source().has_value(),"incoming node lacks source authority");
 bool rejected=false;try{auto noexec=make_requests(false);static_cast<void>(pkgapply_exec::derive(noexec.install));}catch(const pkgapply_exec::error&e){rejected=e.code()==pkgapply_exec::error_code::missing_lifecycle_executor;}check(rejected,"missing lifecycle executor was admitted");
 {
  auto target=context(true); fixture::planning_authorities a(target.target());
  auto removal=pkgapply::package_application_request(pkgapply::removal_application_request::make(
      fixture::removal_with_control(a,pkgplan::installed_control_projection::historically_unavailable()),target,control()));
  rejected=false;try{static_cast<void>(pkgapply_exec::derive(removal));}catch(const pkgapply_exec::error&e){rejected=e.code()==pkgapply_exec::error_code::historical_control_unavailable;}
  check(rejected,"historically unavailable removal control was treated as empty");
 }
 {
  auto target=context(true); fixture::planning_authorities a(target.target());
  auto removal=pkgapply::package_application_request(pkgapply::removal_application_request::make(
      fixture::removal_with_control(a,fixture::historical_control("application/x-foreign")),target,control()));
  rejected=false;try{static_cast<void>(pkgapply_exec::derive(removal));}catch(const pkgapply_exec::error&e){rejected=e.code()==pkgapply_exec::error_code::unsupported_program_format;}
  check(rejected,"unsupported historical lifecycle format was admitted");
 }

 auto base=fs::temp_directory_path()/fs::path("libpkgapply-exec-test-"+std::to_string(::getpid()));fs::remove_all(base);fs::create_directories(base/"exec");fs::create_directories(base/"target");
 pkgapply_exec::lifecycle_session_paths paths{exec_identity<pkgexec::root_view_identity>(2),base/"exec",req.install.target().root_view(),base/"target",base/"session"};
 pkgapply_exec::lifecycle_execution_identity who{exec_identity<pkgexec::interpreter_identity>(3),static_cast<std::uint64_t>(::geteuid()),static_cast<std::uint64_t>(::getegid()),{77,66}};
 rejected=false;try{static_cast<void>(pkgapply_exec::admitted_lifecycle_session::admit(req.install,*upgrade.find(pkgapply_exec::lifecycle_subject::installed,pkgsource::lifecycle_action::pre_remove),paths,who));}catch(const pkgapply_exec::error&e){rejected=e.code()==pkgapply_exec::error_code::request_node_mismatch;}check(rejected,"foreign lifecycle node was admitted");
 auto overlap=paths;overlap.session_root=base/"target"/"session";
 rejected=false;try{static_cast<void>(pkgapply_exec::admitted_lifecycle_session::admit(req.install,*install_pre,overlap,who));}catch(const pkgapply_exec::error&e){rejected=e.code()==pkgapply_exec::error_code::invalid_effect_coordinate;}check(rejected,"overlapping lifecycle effect coordinates were admitted");
 auto invalid_who=who;invalid_who.user_id=static_cast<std::uint64_t>(std::numeric_limits<uid_t>::max());
 rejected=false;try{static_cast<void>(pkgapply_exec::admitted_lifecycle_session::admit(req.install,*install_pre,paths,invalid_who));}catch(const pkgapply_exec::error&e){rejected=e.code()==pkgapply_exec::error_code::invalid_execution_identity;}check(rejected,"unrepresentable lifecycle credentials were admitted");
 auto duplicate_who=who;duplicate_who.supplementary_groups={66,66};
 rejected=false;try{static_cast<void>(pkgapply_exec::admitted_lifecycle_session::admit(req.install,*install_pre,paths,duplicate_who));}catch(const pkgapply_exec::error&e){rejected=e.code()==pkgapply_exec::error_code::invalid_execution_identity;}check(rejected,"duplicate supplementary lifecycle groups were admitted");
 auto session=pkgapply_exec::admitted_lifecycle_session::admit(req.install,*install_pre,paths,who);
 check(session.execution_identity().supplementary_groups==std::vector<std::uint64_t>({66,77}),"supplementary lifecycle groups are not canonical");
 auto prepared=pkgapply_exec::prepare(session);
 check(prepared.request.purpose().kind()==pkgexec::execution_purpose_kind::lifecycle && prepared.request.purpose().action()==pkgsource::lifecycle_action::pre_install,"execution purpose lost lifecycle action");
 check(prepared.request.environment().network()==pkgexec::network_policy::denied,"lifecycle execution must deny networking");
 const auto target_slot=pkgexec::resource_slot::singleton(pkgexec::resource_role::managed_target_root);
 check(prepared.request.resources().binding(target_slot).mount_point().string()=="/target","managed target mount changed");
 check(prepared.resources.materialization(prepared.request.resources().binding(target_slot).resource()).host_path()==base/"target","target host coordinate changed");
 fs::remove_all(base/"session");
 backend ok(backend::mode::success);auto success=pkgapply_exec::execute(session,ok);check(success.succeeded()&&success.execution().request()==prepared.request,"successful lifecycle evidence not retained");
 fs::remove_all(base/"session");
 backend fail(backend::mode::failure);auto failure=pkgapply_exec::execute(session,fail);check(!failure.succeeded()&&failure.execution().status()==pkgexec::execution_status::failed,"failed lifecycle execution was promoted");
 fs::remove_all(base/"session");
 backend bad(backend::mode::throws);rejected=false;try{static_cast<void>(pkgapply_exec::execute(session,bad));}catch(const pkgapply_exec::error&e){rejected=e.code()==pkgapply_exec::error_code::backend_contract_violation;}check(rejected,"backend exception was not classified as contract violation");
 fs::remove_all(base/"session");
 const auto* install_post=install.find(pkgapply_exec::lifecycle_subject::incoming,pkgsource::lifecycle_action::post_install);
 auto foreign_paths=paths;foreign_paths.session_root=base/"foreign-session";
 auto foreign_session=pkgapply_exec::admitted_lifecycle_session::admit(req.install,*install_post,foreign_paths,who);
 auto foreign_prepared=pkgapply_exec::prepare(foreign_session);fs::remove_all(base/"foreign-session");
 backend foreign(backend::mode::foreign,foreign_prepared.request);rejected=false;try{static_cast<void>(pkgapply_exec::execute(session,foreign));}catch(const pkgapply_exec::error&e){rejected=e.code()==pkgapply_exec::error_code::backend_contract_violation;}check(rejected,"foreign backend evidence was admitted");
 fs::remove_all(base);return 0;
}
