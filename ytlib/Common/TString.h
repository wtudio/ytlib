#pragma once

#include <ytlib/Common/Util.h>
#include <string>

namespace wtlib
{
#if defined(UNICODE)
#	define WT_TEXT(STRING) L##STRING
#else
#	define WT_TEXT(STRING) STRING
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

	extern tostream& tcout;
	extern tostream& tcerr;

#if defined(UNICODE)

	std::string ToString(const std::wstring& str);
	std::string ToString(wchar_t const* str);

	std::wstring ToWString(const std::string& str);
	std::wstring ToWString(char const* str);


#	define WT_STRING_TO_TSTRING(STRING) wtlib::ToWString(STRING)
#	define WT_TSTRING_TO_STRING(STRING) wtlib::ToString(STRING)
#else
#	define WT_STRING_TO_TSTRING
#	define WT_TSTRING_TO_STRING
#endif

	//´óÐ¡Ð´×ª»»
	tstring ToUpper(const tstring& str);
	tchar ToUpper(tchar ch);

	tstring ToLower(const tstring& str);
	tchar ToLower(tchar ch);
}



