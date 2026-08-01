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

## Release qualification

Before tagging:

```sh
./ci/qualify.sh shared
./ci/qualify.sh static
```

Review public dependency metadata, SONAME, generated manuals, installed-header
consumers, and the authority-contract test. Keep diagnostics and host paths out
of semantic identities.

Durable evidence codecs may embed canonical records from subordinate owners,
but must require complete semantic authority bodies from the caller rather
than reconstruct them from identity strings. Lifecycle decoding must derive the
execution request purely from the admitted session and must not call resource
preparation or touch host paths.
