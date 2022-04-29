/**
 * @file url_parser.hpp
 * @author WT
 * @brief Url解析器
 * @note Url解析器
 * @date 2022-04-22
 */
#pragma once

#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>

namespace ytlib {

/**
 * @brief url元素
 *
 */
struct UrlView {
  std::string_view protocol;  // 协议
  std::string_view host;      // host
  std::string_view service;   // 端口
  std::string_view path;      // 路径
  std::string_view query;     // 参数
  std::string_view fragment;  // 额外信息
};

/**
 * @brief url元素
 *
 */
struct Url {
  std::string protocol;  // 协议
  std::string host;      // host
  std::string service;   // 端口
  std::string path;      // 路径
  std::string query;     // 参数
  std::string fragment;  // 额外信息

  static Url FromUrlView(const UrlView& url_view) {
    return Url{
        .protocol = std::string(url_view.protocol),
        .host = std::string(url_view.host),
        .service = std::string(url_view.service),
        .path = std::string(url_view.path),
        .query = std::string(url_view.query),
        .fragment = std::string(url_view.fragment)};
  }
};

/**
 * @brief url解析
 * @note url结构：[protocol://][host][:service][path][?query][#fragment]
 * @param url_str url字符串
 * @return std::optional<UrlView> url结构，nullopt则代表解析失败
 */
inline std::optional<UrlView> ParseUrl(std::string_view url_str) {
  std::regex url_regex(
      R"(^(([^:\/?#]+)://)?(([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
      std::regex::ECMAScript);
  std::match_results<std::string_view::const_iterator> url_match_result;

  if (!std::regex_match(url_str.begin(), url_str.end(), url_match_result, url_regex)) return std::nullopt;

  UrlView url;
  if (url_match_result[2].matched) url.protocol = std::string_view(url_match_result[2].first, url_match_result[2].second);
  if (url_match_result[4].matched) {
    std::string_view auth(url_match_result[4].first, url_match_result[4].second);
    size_t pos = auth.find_first_of(':');
    if (pos != std::string_view::npos) {
      url.host = auth.substr(0, pos);
      url.service = auth.substr(pos + 1);
    } else {
      url.host = auth;
    }
  }
  if (url_match_result[5].matched) url.path = std::string_view(url_match_result[5].first, url_match_result[5].second);
  if (url_match_result[7].matched) url.query = std::string_view(url_match_result[7].first, url_match_result[7].second);
  if (url_match_result[9].matched) url.fragment = std::string_view(url_match_result[9].first, url_match_result[9].second);

  return std::optional<UrlView>{url};
}

/**
 * @brief url拼接
 * @note url结构：[protocol://][host][:service][path][?query][#fragment]
 * @param url url结构
 * @return std::string url字符串
 */
inline std::string JoinUrl(const UrlView& url) {
  std::stringstream ss;

  if (!url.protocol.empty()) ss << url.protocol << "://";
  if (!url.host.empty()) ss << url.host;
  if (!url.service.empty()) ss << ":" << url.service;
  if (!url.path.empty()) {
    if (url.path[0] != '/') ss << '/';
    ss << url.path;
  }
  if (!url.query.empty()) ss << '?' << url.query;
  if (!url.fragment.empty()) ss << '#' << url.fragment;

  return ss.str();
}

/**
 * @brief url拼接
 * @note url结构：[protocol://][host][:service][path][?query][#fragment]
 * @param url url结构
 * @return std::string url字符串
 */
inline std::string JoinUrl(const Url& url) {
  return JoinUrl(UrlView{
      url.protocol,
      url.host,
      url.service,
      url.path,
      url.query,
      url.fragment});
}

}  // namespace ytlib
