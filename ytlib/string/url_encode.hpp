/**
 * @file url_encode.hpp
 * @author WT
 * @brief Url编解码
 * @note UrlEncode、UrlDecode
 * @date 2019-07-26
 */
#pragma once

#include <string>

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
inline std::string UrlEncode(const std::string& str, bool up = true) {
  std::string strTemp;
  size_t length = str.length();
  strTemp.reserve(length << 1);
  for (size_t i = 0; i < length; ++i) {
    if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') || (str[i] == '.') || (str[i] == '~'))
      strTemp += str[i];
    else if (str[i] == ' ')
      strTemp += '+';
    else {
      strTemp += '%';
      strTemp += ToHex((unsigned char)str[i] >> 4, up);
      strTemp += ToHex((unsigned char)str[i] & 15, up);
    }
  }
  return strTemp;
}

/**
 * @brief UrlDecode
 * 
 * @param[in] str 待解码字符串
 * @return std::string 解码后的结果字符串
 */
inline std::string UrlDecode(const std::string& str) {
  std::string strTemp;
  size_t length = str.length();
  strTemp.reserve(length);
  for (size_t i = 0; i < length; ++i) {
    if (str[i] == '+')
      strTemp += ' ';
    else if (str[i] == '%') {
      assert(i + 2 < length);
      unsigned char c = (FromHex((unsigned char)str[++i])) << 4;
      strTemp += (c | FromHex((unsigned char)str[++i]));
    } else
      strTemp += str[i];
  }
  return strTemp;
}

}  // namespace ytlib
