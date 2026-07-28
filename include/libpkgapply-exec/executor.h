// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <libpkgapply-exec/model.h>
namespace pkgapply_exec {
[[nodiscard]] prepared_execution prepare(const admitted_lifecycle_session& session);
[[nodiscard]] lifecycle_execution_result execute(const admitted_lifecycle_session& session, pkgexec::execution_backend& backend);
} // namespace pkgapply_exec
