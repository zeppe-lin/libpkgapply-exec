// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../support/test.h"

#include <libpkgapply-exec/error.h>
#include <libpkgapply-exec/model.h>

int main()
{
  TEST_CHECK(pkgapply_exec::to_string(pkgapply_exec::lifecycle_subject::incoming) == "incoming");
  TEST_CHECK(pkgapply_exec::to_string(pkgapply_exec::lifecycle_subject::installed) == "installed");
  TEST_CHECK(pkgapply_exec::to_string(pkgapply_exec::error_code::backend_contract_violation) ==
             "backend-contract-violation");
  TEST_CHECK(pkgapply_exec::to_string(pkgapply_exec::error_code::authority_mismatch) ==
             "authority-mismatch");
  return 0;
}
