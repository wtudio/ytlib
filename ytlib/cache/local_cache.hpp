/**
 * @file local_cache.hpp
 * @author WT
 * @brief 本地缓存
 * @note 简易的本地缓存组件
 * @date 2021-09-06
 */
#pragma once

#include <functional>
#include <unordered_map>

namespace ytlib {

/**
 * @brief LocalCache初始化配置
 * 
 */
struct LocalCacheCfg {
  LocalCacheCfg() {}

  size_t capacity = 1024;  ///<容量上限
  uint32_t ttl = 5000;     ///<超时时间，ms
};

/**
 * @brief LocalCache本地缓存工具
 * @note 支持TTL和LRU淘汰，接近上限时自动清出一部分容量
 * @tparam KeyType 
 * @tparam ValType 
 */
template <typename KeyType, typename ValType>
class LocalCache {
 public:
  struct ValContent {
    ValType val;
  };

 public:
  typedef std::function<bool(const KeyType&, ValType&)> GetDataFun;

 public:
  LocalCache(const LocalCacheCfg& cfg, GetDataFun&& get_data_fun) : cfg_(cfg),
                                                                    get_data_fun_(std::move(get_data_fun)),
                                                                    data_map(cfg_.capacity) {}
  ~LocalCache() {}

  /*
  std::pair<bool, const ValType&> GetVal(const KeyType& key) {
  }
  */

 private:
  LocalCacheCfg cfg_;
  GetDataFun get_data_fun_;
  std::unordered_map<KeyType, ValContent> data_map;
};

}  // namespace ytlib
