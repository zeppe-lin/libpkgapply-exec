# Testing

The qualification surface is separated by evidence role. A failure should say
whether lifecycle authority, execution projection, resource materialization,
backend evidence, or the durable protocol is broken instead of collapsing all
of those questions into one lifecycle omnibus.

Run both dependency closures:

```sh
./ci/qualify.sh shared
./ci/qualify.sh static
```

The Meson suites are:

- `unit`: adapter-owned value vocabulary;
- `integration`: real `libpkgapply`/planner/source authority projected through
  lifecycle derivation, admission, preparation, backend execution, and result
  evidence;
- `protocol`: canonical durable lifecycle-execution records and refusal of
  corrupt or foreign authority;
- `header`: every public header compiled independently;
- `contract`: architecture, codec, release, source-list, and test-layout
  invariants.

## Integration qualification

`derivation` proves installation, upgrade, and removal derive the exact
incoming/installed subject and action sets. Incoming program bytes stay bound
to the exact source snapshot, installed removal bytes stay bound to historical
control, unavailable historical control is not treated as empty, and foreign
durable program formats are refused.

`session-admission` proves one node belongs to the exact application request,
target-root authority is unchanged, effect coordinates are absolute and
non-overlapping, and native credentials are representable and canonical.

`request-projection` exercises the pure internal projection used by both
execution and durable decoding. It proves exact lifecycle purpose/program,
resource slots and logical mount points, closed environment variables, denied
networking, fixed credentials, complete capture, no limits, and disabled
cancellation. Host filesystem coordinates do not enter request identity.

`preparation` proves the real execution/target roots are required, the
single-use session root is protected, private temporary/home directories are
materialized with exact permissions, and concrete resources bind the target
and temporary paths selected by the caller.

`backend-contract` snapshots `backend.capabilities()` before preparation and
requires returned evidence to name both the exact prepared request and that
advertised backend profile. Standard and non-standard exceptions from either
capability observation or execution become `backend_contract_violation`.
Capability failure occurs before lifecycle scratch mutation.

`execution-outcome` proves successful, not-started failure, and started
nonzero-exit evidence remain exact `libpkgexec` evidence; this adapter does not
promote operational failure into lifecycle success.

Concrete Linux namespace, mount, pidfd, networking, and cancellation
realization belongs to `libpkgexec-linux` qualification. This repository uses
an injected backend because backend selection belongs to orchestration.

## Durable evidence codec

`result-codec-roundtrip` retains real successful, not-started-failed, and
started-failed lifecycle results and then removes every backing test directory.
Decode must reconstruct the exact execution request from the admitted session
without calling `prepare()` or touching the filesystem, and canonical re-encode
must reproduce the original bytes.

`result-codec-refusal` covers whole-record corruption, truncation, trailing
bytes, lifecycle-node substitution, execution-session substitution, and
backend-profile substitution. Identity strings alone never rehydrate authority.

Sanitizer builds should instrument the adapter, its dependency closure, and the
unit/integration/protocol executables normally.

## Installed and ABI qualification

Shared qualification compares the built DSO against the reviewed 56-symbol
`abi/libpkgapply-exec.exports` surface. The staged consumer is compiled through
installed pkg-config metadata and executes real lifecycle derivation from one
sealed application request; static qualification therefore exercises the
complete public `libpkgapply` and `libpkgexec` closure rather than taking a
function address.
