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
  size_t capacity = 1000;   ///< 容量上限
  size_t clean_size = 900;  ///< 超过容量上限进行清理的目标size
  uint32_t ttl_ms = 5000;   ///< 超时时间，ms
};

/**
 * @brief LocalCache本地缓存工具
 * @note 支持TTL和LRU淘汰，接近上限时自动清出一部分容量，底层使用unordered_map
 * key和val值是直接保存在map中的，且存在复制开销。如果val尺寸较大建议用智能指针
 * 需要上层保证线程安全
 * @tparam KeyType
 * @tparam ValType
 */
template <typename KeyType, typename ValType>
class LocalCache {
 public:
  LocalCache(const LocalCacheCfg& cfg) : cfg_(cfg),
                                         data_map_(cfg_.capacity) {
    if (cfg_.clean_size >= cfg_.capacity)
      cfg_.clean_size = static_cast<size_t>(cfg_.capacity * 0.9);
  }
  ~LocalCache() {}

  /**
   * @brief 获取缓存数据
   * @note 如果没有缓存数据，则返回std::nullopt
   * @param[in] key 缓存key
   * @return std::optional<ValType>
   */
  std::optional<ValType> Get(const KeyType& key) {
    CleanExpireddata();

    auto finditr = data_map_.find(key);
    if (finditr == data_map_.end()) return std::nullopt;

    ValContent& val_content = finditr->second;
    lru_list_.splice(lru_list_.end(), lru_list_, *(static_cast<std::list<MapItr>::iterator*>(static_cast<void*>(&(val_content.lru_itr)))));
    return val_content.val;
  }

  /**
   * @brief 更新缓存数据
   * @note 有则更新，无则新增
   * @param[in] key 缓存key
   * @param[in] val 缓存数据
   */
  void Update(const KeyType& key, const ValType& val) {
    const auto& emplace_ret = data_map_.emplace(key, val);
    ValContent& val_content = emplace_ret.first->second;
    val_content.load_time = std::chrono::steady_clock::now();
    if (emplace_ret.second) {
      // 新增
      *(static_cast<std::list<MapItr>::iterator*>(static_cast<void*>(&(val_content.lru_itr)))) = lru_list_.emplace(lru_list_.end(), emplace_ret.first);
      *(static_cast<std::list<MapItr>::iterator*>(static_cast<void*>(&(val_content.ttl_itr)))) = ttl_list_.emplace(ttl_list_.end(), emplace_ret.first);
      // 如果超出容量上限需要清理
      if (data_map_.size() >= cfg_.clean_size) Clean();
    } else {
      // 更新
      val_content.val = val;
      lru_list_.splice(lru_list_.end(), lru_list_, *(static_cast<std::list<MapItr>::iterator*>(static_cast<void*>(&(val_content.lru_itr)))));
      ttl_list_.splice(ttl_list_.end(), ttl_list_, *(static_cast<std::list<MapItr>::iterator*>(static_cast<void*>(&(val_content.ttl_itr)))));
    }
  }

  /**
   * @brief 清理过期数据
   * @note 一般不需要手动调用此接口
   */
  void CleanExpireddata() {
    TimePoint time_line = std::chrono::steady_clock::now() - std::chrono::milliseconds(cfg_.ttl_ms);
    auto itr = ttl_list_.begin();
    for (; itr != ttl_list_.end(); ++itr) {
      ValContent& val_content = (*itr)->second;
      if (val_content.load_time < time_line) {
        lru_list_.erase(*(static_cast<std::list<MapItr>::iterator*>(static_cast<void*>(&(val_content.lru_itr)))));
        data_map_.erase(*itr);
      } else {
        break;
      }
    }
    ttl_list_.erase(ttl_list_.begin(), itr);
  }

  /**
   * @brief 清理数据直到数据量小于clean_size
   * @note 一般不需要手动调用此接口
   */
  void Clean() {
    CleanExpireddata();
    if (data_map_.size() <= cfg_.clean_size) return;

    size_t to_clean_size = data_map_.size() - cfg_.clean_size;
    auto itr = lru_list_.begin();
    for (; itr != lru_list_.end(); ++itr) {
      if (to_clean_size) {
        ttl_list_.erase(*(static_cast<std::list<MapItr>::iterator*>(static_cast<void*>(&((*itr)->second.ttl_itr)))));
        data_map_.erase(*itr);
        --to_clean_size;
      } else {
        break;
      }
    }
    lru_list_.erase(lru_list_.begin(), itr);
  }

 private:
  using TimePoint = std::chrono::steady_clock::time_point;
  using ListItrBuf = char[sizeof(std::list<int>::iterator)];

  struct ValContent {
    ValType val;          // 数据
    TimePoint load_time;  // 上次更新时间
    ListItrBuf lru_itr;
    ListItrBuf ttl_itr;
  };

  using MapItr = std::unordered_map<KeyType, ValContent>::iterator;

 private:
  LocalCacheCfg cfg_;
  std::unordered_map<KeyType, ValContent> data_map_;

  std::list<MapItr> lru_list_;
  std::list<MapItr> ttl_list_;
};

}  // namespace ytlib
