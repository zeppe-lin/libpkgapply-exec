// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

/*! \file result_codec.h
 *  \brief Versioned durable encoding for lifecycle execution evidence.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include <libpkgapply-exec/model.h>

namespace pkgapply_exec {

/*! \brief Current canonical lifecycle-execution result encoding. */
inline constexpr std::uint16_t lifecycle_execution_result_encoding_version = 1;
/*! \brief Hard refusal bound for one durable lifecycle-execution record. */
inline constexpr std::size_t maximum_lifecycle_execution_result_encoding_size =
    128U * 1024U * 1024U;

using lifecycle_execution_result_encoding = std::vector<std::uint8_t>;

/*! \brief Encode exact lifecycle-owned evidence into canonical bytes. */
[[nodiscard]] lifecycle_execution_result_encoding
encode_lifecycle_execution_result(const lifecycle_execution_result& result);

/*! \brief Decode evidence under exact caller-supplied session authority.
 *
 * The decoder derives the execution request from the admitted lifecycle
 * session without preparing resources or touching the filesystem. It then
 * decodes the embedded libpkgexec evidence under that exact request and the
 * supplied backend profile.
 */
[[nodiscard]] lifecycle_execution_result decode_lifecycle_execution_result(
    const lifecycle_execution_result_encoding& encoding,
    admitted_lifecycle_session session,
    pkgexec::backend_capability_profile backend);

} // namespace pkgapply_exec
