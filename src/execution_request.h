// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <libpkgapply-exec/model.h>

namespace pkgapply_exec::detail {

[[nodiscard]] pkgexec::resource_identity managed_target_resource_identity(
    const lifecycle_node& node);
[[nodiscard]] pkgexec::resource_identity temporary_resource_identity(
    const lifecycle_node& node);
[[nodiscard]] pkgexec::execution_request lifecycle_execution_request(
    const admitted_lifecycle_session& session);

} // namespace pkgapply_exec::detail
