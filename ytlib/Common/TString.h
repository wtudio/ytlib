/**
 * @file TString.h
 * @brief 提供对wchar_t的相关支持
 * @details 提供对wchar_t的相关支持。实际编程中都使用char降低编程难度，此文件仅供与只支持wchar的接口交互
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <ytlib/Common/Util.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <iterator>
#include <string>
#include <vector>

namespace ytlib {
#if defined(UNICODE)
  #define T_TEXT(STRING) L##STRING
#else
  #define T_TEXT(STRING) STRING
#endif

#if defined(UNICODE)
typedef wchar_t tchar;
#else
typedef char tchar;
#endif

typedef std::basic_string<tchar> tstring;

typedef std::basic_ostream<tchar> tostream;
typedef std::basic_istream<tchar> tistream;
typedef std::basic_ostringstream<tchar> tostringstream;
typedef std::basic_istringstream<tchar> tistringstream;
typedef std::basic_ifstream<tchar> tifstream;
typedef std::basic_ofstream<tchar> tofstream;

#if defined(UNICODE)
  #define tcout std::wcout
  #define tcerr std::wcerr
  #define to_tstring std::to_wstring
#else
  #define tcout std::cout
  #define tcerr std::cerr
  #define to_tstring std::to_string
#endif  // UNICODE

#if defined(UNICODE)

inline void clear_mbstate(std::mbstate_t& mbs) {
  std::memset(&mbs, 0, sizeof(std::mbstate_t));
}

inline void tostring_internal(std::string& outstr, const wchar_t* src, std::size_t size, std::locale const& loc) {
  if (size == 0) {
    outstr.clear();
    return;
  }

  //typedef std::codecvt<wchar_t, char, std::mbstate_t> std::codecvt<wchar_t, char, std::mbstate_t>;
  const std::codecvt<wchar_t, char, std::mbstate_t>& cdcvt = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
  std::mbstate_t state;
  clear_mbstate(state);

  wchar_t const* from_first = src;
  std::size_t const from_size = size;
  wchar_t const* const from_last = from_first + from_size;
  wchar_t const* from_next = from_first;

  std::vector<char> dest(from_size);

  char* to_first = &dest.front();
  std::size_t to_size = dest.size();
  char* to_last = to_first + to_size;
  char* to_next = to_first;

  std::codecvt<wchar_t, char, std::mbstate_t>::result ret;
  std::size_t converted = 0;
  while (from_next != from_last) {
    ret = cdcvt.out(
        state, from_first, from_last,
        from_next, to_first, to_last,
        to_next);
    // XXX: Even if only half of the input has been converted the
    // in() method returns std::codecvt<wchar_t, char, std::mbstate_t>::ok with VC8. I think it should
    // return std::codecvt<wchar_t, char, std::mbstate_t>::partial.
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
      clear_mbstate(state);
      ++from_next;
      from_first = from_next;
      *to_next = '?';
      ++to_next;
      to_first = to_next;
    } else
      break;
  }
  converted = to_next - &dest[0];

  outstr.assign(dest.begin(), dest.begin() + converted);
}

inline void towstring_internal(std::wstring& outstr, const char* src, std::size_t size, std::locale const& loc) {
  if (size == 0) {
    outstr.clear();
    return;
  }

  //typedef std::codecvt<wchar_t, char, std::mbstate_t> std::codecvt<wchar_t, char, std::mbstate_t>;
  const std::codecvt<wchar_t, char, std::mbstate_t>& cdcvt = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
  std::mbstate_t state;
  clear_mbstate(state);

  char const* from_first = src;
  std::size_t const from_size = size;
  char const* const from_last = from_first + from_size;
  char const* from_next = from_first;

  std::vector<wchar_t> dest(from_size);

  wchar_t* to_first = &dest.front();
  std::size_t to_size = dest.size();
  wchar_t* to_last = to_first + to_size;
  wchar_t* to_next = to_first;

  std::codecvt<wchar_t, char, std::mbstate_t>::result ret;
  std::size_t converted = 0;
  while (true) {
    ret = cdcvt.in(
        state, from_first, from_last,
        from_next, to_first, to_last,
        to_next);
    // XXX: Even if only half of the input has been converted the
    // in() method returns std::codecvt<wchar_t, char, std::mbstate_t>::ok. I think it should return
    // std::codecvt<wchar_t, char, std::mbstate_t>::partial.
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
      clear_mbstate(state);
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

  outstr.assign(dest.begin(), dest.begin() + converted);
}

inline std::string ToString(const std::wstring& str) {
  std::string ret;
  tostring_internal(ret, str.c_str(), str.size(), std::locale());
  return ret;
}

inline std::string ToString(wchar_t const* str) {
  std::string ret;
  tostring_internal(ret, str, std::wcslen(str), std::locale());
  return ret;
}

inline std::wstring ToWString(const std::string& str) {
  std::wstring ret;
  towstring_internal(ret, str.c_str(), str.size(), std::locale());
  return ret;
}

inline std::wstring ToWString(char const* str) {
  std::wstring ret;
  towstring_internal(ret, str, std::strlen(str), std::locale());
  return ret;
}

  #define T_STRING_TO_TSTRING(STRING) ytlib::ToWString(STRING)
  #define T_TSTRING_TO_STRING(STRING) ytlib::ToString(STRING)
#else
  #define T_STRING_TO_TSTRING
  #define T_TSTRING_TO_STRING
#endif

//对QString的支持
#if defined(UNICODE)
  #define T_QSTRING_TO_TSTRING(STRING) STRING.toStdWString()
  #define T_TSTRING_TO_QSTRING(STRING) QString::fromStdWString(STRING)
#else
  #define T_QSTRING_TO_TSTRING(STRING) STRING.toStdString()
  #define T_TSTRING_TO_QSTRING(STRING) QString::fromStdString(STRING)
#endif

}  // namespace ytlib
