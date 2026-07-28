#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=$1
model=$root/include/libpkgapply-exec/model.h
derive=$root/src/derive.cpp
executor=$root/src/executor.cpp
meson=$root/meson.build

grep -q 'pkgapply::package_application_request' "$model"
grep -q 'pkgapply::lifecycle_executor_identity' "$model"
grep -q 'pkgexec::execution_backend' "$root/include/libpkgapply-exec/executor.h"
! grep -R -q 'libpkgexec-linux' "$root/include" "$root/src" "$meson"

grep -q 'incoming.build().request().source()' "$derive"
grep -q 'plan().inputs().old_control()' "$derive"
grep -q 'plan().inputs().control()' "$derive"
grep -q 'is_known(control.completeness().removal_lifecycle)' "$derive"
grep -q 'text/x-posix-shell' "$derive"

grep -q 'resource_role::managed_target_root' "$executor"
grep -q 'logical_path::parse("/target")' "$executor"
grep -q 'network_policy::denied' "$executor"
grep -q 'execution_purpose::lifecycle' "$executor"
grep -q 'execution backend returned evidence for another request' "$executor"
grep -q 'numeric_limits<uid_t>::max' "$root/src/model.cpp"
grep -q 'supplementary lifecycle groups must be unique' "$root/src/model.cpp"

if grep -R -E 'pkgapply::apply\(|pkgstate|system\(|popen\(|execl?p?\(' "$root/include" "$root/src" >/dev/null; then
  echo 'application, state, or external utility authority entered lifecycle adapter' >&2
  exit 1
fi
