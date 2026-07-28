# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
public_inc = include_directories('../include')
public_headers = files(
  '../include/libpkgapply-exec/error.h',
  '../include/libpkgapply-exec/model.h',
  '../include/libpkgapply-exec/derive.h',
  '../include/libpkgapply-exec/executor.h',
  '../include/libpkgapply-exec/libpkgapply-exec.h',
)
public_deps = [libpkgapply_dep, libpkgexec_dep]
libpkgapply_exec = library(
  'pkgapply-exec', ['error.cpp', 'model.cpp', 'derive.cpp', 'executor.cpp'],
  include_directories: public_inc, dependencies: public_deps,
  install: true, version: meson.project_version(), soversion: '0',
)
libpkgapply_exec_dep = declare_dependency(
  link_with: libpkgapply_exec, include_directories: public_inc, dependencies: public_deps,
)
meson.override_dependency('libpkgapply-exec', libpkgapply_exec_dep)
install_headers(public_headers, subdir: 'libpkgapply-exec')
pkgconf.generate(
  libpkgapply_exec, name: 'libpkgapply-exec', filebase: 'libpkgapply-exec',
  description: 'Exact package lifecycle execution adapter', subdirs: '.',
  requires: ['libpkgapply >= 1.0.0', 'libpkgexec >= 1.2.0'],
)
