# Contributing

Keep changes contract-first and dependency-direction preserving. Every semantic
change requires tests and documentation. Do not add compatibility behavior for
Pkgfile, pkgmk, fakeroot, the legacy package database, or ambient shell state to
the native authority.

Patch series should apply with `git am`, keep commits independently reviewable,
and include exact qualification results.
