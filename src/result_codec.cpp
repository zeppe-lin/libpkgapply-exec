// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgapply-exec/result_codec.h>

#include <libpkgapply-exec/error.h>
#include <libpkgexec/result_codec.h>

#include "execution_request.h"
#include "model_access.h"

#include <algorithm>
#include <array>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace pkgapply_exec {
namespace {

constexpr std::array<std::uint8_t, 8> encoding_magic{
    'P', 'K', 'G', 'L', 'X', 'R', '1', 0};
constexpr std::size_t checksum_size = 32U;

[[noreturn]] void inconsistent(const std::string& message)
{
  throw error(error_code::inconsistent_result, message);
}

[[noreturn]] void corrupt(const std::string& message)
{
  throw error(error_code::corrupt_encoding, message);
}

[[noreturn]] void mismatch(const std::string& message)
{
  throw error(error_code::authority_mismatch, message);
}

std::string checksum_hex(const std::vector<std::uint8_t>& value,
                         std::size_t size)
{
  return pkgexec::sha256_digest::of_bytes(std::string_view(
      reinterpret_cast<const char*>(value.data()), size)).hex();
}

class writer final {
public:
  void byte(std::uint8_t value)
  {
    output_.push_back(value);
    check_size();
  }

  void u16(std::uint16_t value)
  {
    byte(static_cast<std::uint8_t>((value >> 8U) & 0xffU));
    byte(static_cast<std::uint8_t>(value & 0xffU));
  }

  void u32(std::uint32_t value)
  {
    for (int shift = 24; shift >= 0; shift -= 8)
      byte(static_cast<std::uint8_t>((value >> shift) & 0xffU));
  }

  void raw(const std::uint8_t* data, std::size_t size)
  {
    if (size == 0U)
      return;
    if (size > maximum_lifecycle_execution_result_encoding_size -
                   output_.size())
      inconsistent("lifecycle-execution encoding exceeds maximum size");
    output_.insert(output_.end(), data, data + size);
  }

  void bytes(const std::vector<std::uint8_t>& value)
  {
    if (value.size() > std::numeric_limits<std::uint32_t>::max())
      inconsistent("embedded execution evidence is too large");
    u32(static_cast<std::uint32_t>(value.size()));
    raw(value.data(), value.size());
  }

  void identity(std::string_view value)
  {
    if (value.size() != 64U ||
        !std::all_of(value.begin(), value.end(), [](char current) {
          return (current >= '0' && current <= '9') ||
                 (current >= 'a' && current <= 'f');
        }))
      inconsistent("lifecycle-execution record contains an invalid identity");
    for (std::size_t index = 0; index < value.size(); index += 2U)
      byte(static_cast<std::uint8_t>((digit(value[index]) << 4U) |
                                     digit(value[index + 1U])));
  }

  const lifecycle_execution_result_encoding& output() const noexcept
  {
    return output_;
  }

  lifecycle_execution_result_encoding finish()
  {
    return std::move(output_);
  }

private:
  static std::uint8_t digit(char value)
  {
    return value >= '0' && value <= '9'
        ? static_cast<std::uint8_t>(value - '0')
        : static_cast<std::uint8_t>(value - 'a' + 10);
  }

  void check_size() const
  {
    if (output_.size() > maximum_lifecycle_execution_result_encoding_size)
      inconsistent("lifecycle-execution encoding exceeds maximum size");
  }

  lifecycle_execution_result_encoding output_;
};

class reader final {
public:
  reader(const lifecycle_execution_result_encoding& input, std::size_t limit)
      : input_(input), limit_(limit)
  {
  }

  std::uint8_t byte()
  {
    require(1U);
    return input_[offset_++];
  }

  std::uint16_t u16()
  {
    std::uint16_t value = 0U;
    for (int index = 0; index < 2; ++index)
      value = static_cast<std::uint16_t>((value << 8U) | byte());
    return value;
  }

  std::uint32_t u32()
  {
    std::uint32_t value = 0U;
    for (int index = 0; index < 4; ++index)
      value = (value << 8U) | byte();
    return value;
  }

  std::string identity()
  {
    static constexpr char digits[] = "0123456789abcdef";
    require(32U);
    std::string value(64U, '0');
    for (std::size_t index = 0; index < 32U; ++index) {
      const auto current = input_[offset_++];
      value[index * 2U] = digits[(current >> 4U) & 0x0fU];
      value[index * 2U + 1U] = digits[current & 0x0fU];
    }
    return value;
  }

  std::vector<std::uint8_t> bytes(std::size_t maximum)
  {
    const auto size = static_cast<std::size_t>(u32());
    if (size > maximum)
      corrupt("embedded execution evidence exceeds its limit");
    require(size);
    std::vector<std::uint8_t> value(
        input_.begin() + static_cast<std::ptrdiff_t>(offset_),
        input_.begin() + static_cast<std::ptrdiff_t>(offset_ + size));
    offset_ += size;
    return value;
  }

  void finish() const
  {
    if (offset_ != limit_)
      corrupt("lifecycle-execution encoding contains trailing payload bytes");
  }

private:
  void require(std::size_t size) const
  {
    if (offset_ > limit_ || size > limit_ - offset_)
      corrupt("lifecycle-execution encoding is truncated");
  }

  const lifecycle_execution_result_encoding& input_;
  std::size_t limit_;
  std::size_t offset_ = 0U;
};

void validate_execution_binding(const lifecycle_node& node,
                                const pkgexec::execution_request& request)
{
  if (request.program() != node.program() ||
      request.purpose().kind() != pkgexec::execution_purpose_kind::lifecycle ||
      !request.purpose().action() ||
      *request.purpose().action() != node.action())
    inconsistent("lifecycle result contains execution evidence for another node");
}

void validate_result(const lifecycle_execution_result& result)
{
  validate_execution_binding(result.node(), result.execution().request());
  const auto expected = detail::model_access::result(
      result.node(), result.execution());
  if (expected.identity() != result.identity())
    inconsistent("lifecycle result identity does not match retained evidence");
}

} // namespace

lifecycle_execution_result_encoding encode_lifecycle_execution_result(
    const lifecycle_execution_result& result)
{
  validate_result(result);

  writer output;
  output.raw(encoding_magic.data(), encoding_magic.size());
  output.u16(lifecycle_execution_result_encoding_version);
  output.identity(result.node().identity().hex());
  output.identity(result.execution().request().identity().hex());
  output.identity(result.execution().backend().identity().hex());
  output.identity(result.execution().identity().hex());
  output.identity(result.identity().hex());
  output.bytes(pkgexec::encode_execution_result(result.execution()));

  const auto& payload = output.output();
  output.identity(checksum_hex(payload, payload.size()));
  return output.finish();
}

lifecycle_execution_result decode_lifecycle_execution_result(
    const lifecycle_execution_result_encoding& encoding,
    admitted_lifecycle_session session,
    pkgexec::backend_capability_profile backend)
{
  try {
    if (encoding.size() > maximum_lifecycle_execution_result_encoding_size)
      corrupt("lifecycle-execution encoding exceeds maximum size");
    if (encoding.size() < encoding_magic.size() + 2U + checksum_size)
      corrupt("lifecycle-execution encoding is truncated");

    const auto payload_size = encoding.size() - checksum_size;
    const auto actual_checksum = checksum_hex(encoding, payload_size);
    static constexpr char digits[] = "0123456789abcdef";
    std::string retained_checksum(64U, '0');
    for (std::size_t index = 0; index < checksum_size; ++index) {
      const auto current = encoding[payload_size + index];
      retained_checksum[index * 2U] = digits[(current >> 4U) & 0x0fU];
      retained_checksum[index * 2U + 1U] = digits[current & 0x0fU];
    }
    if (retained_checksum != actual_checksum)
      corrupt("lifecycle-execution encoding checksum mismatch");

    reader input(encoding, payload_size);
    for (const auto expected : encoding_magic) {
      if (input.byte() != expected)
        corrupt("lifecycle-execution encoding has invalid magic");
    }
    if (input.u16() != lifecycle_execution_result_encoding_version)
      corrupt("lifecycle-execution encoding version is unsupported");

    const auto node_identity = input.identity();
    const auto execution_request_identity = input.identity();
    const auto backend_identity = input.identity();
    const auto execution_identity = input.identity();
    const auto result_identity = input.identity();

    if (session.node().identity().hex() != node_identity)
      mismatch("lifecycle-execution record belongs to another lifecycle node");
    if (backend.identity().hex() != backend_identity)
      mismatch("lifecycle-execution record belongs to another backend profile");

    auto execution_request = detail::lifecycle_execution_request(session);
    if (execution_request.identity().hex() != execution_request_identity)
      mismatch("lifecycle-execution record belongs to another lifecycle session");

    auto execution_encoding = input.bytes(
        pkgexec::maximum_execution_result_encoding_size);
    input.finish();

    auto execution = [&]() -> pkgexec::execution_result {
      try {
        return pkgexec::decode_execution_result(
            execution_encoding, std::move(execution_request),
            std::move(backend));
      } catch (const pkgexec::error& problem) {
        if (problem.code() == pkgexec::error_code::authority_mismatch)
          mismatch("embedded execution evidence belongs to another authority");
        corrupt("embedded execution evidence is invalid: " +
                std::string(problem.what()));
      }
    }();
    if (execution.identity().hex() != execution_identity)
      corrupt("embedded execution evidence identity mismatch");

    auto decoded = detail::model_access::result(
        session.node(), std::move(execution));
    if (decoded.identity().hex() != result_identity)
      corrupt("lifecycle result identity mismatch");
    validate_result(decoded);
    if (encode_lifecycle_execution_result(decoded) != encoding)
      corrupt("lifecycle-execution encoding is not canonical");
    return decoded;
  } catch (const error& problem) {
    if (problem.code() == error_code::corrupt_encoding ||
        problem.code() == error_code::authority_mismatch)
      throw;
    throw error(error_code::corrupt_encoding,
                "lifecycle-execution encoding violates the result contract: " +
                    std::string(problem.what()));
  } catch (const std::exception& problem) {
    throw error(error_code::corrupt_encoding,
                "lifecycle-execution encoding violates a subordinate contract: " +
                    std::string(problem.what()));
  }
}

} // namespace pkgapply_exec
