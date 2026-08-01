# Design

## Authority

`libpkgapply-exec` owns one translation boundary:

```text
accepted libpkgapply application request
        |
        +-- incoming source snapshot lifecycle programs
        |
        `-- durable installed removal control
        |
canonical exact lifecycle-node set
        |
caller selects one node and transaction position
        |
admitted lifecycle session
        |
backend-neutral libpkgexec request and resources
        |
retained lifecycle execution evidence
```

A node binds the application-request and operation-plan identities, operation
kind, target-context and lifecycle-executor identities, incoming or installed
subject, package release, exact action and program bytes, and its source
snapshot or installed-control authority. Host paths never enter node identity.

The vector returned by `derive()` is sorted by node identity solely to make the
set deterministic. It is not an execution schedule. In particular, the library
does not decide where pre/post actions occur relative to filesystem application
or state publication, or how old-removal and incoming-install actions interleave
during upgrade.

## Derivation

Installation derives incoming `pre_install` and `post_install` nodes when those
programs exist in the exact source snapshot retained by the successful incoming
build.

Removal derives installed `pre_remove` and `post_remove` nodes only from the
accepted removal plan's historical control projection.

Upgrade derives both groups: old installed removal nodes from historical
control and incoming installation nodes from the exact source snapshot.

Historical removal control marked `historically_unavailable` is not equivalent
to known empty control. Derivation fails rather than silently omitting unknown
nodes. Version 0.1 accepts only the durable format `text/x-posix-shell`, mapping
it to `pkgsource::program_language::posix_shell` without changing bytes.

Incoming removal declarations are not executed during installation or upgrade.
They are planner control intended for durable installed state and become
authority only when a later accepted removal or upgrade plan retains them as
historical control.

## Session admission

One admitted session retains the exact application request and one node from
that request. Admission verifies:

- request, plan, operation, and target identities match the node;
- the application target binds the node's lifecycle-executor identity;
- the call-scoped target-root identity matches the application target;
- execution root, managed target root, and session root paths are absolute,
  normalized, and non-overlapping;
- numeric credentials fit native identifier types, supplementary groups are
  unique, and their retained order is canonical.

The execution root is a caller-supplied environment containing the exact
interpreter. The managed target is a separate writable resource mounted at
`/target`. The session root is single-use scratch authority and must be absent
or an empty real directory when preparation begins.

## Execution request

Version 0.1 seals:

- lifecycle purpose bound to the node's exact action;
- the exact POSIX-shell program bytes;
- the caller-supplied interpreter and execution-root identities;
- a writable managed-target resource mounted at `/target`;
- a private writable temporary resource mounted at `/tmp`;
- working directory `/target`;
- a closed C.UTF-8/UTC environment;
- denied networking, closed stdin, and complete stdout/stderr capture;
- fixed numeric credentials with `no_new_privileges`;
- no resource limits and disabled cancellation.

The closed environment additionally defines:

```text
ZEPPE_LIN_TARGET_ROOT=/target
ZEPPE_LIN_PACKAGE_NAME
ZEPPE_LIN_PACKAGE_VERSION
ZEPPE_LIN_PACKAGE_RELEASE
ZEPPE_LIN_LIFECYCLE_ACTION
ZEPPE_LIN_LIFECYCLE_SUBJECT
```

These variables are execution policy, not ambient inheritance.

## Evidence and failure

The adapter catches backend exceptions and classifies them as backend-contract
violations. Returned evidence must belong to the exact prepared request.
Successful lifecycle evidence is only successful `libpkgexec` evidence; a
nonzero exit, signal, unsupported backend, cleanup failure, or capture failure
remains failed.

Execution diagnostics remain inside `libpkgexec` operational evidence. The
adapter result identity binds only the exact lifecycle-node identity and
execution-evidence identity.

## Exclusions

The library does not:

- invoke `libpkgapply::apply()`;
- mutate or publish `libpkgstate`;
- choose transaction-node ordering or continuation policy;
- reconstruct unavailable installed control from current sources;
- execute caller-supplied scripts beside the accepted request;
- grant network access;
- claim recovery or rollback for lifecycle-created side effects;
- depend on `libpkgexec-linux`.

## Durable lifecycle-execution evidence

The durable record belongs to this adapter because it binds one exact lifecycle
node to the corresponding `libpkgexec` process evidence. The record embeds the
canonical `libpkgexec 1.4` execution-result encoding and adds only
adapter-owned identities.

The record does not serialize an application request, lifecycle node, admitted
session, execution request, backend profile, host path, credential policy,
resource materialization, or backend semantics. Decode requires the exact
`admitted_lifecycle_session` and `pkgexec::backend_capability_profile` bodies
from their owning authorities. Identity strings alone are not rehydration
authority.

Decode derives the exact execution request from the admitted session through a
pure internal projection. It does not call `prepare()`, create a session root,
inspect the execution or target roots, change ownership, construct execution
resources, invoke a backend, or otherwise touch the filesystem. The embedded
execution record must reopen under that derived request and the supplied
backend profile.

The retained lifecycle-result identity is recomputed from the exact node and
execution-evidence identities. Every accepted record is re-encoded and must
reproduce its original bytes. Diagnostic prose remains subordinate
`libpkgexec` evidence and is protected by the nested and whole-record
checksums.
