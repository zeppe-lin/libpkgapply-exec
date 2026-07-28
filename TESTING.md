# Testing

The test suite proves:

- installation, upgrade, and removal derive the exact subject/action sets;
- incoming program bytes remain bound to their source snapshot;
- installed removal bytes remain bound to historical installed control;
- unavailable historical control is refused rather than treated as empty;
- unsupported durable program formats are refused;
- node-set order is deterministic identity order, not a schedule;
- missing lifecycle-executor authority is refused;
- session admission binds request, node, target root, and effect coordinates;
- unrepresentable or non-canonical numeric credentials are refused;
- execution requests use lifecycle purpose, denied networking, exact target
  mounting, fixed credentials, and complete capture;
- backend failure evidence is retained, foreign evidence is refused, and
  backend exceptions are rejected.

Run both dependency closures:

```sh
./ci/qualify.sh shared
./ci/qualify.sh static
```

Sanitizer builds should instrument the adapter and tests normally. Tests use an
in-process backend fixture; Linux namespace and pidfd realization belongs to
`libpkgexec-linux` qualification.
