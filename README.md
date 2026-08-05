# libpkgapply-exec 2.0.0

`libpkgapply-exec` is the backend-neutral package lifecycle execution adapter
for Zeppe-Lin. It projects exact lifecycle nodes from one sealed
`libpkgapply` application request and realizes one selected node through a
caller-supplied `libpkgexec` backend.

The library binds lifecycle authority to the accepted operation plan, exact
application request, target context, target-selected lifecycle executor,
package release, action, program bytes, and either the incoming source snapshot
or durable historical installed control.

It does not choose transaction order, apply filesystem plans, acquire an outer
mutation lease, publish `libpkgstate`, resolve dependencies, construct a Linux
backend, or claim rollback of arbitrary lifecycle side effects.

## Build

```sh
meson setup build   -Ddefault_library=shared   -Dlink_mode=shared   -Dman_pages=enabled
meson compile -C build
meson test -C build --print-errorlogs
```

Shared and static dependency closures are separate builds. See `TESTING.md`.

## Durable evidence

`libpkgapply-exec` provides a versioned canonical codec for
`lifecycle_execution_result`. It embeds `libpkgexec`'s execution-result record
and binds it to the exact lifecycle node. Decode requires the complete admitted
lifecycle session and backend profile; it derives the execution request without
preparing resources or touching the filesystem.
