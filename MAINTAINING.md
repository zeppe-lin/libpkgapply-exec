# Maintaining

## Boundary discipline

Lifecycle programs must come only from the sealed incoming source snapshot or
the accepted plan's durable historical removal control. Do not add an API that
accepts free-standing program bytes, recipe paths, current collection state, or
installed-control reconstruction.

Do not turn canonical node-set order into transaction order. Ordering relative
to filesystem application, state publication, other packages, and failure
continuation belongs to the effectful controller.

The adapter must remain backend-neutral and must not depend on
`libpkgexec-linux`.

`admitted_lifecycle_session` retains `libpkgapply::package_application_request`
by value, while `admitted_lifecycle_session`, `prepared_execution`, and
`lifecycle_execution_result` retain `libpkgexec` values by value. A future
application or execution ABI generation therefore requires an explicit
carrier-layout review before the accepted dependency interval is widened. The
application-4 rebind preserves every retained application and public adapter
layout, so provider SONAME 3 remains valid. Shared qualification must prove the
actual `libpkgapply-exec.so.3` product names `libpkgapply.so.4` and
`libpkgexec.so.2`, and refuses obsolete owner generations.

## Release qualification

Before tagging:

```sh
./ci/qualify.sh shared
./ci/qualify.sh static
```

Review public dependency metadata, SONAME, generated manuals, installed-header
consumers, and the authority-contract test. The shared generation-3 release
surface is the reviewed 56-symbol `abi/libpkgapply-exec.exports` manifest; GCC
and Clang shared builds must match it exactly. Keep diagnostics and host paths
out of semantic identities.

Durable evidence codecs may embed canonical records from subordinate owners,
but must require complete semantic authority bodies from the caller rather
than reconstruct them from identity strings. Lifecycle decoding must derive the
execution request purely from the admitted session and must not call resource
preparation or touch host paths.
