// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgapply-exec/model.h>
#include <cstddef>

namespace {
template<class T, std::size_t Size, std::size_t Align>
constexpr void require_layout()
{
  static_assert(sizeof(T) == Size, "ABI size changed");
  static_assert(alignof(T) == Align, "ABI alignment changed");
}
}

int main()
{
#if defined(__x86_64__) || defined(_M_X64)
  require_layout<pkgexec::interpreter_identity, 32, 8>();
  require_layout<pkgexec::execution_request, 720, 8>();
  require_layout<pkgexec::execution_resources, 96, 8>();
  require_layout<pkgexec::execution_result, 1160, 8>();
  require_layout<pkgapply_exec::lifecycle_session_paths, 216, 8>();
  require_layout<pkgapply_exec::lifecycle_execution_identity, 72, 8>();
  require_layout<pkgapply_exec::admitted_lifecycle_session, 3448, 8>();
  require_layout<pkgapply_exec::prepared_execution, 856, 8>();
  require_layout<pkgapply_exec::lifecycle_execution_result, 1776, 8>();
#endif
  return 0;
}
