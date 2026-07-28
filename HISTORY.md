# History

## 0.1.0 — 2026-07-28

- Established exact lifecycle-node derivation from sealed application requests.
- Bound incoming installation actions to source snapshots and installed removal
  actions to durable historical control.
- Refused historically unavailable control and unsupported program formats.
- Added backend-neutral lifecycle execution with an explicit managed target,
  denied networking, fixed credentials, and retained execution evidence.
- Kept transaction ordering, filesystem application, recovery, and state
  publication outside the adapter.
