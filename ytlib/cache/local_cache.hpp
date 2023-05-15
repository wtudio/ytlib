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
 * @brief LocalCache本地缓存工具
 * @note 支持TTL和LRU淘汰，接近上限时自动清出一部分容量，底层使用unordered_map
 * key和val值是直接保存在map中的，且存在复制开销。如果val尺寸较大建议用智能指针
 * 非线程安全，需要上层保证线程安全
 * @tparam KeyType 值类型，需要支持比较
 * @tparam ValType 数据类型
 */
template <typename KeyType, typename ValType>
class LocalCache {
 public:
  /**
   * @brief 配置
   *
   */
  struct Cfg {
    size_t capacity = 1000 * 1024;                                      ///< 容量上限
    size_t clean_size = 900 * 1024;                                     ///< 超过容量上限进行清理的目标size
    std::chrono::steady_clock::duration ttl = std::chrono::seconds(5);  ///< 超时时间

    /// 校验配置
    static Cfg Verify(const Cfg& verify_cfg) {
      Cfg cfg(verify_cfg);

      if (cfg.clean_size >= cfg.capacity)
        cfg.clean_size = static_cast<size_t>(cfg.capacity * 0.9);

      return cfg;
    }
  };

  explicit LocalCache(const LocalCache::Cfg& cfg)
      : cfg_(LocalCache::Cfg::Verify(cfg)),
        data_map_(cfg_.capacity) {
  }

  ~LocalCache() {}

  LocalCache(const LocalCache&) = delete;             ///< no copy
  LocalCache& operator=(const LocalCache&) = delete;  ///< no copy

  /**
   * @brief 获取缓存数据
   * @note 如果没有缓存数据，则返回std::nullopt
   * @param[in] key 缓存key
   * @return std::optional<ValType> 缓存数据
   */
  std::optional<ValType> Get(const KeyType& key) {
    CleanExpireddata();

    auto finditr = data_map_.find(key);
    if (finditr == data_map_.end()) [[unlikely]]
      return std::nullopt;

    ValContent& val_content = finditr->second;
    lru_list_.splice(lru_list_.end(), lru_list_, val_content.lru_itr);
    return val_content.val;
  }

  /**
   * @brief 更新缓存数据
   * @note 有则更新，无则新增
   * @tparam Args 缓存数据类型，或缓存数据的构造参数类型
   * @param[in] key 缓存key
   * @param[in] args 缓存数据，或缓存数据的构造参数
   */
  template <typename... Args>
    requires std::constructible_from<ValType, Args...>
  void Update(const KeyType& key, Args&&... args) {
    const auto& emplace_ret = data_map_.emplace(key, std::forward<Args>(args)...);
    ValContent& val_content = emplace_ret.first->second;
    val_content.load_time = std::chrono::steady_clock::now();

    if (emplace_ret.second) {
      // 新增
      val_content.lru_itr = lru_list_.emplace(lru_list_.end(), key);
      val_content.ttl_itr = ttl_list_.emplace(ttl_list_.end(), key);
      // 如果达到容量上限则需要清理
      if (data_map_.size() >= cfg_.capacity) Clean();
    } else {
      // 更新
      val_content.val = ValType(std::forward<Args>(args)...);
      lru_list_.splice(lru_list_.end(), lru_list_, val_content.lru_itr);
      ttl_list_.splice(ttl_list_.end(), ttl_list_, val_content.ttl_itr);
    }
  }

  /**
   * @brief 删除某个数据
   *
   * @param[in] key 待删除数据的key
   */
  void Del(const KeyType& key) {
    CleanExpireddata();

    auto finditr = data_map_.find(key);
    if (finditr == data_map_.end()) [[unlikely]]
      return;

    ValContent& val_content = finditr->second;
    lru_list_.erase(val_content.lru_itr);
    ttl_list_.erase(val_content.ttl_itr);

    data_map_.erase(finditr);
  }

  /**
   * @brief 清理过期数据
   * @note 一般不需要手动调用此接口
   */
  void CleanExpireddata() {
    std::chrono::steady_clock::time_point time_line = std::chrono::steady_clock::now() - cfg_.ttl;

    auto itr = ttl_list_.begin();
    for (; itr != ttl_list_.end(); ++itr) {
      auto finditr = data_map_.find(*itr);  // 此处一定能找到
      ValContent& val_content = finditr->second;
      if (val_content.load_time < time_line) {
        lru_list_.erase(val_content.lru_itr);
        data_map_.erase(finditr);
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
        --to_clean_size;
        auto finditr = data_map_.find(*itr);  // 此处一定能找到
        ttl_list_.erase(finditr->second.ttl_itr);
        data_map_.erase(finditr);
      } else {
        break;
      }
    }
    lru_list_.erase(lru_list_.begin(), itr);
  }

  /**
   * @brief 删除所有数据，恢复到初始状态
   *
   */
  void Clear() {
    lru_list_.clear();
    ttl_list_.clear();
    data_map_.clear();
  }

  /**
   * @brief 获取当前缓存数据量
   *
   * @return const size_t 当前缓存数据量
   */
  const size_t Size() const {
    return data_map_.size();
  }

  /**
   * @brief 获取配置
   *
   * @return const LocalCache::Cfg&
   */
  const LocalCache::Cfg& GetCfg() const { return cfg_; }

 private:
  struct ValContent {
   public:
    template <typename... Args>
    explicit ValContent(Args&&... args) : val(std::forward<Args>(args)...) {}

    ~ValContent() {}

    ValContent(const ValContent&) = delete;             ///< no copy
    ValContent& operator=(const ValContent&) = delete;  ///< no copy

   public:
    ValType val;  // 数据

    std::chrono::steady_clock::time_point load_time;  // 上次更新时间

    std::list<KeyType>::iterator lru_itr;
    std::list<KeyType>::iterator ttl_itr;
  };

  const LocalCache::Cfg cfg_;
  std::unordered_map<KeyType, ValContent> data_map_;

  std::list<KeyType> lru_list_;
  std::list<KeyType> ttl_list_;
};

}  // namespace ytlib
