#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=$1
grep -q "version: '0.1.0'" "$root/meson.build"
grep -q '^# libpkgapply-exec 0.1.0$' "$root/README.md"
grep -q '^## 0.1.0 — 2026-07-28$' "$root/HISTORY.md"
grep -q "soversion: '0'" "$root/src/meson.build"
grep -q "libpkgapply >= 1.0.0" "$root/src/meson.build"
grep -q "libpkgexec >= 1.2.0" "$root/src/meson.build"
