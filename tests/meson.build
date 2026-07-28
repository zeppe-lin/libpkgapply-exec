# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
lifecycle_test = executable('lifecycle-test', 'lifecycle_test.cpp', dependencies: libpkgapply_exec_dep)
test('lifecycle authority', lifecycle_test)
headers_test = executable('public-headers-test', 'public_headers.cpp', dependencies: libpkgapply_exec_dep)
test('public headers', headers_test)
