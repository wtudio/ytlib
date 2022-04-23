#include <gtest/gtest.h>

#include "url_parser.hpp"

namespace ytlib {

TEST(URL_PARSER_TEST, JoinUrl) {
  struct TestCase {
    std::string name;

    Url url;

    std::string want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .url = Url{},
      .want_result = ""});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .url = Url{
          .protocol = "http",
      },
      .want_result = "http://"});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .url = Url{
          .protocol = "http",
          .host = "127.0.0.1",
          .service = "80",
          .path = "index.html",
          .query = "key1=val1&key2=val2",
          .fragment = "all",
      },
      .want_result = "http://127.0.0.1:80/index.html?key1=val1&key2=val2#all"});

  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .url = Url{
          .path = "index.html",
          .query = "key1=val1&key2=val2",
          .fragment = "all",
      },
      .want_result = "/index.html?key1=val1&key2=val2#all"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = JoinUrl(test_cases[ii].url);
    EXPECT_STREQ(ret.c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(URL_PARSER_TEST, ParseUrl) {
  struct TestCase {
    std::string name;

    std::string url_str;

    std::optional<UrlView> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .url_str = "",
      .want_result = UrlView{
          .path = "",
      }});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .url_str = "http://127.0.0.1:80/index.html?key1=val1&key2=val2#all",
      .want_result = UrlView{
          .protocol = "http",
          .host = "127.0.0.1",
          .service = "80",
          .path = "/index.html",
          .query = "key1=val1&key2=val2",
          .fragment = "all",
      }});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .url_str = "www.abc.com:8080/index.html?key1=val1",
      .want_result = UrlView{
          .host = "www.abc.com",
          .service = "8080",
          .path = "/index.html",
          .query = "key1=val1",
      }});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .url_str = "www.abc.com:/index.html?key1=val1",
      .want_result = UrlView{
          .host = "www.abc.com",
          .service = "",
          .path = "/index.html",
          .query = "key1=val1",
      }});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .url_str = "www.abc.com/index.html?key1=val1",
      .want_result = UrlView{
          .host = "www.abc.com",
          .path = "/index.html",
          .query = "key1=val1",
      }});
  test_cases.emplace_back(TestCase{
      .name = "case 6",
      .url_str = "www.abc.com?key1=val1",
      .want_result = UrlView{
          .host = "www.abc.com",
          .path = "",
          .query = "key1=val1",
      }});
  test_cases.emplace_back(TestCase{
      .name = "case 7",
      .url_str = "xxxxx",
      .want_result = UrlView{
          .host = "xxxxx",
          .path = "",
      }});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = ParseUrl(test_cases[ii].url_str);
    EXPECT_EQ(static_cast<bool>(ret), static_cast<bool>(test_cases[ii].want_result));
    if (ret && test_cases[ii].want_result) {
      EXPECT_EQ(ret->protocol, test_cases[ii].want_result->protocol)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
      EXPECT_EQ(ret->host, test_cases[ii].want_result->host)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
      EXPECT_EQ(ret->service, test_cases[ii].want_result->service)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
      EXPECT_EQ(ret->path, test_cases[ii].want_result->path)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
      EXPECT_EQ(ret->query, test_cases[ii].want_result->query)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
      EXPECT_EQ(ret->fragment, test_cases[ii].want_result->fragment)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    }
  }
}

}  // namespace ytlib
