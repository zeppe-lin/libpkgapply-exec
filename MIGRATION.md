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

Rebuild every consumer against `libpkgapply 2.0.0`. The lifecycle node model is
semantically unchanged, but admitted sessions retain complete application
requests by value and therefore cross an ABI boundary.
