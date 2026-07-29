#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=$1
grep -q "version: '1.0.0'" "$root/meson.build"
grep -q '^# libpkgapply-exec 1.0.0$' "$root/README.md"
grep -q '^## 1.0.0 — 2026-07-29$' "$root/HISTORY.md"
grep -q "soversion: '1'" "$root/src/meson.build"
grep -q "libpkgapply >= 2.0.0" "$root/src/meson.build"
grep -q "libpkgexec >= 1.3.0" "$root/src/meson.build"
