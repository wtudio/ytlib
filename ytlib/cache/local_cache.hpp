/**
 * @file local_cache.hpp
 * @author WT
 * @brief 本地缓存
 * @note 简易的本地缓存组件，底层使用unordered_map
 * @date 2021-09-06
 */
#pragma once

#include <chrono>
#include <list>
#include <optional>
#include <unordered_map>

namespace ytlib {

/**
 * @brief LocalCache初始化配置
 *
 */
struct LocalCacheCfg {
  LocalCacheCfg() {}

  size_t capacity = 1000;   ///<容量上限
  size_t clean_size = 900;  ///<超过容量上限进行清理后的上限size
  uint32_t ttl_ms = 5000;   ///<超时时间，ms
};

/**
 * @brief LocalCache本地缓存工具
 * @note 支持TTL和LRU淘汰，接近上限时自动清出一部分容量，底层使用unordered_map
 * key和val值是直接保存在map中的，且存在复制开销。如果val尺寸较大建议用智能指针
 * @tparam KeyType
 * @tparam ValType
 */
template <typename KeyType, typename ValType>
class LocalCache {
 public:
  LocalCache(const LocalCacheCfg& cfg) : cfg_(cfg),
                                         start_time_(std::chrono::steady_clock::now()) {
    if (cfg_.clean_size >= cfg_.capacity)
      cfg_.clean_size = static_cast<size_t>(cfg_.capacity * 0.9);

    data_map_.reserve(cfg_.capacity);
  }
  ~LocalCache() {}

  /**
   * @brief 获取缓存数据
   * @note 如果没有缓存数据，则返回std::nullopt
   * @param[in] key 缓存key
   * @return std::optional<ValType>
   */
  std::optional<ValType> Get(const KeyType& key) {
    return std::nullopt;
  }

  /**
   * @brief 更新缓存数据
   * @note 有则更新，无则新增
   * @param[in] key 缓存key
   * @param[in] val 缓存数据
   */
  void Update(const KeyType& key, const ValType& val) {
    const auto& emplace_ret = data_map_.emplace(key, {val});
    const ValContent& val_content = emplace_ret.first->second;
    if (emplace_ret.second) {
      // 新增

    } else {
      // 更新
    }
  }

  /**
   * @brief 清理过期数据
   * @note 一般不需要手动调用此接口
   */
  void Clean() {
  }

 private:
  using TimePoint = std::chrono::steady_clock::time_point;

  struct ValContent {
    ValType val;          // 数据
    TimePoint load_time;  // 上次更新时间
    std::list<typename std::unordered_map<KeyType, ValContent>::iterator>::iterator lru_itr;
    std::list<typename std::unordered_map<KeyType, ValContent>::iterator>::iterator ttl_itr;
  };

 private:
  LocalCacheCfg cfg_;
  TimePoint start_time_;

  std::unordered_map<KeyType, ValContent> data_map_;

  std::list<typename std::unordered_map<KeyType, ValContent>::iterator> lru_list_;
  std::list<typename std::unordered_map<KeyType, ValContent>::iterator> ttl_list_;
};

}  // namespace ytlib
