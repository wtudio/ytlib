/**
 * @file remote_cache.hpp
 * @author WT
 * @brief 远程数据缓存工具
 * @note 获取数据时先查本地缓存，未命中则去调用实际远程数据接口
 * @date 2021-11-03
 */
#pragma once

#include <functional>
#include <optional>

#include <unifex/async_mutex.hpp>
#include <unifex/task.hpp>

#include "ytlib/cache/local_cache.hpp"

namespace ytlib {

template <typename KeyType, typename ValType>
class RemoteCache {
 public:
  using LocalCacheType = LocalCache<KeyType, ValType>;

  using UpdateDataFunc = std::function<unifex::task<std::optional<ValType>>(const KeyType&)>;

 public:
  struct Cfg {
    LocalCacheType::Cfg local_cache_cfg;

    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      cfg.local_cache_cfg = LocalCacheType::Cfg::Verify(cfg.local_cache_cfg);

      return cfg;
    }
  };

  RemoteCache(const RemoteCache::Cfg& cfg)
      : cfg_(RemoteCache::Cfg::Verify(cfg)),
        local_cache_(cfg_.local_cache_cfg) {
    update_data_fun_ = [](auto) -> unifex::task<std::optional<ValType>> { co_return std::nullopt; };
  }

  ~RemoteCache() {}

  void SetUpdateDataFunc(const UpdateDataFunc& update_data_fun) {
    update_data_fun_ = update_data_fun;
  }

  void SetUpdateDataFunc(UpdateDataFunc&& update_data_fun) {
    update_data_fun_ = std::move(update_data_fun);
  }

  unifex::task<std::optional<ValType>> Get(const KeyType& key) {
    co_await local_cache_mutex_.async_lock();
    auto local_ret = local_cache_.Get(key);
    local_cache_mutex_.unlock();

    if (local_ret) co_return local_ret;

    auto ret = co_await update_data_fun_(key);

    if (ret) {
      co_await local_cache_mutex_.async_lock();
      local_cache_.Update(key, *ret);
      local_cache_mutex_.unlock();
    }
    co_return ret;
  }

 private:
  const RemoteCache::Cfg cfg_;

  unifex::async_mutex local_cache_mutex_;
  LocalCacheType local_cache_;

  UpdateDataFunc update_data_fun_;
};

}  // namespace ytlib