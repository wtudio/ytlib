/**
 * @file http_dispatcher.hpp
 * @brief 基于boost.beast的http客户端
 * @note 基于boost.beast的http客户端
 * @author WT
 * @date 2022-04-17
 */
#pragma once

#include <functional>
#include <map>
#include <string>

namespace ytlib {

template <typename HttpHandleFuncType>
class HttpDispatcher {
 public:
  using HttpHandle = std::function<HttpHandleFuncType>;

 public:
  HttpDispatcher() {}
  ~HttpDispatcher() {}

  void RegisterHttpHandle(std::string_view pattern, HttpHandle&& handle) {
    http_handle_map_.emplace(pattern, std::move(handle));
  }

  void RegisterHttpHandle(std::string_view pattern, const HttpHandle& handle) {
    http_handle_map_.emplace(pattern, handle);
  }

  const HttpHandle& GetHttpHandle(std::string_view target) const {
    // todo 优化
    for (const auto& itr : http_handle_map_) {
      if (target.size() < itr.first.size()) continue;
      if (target.substr(0, itr.first.size()) != itr.first) continue;
      return itr.second;
    }

    static HttpHandle empty_handle;
    return empty_handle;
  }

 private:
  std::map<std::string, HttpHandle> http_handle_map_;
};

}  // namespace ytlib
