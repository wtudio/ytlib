/**
 * @file url_parser.hpp
 * @author WT
 * @brief Url解析器
 * @note Url解析器
 * @date 2022-04-22
 */
#pragma once

#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace ytlib {

/**
 * @brief url元素
 *
 * @tparam StringType
 */
template <class StringType = std::string_view>
  requires(
      std::is_same_v<StringType, std::string_view> ||
      std::is_same_v<StringType, std::string>)
struct Url {
  StringType protocol;  // 协议
  StringType host;      // host
  StringType service;   // 端口
  StringType path;      // 路径
  StringType query;     // 参数
  StringType fragment;  // 额外信息
};

/**
 * @brief url解析
 * @note url结构：[protocol://][host][:service][path][?query][#fragment]
 * @param url_str url字符串
 * @return std::optional<UrlView> url结构，nullopt则代表解析失败
 */
template <class StringType = std::string_view>
std::optional<Url<StringType> > ParseUrl(std::string_view url_str) {
  Url<StringType> url;
  std::string_view remaining = url_str;

  // protocol
  auto scheme_end = remaining.find("://");
  if (scheme_end != std::string_view::npos) {
    url.protocol = StringType(remaining.substr(0, scheme_end));
    remaining = remaining.substr(scheme_end + 3);
  }

  // fragment
  auto frag_pos = remaining.find('#');
  if (frag_pos != std::string_view::npos) {
    url.fragment = StringType(remaining.substr(frag_pos + 1));
    remaining = remaining.substr(0, frag_pos);
  }

  // query
  auto query_pos = remaining.find('?');
  if (query_pos != std::string_view::npos) {
    url.query = StringType(remaining.substr(query_pos + 1));
    remaining = remaining.substr(0, query_pos);
  }

  // authority and path
  auto path_pos = remaining.find('/');
  std::string_view authority;
  if (path_pos != std::string_view::npos) {
    authority = remaining.substr(0, path_pos);
    url.path = StringType(remaining.substr(path_pos));
  } else {
    authority = remaining;
  }

  // host:service from authority (strip userinfo if present)
  if (!authority.empty()) {
    auto at_pos = authority.find('@');
    std::string_view host_part = (at_pos != std::string_view::npos)
                                     ? authority.substr(at_pos + 1)
                                     : authority;
    auto colon_pos = host_part.find(':');
    if (colon_pos != std::string_view::npos) {
      url.host = StringType(host_part.substr(0, colon_pos));
      url.service = StringType(host_part.substr(colon_pos + 1));
    } else {
      url.host = StringType(host_part);
    }
  }

  return std::optional<Url<StringType> >{url};
}

/**
 * @brief url拼接
 * @note url结构：[protocol://][host][:service][path][?query][#fragment]
 * @param url url结构
 * @return std::string url字符串
 */
template <class StringType = std::string_view>
std::string JoinUrl(const Url<StringType>& url) {
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

}  // namespace ytlib
