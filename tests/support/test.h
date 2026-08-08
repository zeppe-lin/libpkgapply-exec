// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include <libpkgapply-exec/error.h>

#define TEST_CHECK(value) \
  do { \
    if (!(value)) { \
      std::cerr << __FILE__ << ':' << __LINE__ << ": check failed: " #value << '\n'; \
      std::exit(1); \
    } \
  } while (false)

namespace pkgapply_exec_test {

template<class Function>
void expect_error(pkgapply_exec::error_code expected, Function&& function)
{
  try {
    function();
  } catch (const pkgapply_exec::error& value) {
    if (value.code() == expected)
      return;
    throw std::runtime_error("unexpected libpkgapply-exec error code");
  }
  throw std::runtime_error("expected libpkgapply-exec error was not thrown");
}

} // namespace pkgapply_exec_test
