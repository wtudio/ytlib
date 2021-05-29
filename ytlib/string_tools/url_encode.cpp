/**
 * @file url_encode.cpp
 * @brief Url编解码
 * @details UrlEncode、UrlDecode
 * @author WT
 * @date 2019-07-26
 */
#include "url_encode.hpp"

#include <cassert>

namespace ytlib {

inline unsigned char ToHex(unsigned char x, bool up) {
  return x > 9 ? x + (up ? 55 : 87) : x + 48;
}

inline unsigned char FromHex(unsigned char x) {
  unsigned char y;
  if (x >= 'A' && x <= 'Z')
    y = x - 55;
  else if (x >= 'a' && x <= 'z')
    y = x - 87;
  else if (x >= '0' && x <= '9')
    y = x - '0';
  else
    assert(0);
  return y;
}

std::string UrlEncode(const std::string& str, bool up) {
  std::string strTemp;
  std::size_t length = str.length();
  strTemp.reserve(length << 1);
  for (std::size_t i = 0; i < length; ++i) {
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

std::string UrlDecode(const std::string& str) {
  std::string strTemp;
  std::size_t length = str.length();
  strTemp.reserve(length);
  for (std::size_t i = 0; i < length; ++i) {
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
