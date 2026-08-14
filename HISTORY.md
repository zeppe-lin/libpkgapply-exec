# History

## 3.0.1 — 2026-08-14

- Close lifecycle execution on the source-ABI-4 runtime authority.
- Require `libpkgapply >= 3.0.1, < 4.0.0`, excluding application 3.0.0
  whose admitted build-plan interval could still select the source-3 closure.
- Require `libpkgexec >= 2.1.1, < 3.0.0`, the first execution-2 release
  whose source-4 dependency is addressable by version.
- Qualify fixture construction against `libpkgbuild-image >= 1.0.1` and the
  source-plan-2/build-plan-1.1 closure.
- Keep SONAME 3 and the lifecycle-result codec generation unchanged.

## 3.0.0 — 2026-08-12

- Made lifecycle-result evidence self-contained with the exact `libpkgexec` backend-capability-profile owner encoding that admitted its embedded execution result.
- Removed the caller-supplied backend-profile argument from durable lifecycle-result decode; historical execution authority is no longer reconstructed by an outer controller.
- Kept the durable lifecycle-result format at its first real schema generation while failing closed on incompatible development bytes.
- Advanced the SONAME to `libpkgapply-exec.so.3` for the intentional decoder ABI break and required `libpkgexec >= 2.1.0` for the profile codec.
- Requalified the reviewed 56-symbol ABI surface, protocol refusal/round-trip tests, installed consumer, and shared/static dependency closure.

## 2.0.0 — 2026-08-05

- Rebuilt admitted lifecycle sessions against the corrected opaque `libpkgapply 3.0.0` application-request ABI.
- Advanced the SONAME to `libpkgapply-exec.so.2`; the public session retains the application request by value.
- Qualified lifecycle derivation against resolver-backed build authority and the standalone build-image and build-plan boundaries.
- Preserved lifecycle-node and durable execution-evidence protocols at their first deployed generation.
- Bound runtime evidence to the backend capability profile observed before lifecycle resource preparation and translated standard/non-standard backend throws into adapter-owned contract failures.
- Split qualification into unit, integration, protocol, standalone-header, and static-contract roles.
- Froze the pre-release generation-2 ELF surface to one reviewed 56-symbol GCC/Clang-stable manifest and qualified a real installed consumer.
- Qualified the 2.0 boundary against the generation-3 `libpkgapply.so.3` runtime while retaining this library's own SONAME 2.
- Bound the unchanged lifecycle execution carriers to `libpkgexec 2.x` / `libpkgexec.so.2`; the exec1→exec2 owner correction does not advance this library's SONAME because the retained execution layouts are unchanged.

## 1.1.0 — 2026-08-02

- Added the canonical version-1 durable codec for lifecycle execution results.
- Embedded exact `libpkgexec 1.4.0` execution evidence without duplicating its schema.
- Required the complete admitted lifecycle session and backend profile during decode.
- Derived the exact execution request through a pure path that performs no resource preparation or filesystem access.
- Added corruption, truncation, node/session/backend substitution, canonical round-trip, and Meson-source-path qualification.

## 1.0.0 — 2026-07-29

- Rebuilt lifecycle execution sessions against `libpkgapply 3.0.0` and
  `libpkgexec 1.3.0`.
- Advanced the SONAME to `libpkgapply-exec.so.1` because
  `admitted_lifecycle_session` embeds the changed application request by value.
- Preserved lifecycle-node derivation, execution-resource policy, result
  evidence, and identity domains.
- Excluded generation-1 source/build/application libraries from the runtime
  closure.

## 0.1.0 — 2026-07-28

- Established exact lifecycle-node derivation from sealed application requests.
- Bound incoming installation actions to source snapshots and installed removal
  actions to durable historical control.
- Refused historically unavailable control and unsupported program formats.
- Added backend-neutral lifecycle execution with an explicit managed target,
  denied networking, fixed credentials, and retained execution evidence.
- Kept transaction ordering, filesystem application, recovery, and state
  publication outside the adapter.
