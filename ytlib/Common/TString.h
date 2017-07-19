#pragma once

#include <ytlib/Common/Util.h>
#include <string>

namespace ytlib
{
#if defined(UNICODE)
#	define T_TEXT(STRING) L##STRING
#else
#	define T_TEXT(STRING) STRING
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


#	define T_STRING_TO_TSTRING(STRING) ytlib::ToWString(STRING)
#	define T_TSTRING_TO_STRING(STRING) ytlib::ToString(STRING)
#else
#	define T_STRING_TO_TSTRING
#	define T_TSTRING_TO_STRING
#endif

	//´óÐ¡Ð´×ª»»
	tstring ToUpper(const tstring& str);
	tchar ToUpper(tchar ch);

	tstring ToLower(const tstring& str);
	tchar ToLower(tchar ch);
}



