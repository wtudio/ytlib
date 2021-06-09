/**
 * @file tstring.hpp
 * @brief 提供对wchar_t的相关支持
 * @details 提供对wchar_t的相关支持。实际编程中都使用char降低编程难度，此文件仅供与只支持wchar的接口交互
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include "string_tools_exports.h"

#include <locale>
#include <string>

namespace ytlib {

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

STRING_TOOLS_API std::string ToString(const wchar_t* src, std::size_t size, const std::locale& loc);

STRING_TOOLS_API std::wstring ToWString(const char* src, std::size_t size, const std::locale& loc);

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

#if defined(UNICODE)
  #define T_TEXT(STRING) L##STRING
  #define T_STR_TO_TSTR(STRING) ytlib::ToWString(STRING)
  #define T_TSTR_TO_STR(STRING) ytlib::ToString(STRING)
#else
  #define T_TEXT(STRING) STRING
  #define T_STR_TO_TSTR
  #define T_TSTR_TO_STR
#endif

}  // namespace ytlib
