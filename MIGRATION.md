# Migration

## 0.1.0

This is the first release. Callers must:

1. derive the canonical lifecycle-node set from one exact application request;
2. choose one exact node and its transaction position outside this library;
3. supply a separate execution root, managed target root, single-use session
   root, exact interpreter identity, and numeric credentials;
4. execute through an injected `pkgexec::execution_backend`;
5. retain the returned execution evidence and decide transaction continuation
   and state publication externally.

Lifecycle programs see the managed target at `/target` and receive the
`ZEPPE_LIN_*` closed-environment variables documented in `DESIGN.md`.

## 0.1.0 to 1.0.0

Rebuild every consumer against `libpkgapply 3.0.0`. The lifecycle node model is
semantically unchanged, but admitted sessions retain complete application
requests by value and therefore cross an ABI boundary.

## Durable evidence records

Version 1 records introduced after 1.0.0 are new controller evidence, not a
conversion of legacy lifecycle logs. There is no importer for ad-hoc script
output or transaction journals. Callers must retain the exact admitted
lifecycle session and backend profile authorities needed for decoding.

## 1.1.0 to 2.0.0

Rebuild every consumer. `admitted_lifecycle_session` retains the corrected
`libpkgapply 3.0.0` application request by value, so the loader ABI advances to
`libpkgapply-exec.so.2`. Lifecycle-node and result-record protocols remain at
generation 1; no durable lifecycle record format changed.
