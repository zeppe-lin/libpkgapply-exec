# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
project(
  'libpkgapply-exec', 'cpp',
  version: '2.0.0',
  license: 'GPL-3.0-or-later',
  meson_version: '>=1.3.0',
  default_options: ['cpp_std=c++17', 'warning_level=3', 'default_library=shared', 'b_ndebug=if-release'],
)

library_kind = get_option('default_library')
if library_kind == 'both'
  error('libpkgapply-exec requires separate shared and static builds')
endif
want_static = get_option('link_mode') == 'static'
if (library_kind == 'static') != want_static
  error('default_library and link_mode must both select shared or static')
endif

libpkgapply_dep = dependency(
  'libpkgapply', version: ['>=3.0.0', '<4.0.0'], required: true, static: want_static,
  fallback: ['libpkgapply', 'libpkgapply_dep'],
)
libpkgexec_dep = dependency(
  'libpkgexec', version: ['>=2.0.0', '<3.0.0'], required: true, static: want_static,
  fallback: ['libpkgexec', 'libpkgexec_dep'],
)
pkgconf = import('pkgconfig')
subdir('src')
if not get_option('tests').disabled()
  subdir('tests')
endif
subdir('man')
