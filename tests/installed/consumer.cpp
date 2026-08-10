// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgapply-exec/libpkgapply-exec.h>

#include "../fixtures/lifecycle.h"

#include <string>

int main()
{
  const auto application = pkgapply_exec_test::requests();
  const auto nodes = pkgapply_exec::derive(application.install);
  const auto* pre = nodes.find(
      pkgapply_exec::lifecycle_subject::incoming,
      pkgsource::lifecycle_action::pre_install);
  if (pre == nullptr || !pre->source() ||
      pre->application_request() != application.install.identity() ||
      pre->plan() != application.install.plan() ||
      pre->program().material() != std::string("echo pre-install\n"))
    return 1;

  pkgapply_exec::error sample(
      pkgapply_exec::error_code::authority_mismatch,
      "installed consumer");
  return sample.code() == pkgapply_exec::error_code::authority_mismatch ? 0 : 1;
}
