#include <gtest/gtest.h>

#include "string_algs.hpp"
#include "string_util.hpp"
#include "url_encode.hpp"

using std::pair;
using std::string;
using std::vector;

namespace ytlib {

TEST(STRING_TEST, DEMO_TEST) {
  //kmp
  string ss = "abcdef abcdefg abcdefgh";
  ASSERT_EQ(KMP(ss, "abcdef"), 0);
  ASSERT_EQ(KMP(ss, "abcdefg"), 7);
  ASSERT_EQ(KMP(ss, "abcdefgh"), 15);
  ASSERT_EQ(KMP(ss, "aaaa"), ss.length());

  //StrDif
  ASSERT_EQ(StrDif("abcdxfg", "abcdefg"), 1);
  ASSERT_EQ(StrDif("abcdfg", "abcdefg"), 1);
  ASSERT_EQ(StrDif("abcdfg", "abcdef"), 2);

  //LongestSubStrWithoutDup
  pair<std::size_t, std::size_t> re = LongestSubStrWithoutDup("arabcacfr");
  ASSERT_EQ(re.first, 1);
  ASSERT_EQ(re.second, 4);

  //replaceAll
  string s = "abc123abc123abc123abc123abc123abc12";
  int len = s.size();
  const char* p = s.c_str();
  replaceAll(s, "abc", "yhn");
  EXPECT_STREQ(s.c_str(), "yhn123yhn123yhn123yhn123yhn123yhn12");
  ASSERT_EQ(s.size(), len);
  ASSERT_EQ(s.c_str(), p);

  replaceAll(s, "123", "45");
  EXPECT_STREQ(s.c_str(), "yhn45yhn45yhn45yhn45yhn45yhn12");
  ASSERT_LT(s.size(), len);
  ASSERT_EQ(s.c_str(), p);

  replaceAll(s, "45", "6789000");
  EXPECT_STREQ(s.c_str(), "yhn6789000yhn6789000yhn6789000yhn6789000yhn6789000yhn12");
  ASSERT_GT(s.size(), len);
  ASSERT_NE(s.c_str(), p);

  replaceAll(s, "00", "#");
  EXPECT_STREQ(s.c_str(), "yhn6789#0yhn6789#0yhn6789#0yhn6789#0yhn6789#0yhn12");

  //splitAll
  vector<string> re2 = splitAll("&123%$#456%$#7890#$%", "&%$#");
  vector<string> answer2{"123", "456", "7890"};
  ASSERT_EQ(re2.size(), answer2.size());
  for (std::size_t ii = 0; ii < re2.size(); ++ii) {
    EXPECT_STREQ(re2[ii].c_str(), answer2[ii].c_str());
  }

  string s2 = "http://abc123.com/aaa/bbbb?qa=1&qb=adf";
  printf("%lld\t%lld\t%X\t%s\n", s2.size(), s2.capacity(), static_cast<const void*>(s2.c_str()), s2.c_str());

  string s3 = UrlEncode(s2, false);
  printf("%lld\t%lld\t%X\t%s\n", s3.size(), s3.capacity(), static_cast<const void*>(s3.c_str()), s3.c_str());

  string s4 = UrlDecode(s3);
  printf("%lld\t%lld\t%X\t%s\n", s4.size(), s4.capacity(), static_cast<const void*>(s4.c_str()), s4.c_str());
}
}  // namespace ytlib
