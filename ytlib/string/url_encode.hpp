/**
 * @file url_encode.hpp
 * @author WT
 * @brief Url编解码
 * @note UrlEncode、UrlDecode
 * @date 2019-07-26
 */
#pragma once

#include <string>
#include <string_view>

namespace ytlib {

inline unsigned char ToHex(unsigned char x, bool up) {
  return x > 9 ? x + (up ? 55 : 87) : x + 48;
}

inline unsigned char FromHex(unsigned char x) {
  unsigned char y = 0;
  if (x >= 'A' && x <= 'Z')
    y = x - 55;
  else if (x >= 'a' && x <= 'z')
    y = x - 87;
  else if (x >= '0' && x <= '9')
    y = x - '0';
  return y;
}

/**
 * @brief UrlEncode
 *
 * @param[in] str 待编码字符串
 * @param[in] up 是否转码为大写字符
 * @return std::string 转码后的结果字符串
 */
inline std::string UrlEncode(std::string_view str, bool up = true) {
  std::string ret_str;
  size_t len = str.length();
  ret_str.reserve(len << 1);
  for (size_t i = 0; i < len; ++i) {
    if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') || (str[i] == '.') || (str[i] == '~')) {
      ret_str += str[i];
    } else if (str[i] == ' ') {
      ret_str += '+';
    } else {
      ret_str += '%';
      ret_str += ToHex((unsigned char)str[i] >> 4, up);
      ret_str += ToHex((unsigned char)str[i] & 15, up);
    }
  }
  return ret_str;
}

/**
 * @brief UrlDecode
 *
 * @param[in] str 待解码字符串
 * @return std::string 解码后的结果字符串
 */
inline std::string UrlDecode(std::string_view str) {
  std::string ret_str;
  size_t len = str.length();
  ret_str.reserve(len);
  for (size_t i = 0; i < len; ++i) {
    if (str[i] == '+') {
      ret_str += ' ';
    } else if (str[i] == '%') {
      if (i + 2 < len) {
        unsigned char c = (FromHex((unsigned char)str[++i])) << 4;
        ret_str += (c | FromHex((unsigned char)str[++i]));
      } else {
        break;
      }
    } else {
      ret_str += str[i];
    }
  }
  return ret_str;
}

}  // namespace ytlib
