# History

## 1.0.0 — 2026-07-29

- Rebuilt lifecycle execution sessions against `libpkgapply 2.0.0` and
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
