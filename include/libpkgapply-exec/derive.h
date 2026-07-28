// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <libpkgapply-exec/model.h>
namespace pkgapply_exec {
/*! Derive the complete canonical lifecycle-node set. Node vector order is identity order, never transaction execution order. */
[[nodiscard]] lifecycle_node_set derive(const pkgapply::package_application_request& request);
} // namespace pkgapply_exec
