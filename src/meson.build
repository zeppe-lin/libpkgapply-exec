# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
public_inc = include_directories('../include')

elf_export_generator = find_program(
  meson.project_source_root() / 'tools/generate-elf-export-script.sh',
)
elf_export_script_name = 'libpkgapply-exec.exports.map'
elf_export_script_path = meson.current_build_dir() / elf_export_script_name
elf_export_script = configure_file(
  input: '../abi/libpkgapply-exec.exports',
  output: elf_export_script_name,
  command: [elf_export_generator, '@INPUT@'],
  capture: true,
)

library_link_args = ['-Wl,--version-script=' + elf_export_script_path]
library_link_depends = [elf_export_script]
public_headers = files(
  '../include/libpkgapply-exec/error.h',
  '../include/libpkgapply-exec/model.h',
  '../include/libpkgapply-exec/derive.h',
  '../include/libpkgapply-exec/executor.h',
  '../include/libpkgapply-exec/result_codec.h',
  '../include/libpkgapply-exec/libpkgapply-exec.h',
)
public_deps = [libpkgapply_dep, libpkgexec_dep]
libpkgapply_exec = library(
  'pkgapply-exec', ['error.cpp', 'model.cpp', 'derive.cpp', 'execution_request.cpp', 'executor.cpp', 'result_codec.cpp'],
  include_directories: public_inc, dependencies: public_deps,
  link_args: library_link_args, link_depends: library_link_depends,
  install: true, version: meson.project_version(), soversion: '2',
)
libpkgapply_exec_dep = declare_dependency(
  link_with: libpkgapply_exec, include_directories: public_inc, dependencies: public_deps,
)
meson.override_dependency('libpkgapply-exec', libpkgapply_exec_dep)
install_headers(public_headers, subdir: 'libpkgapply-exec')
pkgconf.generate(
  libpkgapply_exec, name: 'libpkgapply-exec', filebase: 'libpkgapply-exec',
  description: 'Exact package lifecycle execution adapter', subdirs: '.',
  requires: ['libpkgapply >= 3.0.0, libpkgapply < 4.0.0', 'libpkgexec >= 1.4.0, libpkgexec < 2.0.0'],
)
