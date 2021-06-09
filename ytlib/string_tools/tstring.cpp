#include "tstring.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <iterator>
#include <vector>

namespace ytlib {

inline void clear_mbstate(std::mbstate_t& mbs) {
  std::memset(&mbs, 0, sizeof(std::mbstate_t));
}

std::string ToString(const wchar_t* src, std::size_t size, const std::locale& loc) {
  if (size == 0) return "";

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

  return std::string(dest.begin(), dest.begin() + converted);
}

std::wstring ToWString(const char* src, std::size_t size, const std::locale& loc) {
  if (size == 0) return L"";

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

  return std::wstring(dest.begin(), dest.begin() + converted);
}

}  // namespace ytlib
