#include <ytlib/Common/TString.h>
#include <vector>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <cctype>
#include <algorithm>
#include <iterator>

namespace ytlib
{
#if defined(UNICODE) 
	tostream & tcout = std::wcout;
	tostream & tcerr = std::wcerr;
#else
	tostream & tcout = std::cout;
	tostream & tcerr = std::cerr;
#endif // UNICODE

#ifdef UNICODE
	static void clear_mbstate(std::mbstate_t& mbs) {
		std::memset(&mbs, 0, sizeof(std::mbstate_t));
	}

	static void tostring_internal(std::string & outstr, const wchar_t * src, std::size_t size, std::locale const & loc) {
		if (size == 0) {
			outstr.clear();
			return;
		}

		typedef std::codecvt<wchar_t, char, std::mbstate_t> CodeCvt;
		const CodeCvt & cdcvt = std::use_facet<CodeCvt>(loc);
		std::mbstate_t state;
		clear_mbstate(state);

		wchar_t const * from_first = src;
		std::size_t const from_size = size;
		wchar_t const * const from_last = from_first + from_size;
		wchar_t const * from_next = from_first;

		std::vector<char> dest(from_size);

		char * to_first = &dest.front();
		std::size_t to_size = dest.size();
		char * to_last = to_first + to_size;
		char * to_next = to_first;

		CodeCvt::result ret;
		std::size_t converted = 0;
		while (from_next != from_last) {
			ret = cdcvt.out(
				state, from_first, from_last,
				from_next, to_first, to_last,
				to_next);
			// XXX: Even if only half of the input has been converted the
			// in() method returns CodeCvt::ok with VC8. I think it should
			// return CodeCvt::partial.
			if ((ret == CodeCvt::partial || ret == CodeCvt::ok) && from_next != from_last) {
				to_size = dest.size() * 2;
				dest.resize(to_size);
				converted = to_next - to_first;
				to_first = &dest.front();
				to_last = to_first + to_size;
				to_next = to_first + converted;
			}
			else if (ret == CodeCvt::ok && from_next == from_last) {
				break;
			}
			else if (ret == CodeCvt::error	&& to_next != to_last && from_next != from_last) {
				clear_mbstate(state);
				++from_next;
				from_first = from_next;
				*to_next = '?';
				++to_next;
				to_first = to_next;
			}
			else
				break;
		}
		converted = to_next - &dest[0];

		outstr.assign(dest.begin(), dest.begin() + converted);
	}

	static void towstring_internal(std::wstring& outstr, const char* src, std::size_t size, std::locale const& loc) {
		if (size == 0) {
			outstr.clear();
			return;
		}

		typedef std::codecvt<wchar_t, char, std::mbstate_t> CodeCvt;
		const CodeCvt & cdcvt = std::use_facet<CodeCvt>(loc);
		std::mbstate_t state;
		clear_mbstate(state);

		char const * from_first = src;
		std::size_t const from_size = size;
		char const * const from_last = from_first + from_size;
		char const * from_next = from_first;

		std::vector<wchar_t> dest(from_size);

		wchar_t * to_first = &dest.front();
		std::size_t to_size = dest.size();
		wchar_t * to_last = to_first + to_size;
		wchar_t * to_next = to_first;

		CodeCvt::result ret;
		std::size_t converted = 0;
		while (true) {
			ret = cdcvt.in(
				state, from_first, from_last,
				from_next, to_first, to_last,
				to_next);
			// XXX: Even if only half of the input has been converted the
			// in() method returns CodeCvt::ok. I think it should return
			// CodeCvt::partial.
			if ((ret == CodeCvt::partial || ret == CodeCvt::ok) && from_next != from_last) {
				to_size = dest.size() * 2;
				dest.resize(to_size);
				converted = to_next - to_first;
				to_first = &dest.front();
				to_last = to_first + to_size;
				to_next = to_first + converted;
				continue;
			}
			else if (ret == CodeCvt::ok && from_next == from_last) {
				break;
			}
			else if (ret == CodeCvt::error && to_next != to_last && from_next != from_last) {
				clear_mbstate(state);
				++from_next;
				from_first = from_next;
				*to_next = L'?';
				++to_next;
				to_first = to_next;
			}
			else {
				break;
			}

		}
		converted = to_next - &dest[0];

		outstr.assign(dest.begin(), dest.begin() + converted);
	}

	std::string ToString(const std::wstring& str) {
		std::string ret;
		tostring_internal(ret, str.c_str(), str.size(), std::locale());
		return ret;
	}

	std::string ToString(wchar_t const* str) {
		std::string ret;
		tostring_internal(ret, str, std::wcslen(str), std::locale());
		return ret;
	}



	std::wstring ToWString(const std::string& str) {
		std::wstring ret;
		towstring_internal(ret, str.c_str(), str.size(), std::locale());
		return ret;
	}

	std::wstring ToWString(char const* str) {
		std::wstring ret;
		towstring_internal(ret, str, std::strlen(str), std::locale());
		return ret;
	}
#endif
	struct toupper_func {
		tchar
			operator () (tchar ch) const {
			return std::char_traits<tchar>::to_char_type(
#ifdef UNICODE
				std::towupper
#else
				std::toupper
#endif
				(std::char_traits<tchar>::to_int_type(ch)));
		}
	};

	struct tolower_func {
		tchar
			operator () (tchar ch) const {
			return std::char_traits<tchar>::to_char_type(
#ifdef UNICODE
				std::towlower
#else
				std::tolower
#endif
				(std::char_traits<tchar>::to_int_type(ch)));
		}
	};

	tstring ToUpper(const tstring& str) {
		tstring ret;
		std::transform(str.begin(), str.end(), std::back_inserter(ret), toupper_func());
		return ret;
	}

	tchar ToUpper(tchar ch) {
		return toupper_func()(ch);
	}

	tstring ToLower(const tstring& str) {
		tstring ret;
		std::transform(str.begin(), str.end(), std::back_inserter(ret), tolower_func());
		return ret;
	}

	tchar ToLower(tchar ch) {
		return tolower_func()(ch);
	}
}
