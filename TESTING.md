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
- backend failure evidence is retained; foreign request or backend-profile
  evidence is refused; capability and execution exceptions, including
  non-standard exceptions, are rejected;
- backend capability observation occurs before lifecycle scratch mutation.

Run both dependency closures:

```sh
./ci/qualify.sh shared
./ci/qualify.sh static
```

Sanitizer builds should instrument the adapter and tests normally. Tests use an
in-process backend fixture; Linux namespace and pidfd realization belongs to
`libpkgexec-linux` qualification.

## Durable evidence codec

The codec fixture produces real successful and failed lifecycle results through
the adapter, removes every backing test directory, and then decodes the records
under the retained admitted session and backend profile. This proves that
decoding derives the execution request without calling resource preparation or
touching the filesystem.

Negative cases cover whole-record corruption, truncation, substitution of the
lifecycle node, substitution of execution-session authority, and substitution
of the backend profile. Re-encoding every accepted record must reproduce the
original bytes exactly.
