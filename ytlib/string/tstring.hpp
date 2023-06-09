/**
 * @file tstring.hpp
 * @author WT
 * @brief 提供对wchar_t的相关支持
 * @note 提供对wchar_t的相关支持。实际编程中都使用char降低编程难度，此文件仅供与只支持wchar的接口交互
 * @date 2019-07-26
 */
#pragma once

#include <algorithm>
#include <cctype>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <iterator>
#include <locale>
#include <string>
#include <vector>

namespace ytlib {

#if defined(USE_WCHAR)
using tchar = wchar_t;
#else
using tchar = char;
#endif

using tstring = std::basic_string<tchar>;

using tostream = std::basic_ostream<tchar>;
using tistream = std::basic_istream<tchar>;
using tostringstream = std::basic_ostringstream<tchar>;
using tistringstream = std::basic_istringstream<tchar>;
using tifstream = std::basic_ifstream<tchar>;
using tofstream = std::basic_ofstream<tchar>;

#if defined(USE_WCHAR)
  #define tcout std::wcout
  #define tcerr std::wcerr
  #define to_tstring std::to_wstring

#else
  #define tcout std::cout
  #define tcerr std::cerr
  #define to_tstring std::to_string

#endif  // USE_WCHAR

/**
 * @brief wstring转string
 *
 * @param[in] src wstring字符串指针
 * @param[in] size wstring字符串长度
 * @param[in] loc
 * @return std::string string结果
 */
inline std::string ToString(const wchar_t* src, size_t size, const std::locale& loc) {
  if (size == 0) return "";

  const std::codecvt<wchar_t, char, std::mbstate_t>& cdcvt = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
  std::mbstate_t state;
  std::memset(&state, 0, sizeof(std::mbstate_t));

  wchar_t const* from_first = src;
  size_t const from_size = size;
  wchar_t const* const from_last = from_first + from_size;
  wchar_t const* from_next = from_first;

  std::vector<char> dest(from_size);

  char* to_first = &dest.front();
  size_t to_size = dest.size();
  char* to_last = to_first + to_size;
  char* to_next = to_first;

  std::codecvt<wchar_t, char, std::mbstate_t>::result ret;
  size_t converted = 0;
  while (from_next != from_last) {
    ret = cdcvt.out(state, from_first, from_last, from_next, to_first, to_last, to_next);
    if ((ret == std::codecvt<wchar_t, char, std::mbstate_t>::partial || ret == std::codecvt<wchar_t, char, std::mbstate_t>::ok) && from_next != from_last) {
      to_size = dest.size() * 2;
      dest.resize(to_size);
      converted = to_next - to_first;
      to_first = &dest.front();
      to_last = to_first + to_size;
      to_next = to_first + converted;
    } else if (ret == std::codecvt<wchar_t, char, std::mbstate_t>::ok && from_next == from_last) {
      break;
    } else if (ret == std::codecvt<wchar_t, char, std::mbstate_t>::error && to_next != to_last && from_next != from_last) {
      std::memset(&state, 0, sizeof(std::mbstate_t));
      ++from_next;
      from_first = from_next;
      *to_next = '?';
      ++to_next;
      to_first = to_next;
    } else
      break;
  }
  converted = to_next - &dest[0];

  return std::string(dest.begin(), dest.begin() + converted);
}

/**
 * @brief string转wstring
 *
 * @param src string字符串指针
 * @param size string字符串长度
 * @param loc
 * @return std::wstring wstring结果
 */
inline std::wstring ToWString(const char* src, size_t size, const std::locale& loc) {
  if (size == 0) return L"";

  const std::codecvt<wchar_t, char, std::mbstate_t>& cdcvt = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
  std::mbstate_t state;
  std::memset(&state, 0, sizeof(std::mbstate_t));

  char const* from_first = src;
  size_t const from_size = size;
  char const* const from_last = from_first + from_size;
  char const* from_next = from_first;

  std::vector<wchar_t> dest(from_size);

  wchar_t* to_first = &dest.front();
  size_t to_size = dest.size();
  wchar_t* to_last = to_first + to_size;
  wchar_t* to_next = to_first;

  std::codecvt<wchar_t, char, std::mbstate_t>::result ret;
  size_t converted = 0;
  while (true) {
    ret = cdcvt.in(state, from_first, from_last, from_next, to_first, to_last, to_next);
    if ((ret == std::codecvt<wchar_t, char, std::mbstate_t>::partial || ret == std::codecvt<wchar_t, char, std::mbstate_t>::ok) && from_next != from_last) {
      to_size = dest.size() * 2;
      dest.resize(to_size);
      converted = to_next - to_first;
      to_first = &dest.front();
      to_last = to_first + to_size;
      to_next = to_first + converted;
      continue;
    } else if (ret == std::codecvt<wchar_t, char, std::mbstate_t>::ok && from_next == from_last) {
      break;
    } else if (ret == std::codecvt<wchar_t, char, std::mbstate_t>::error && to_next != to_last && from_next != from_last) {
      std::memset(&state, 0, sizeof(std::mbstate_t));
      ++from_next;
      from_first = from_next;
      *to_next = L'?';
      ++to_next;
      to_first = to_next;
    } else {
      break;
    }
  }
  converted = to_next - &dest[0];

  return std::wstring(dest.begin(), dest.begin() + converted);
}

inline std::string ToString(const std::wstring& str) {
  return ToString(str.c_str(), str.size(), std::locale());
}

inline std::string ToString(const wchar_t* str) {
  return ToString(str, std::wcslen(str), std::locale());
}

inline std::wstring ToWString(const std::string& str) {
  return ToWString(str.c_str(), str.size(), std::locale());
}

inline std::wstring ToWString(const char* str) {
  return ToWString(str, std::strlen(str), std::locale());
}

#if defined(USE_WCHAR)
  #define T_TEXT(s) L##s
  #define T_STR_TO_TSTR(s) ytlib::ToWString(s)
  #define T_TSTR_TO_STR(s) ytlib::ToString(s)
#else
  #define T_TEXT(s) s
  #define T_STR_TO_TSTR
  #define T_TSTR_TO_STR
#endif

}  // namespace ytlib
