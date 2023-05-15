/**
 * @file base64_encode.hpp
 * @author WT
 * @brief base64编解码
 * @note Base64Encode、Base64Decode
 * @date 2023-05-15
 */

#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace ytlib {

inline std::string Base64Encode(std::string_view data) {
  if (data.empty()) return "";

  size_t data_size = data.size();

  std::string ret_str;
  ret_str.reserve(((data_size - 1) / 3 + 1) * 4);

  static constexpr const char base64_chars[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789+/";

  uint32_t offset = 0;
  uint32_t bit_stream = 0;
  for (size_t ii = 0; ii < data_size; ++ii) {
    uint8_t num_val = static_cast<uint8_t>(data[ii]);
    offset = 16 - ii % 3 * 8;
    bit_stream += num_val << offset;
    if (offset == 16) {
      ret_str += base64_chars[bit_stream >> 18 & 0x3f];
    } else if (offset == 8) {
      ret_str += base64_chars[bit_stream >> 12 & 0x3f];
    } else if (offset == 0 && ii != 3) {
      ret_str += base64_chars[bit_stream >> 6 & 0x3f];
      ret_str += base64_chars[bit_stream & 0x3f];
      bit_stream = 0;
    }
  }
  if (offset == 16) {
    ret_str += base64_chars[bit_stream >> 12 & 0x3f];
    ret_str += "==";
  } else if (offset == 8) {
    ret_str += base64_chars[bit_stream >> 6 & 0x3f];
    ret_str += '=';
  }

  return ret_str;
}

inline std::string Base64Decode(std::string_view data) {
  if (data.empty()) return "";

  size_t data_size = data.size();
  if (data_size % 4 != 0) [[unlikely]]
    throw std::runtime_error("Invalid base64 data.");

  std::string ret_str;
  ret_str.reserve(data_size / 4 * 3);

  static constexpr auto find_bit_val_func = [](char c) -> uint8_t {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return 255;
  };

  uint32_t bit_stream = 0;
  int offset = 0;
  for (size_t ii = 0; ii < data_size; ++ii) {
    uint8_t num_val = find_bit_val_func(data[ii]);
    if (num_val < 64) {
      offset = 18 - ii % 4 * 6;
      bit_stream += num_val << offset;
      if (offset == 12) {
        ret_str += static_cast<char>(bit_stream >> 16 & 0xff);
      }
      if (offset == 6) {
        ret_str += static_cast<char>(bit_stream >> 8 & 0xff);
      }
      if (offset == 0 && ii != 4) {
        ret_str += static_cast<char>(bit_stream & 0xff);
        bit_stream = 0;
      }
    } else if (data[ii] != '=') {
      throw std::runtime_error("Invalid base64 data.");
    }
  }

  return ret_str;
}

}  // namespace ytlib