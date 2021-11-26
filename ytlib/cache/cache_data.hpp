/**
 * @file cache_data.hpp
 * @author WT
 * @brief 带缓存的数据获取工具
 * @note 获取数据时先查缓存，未命中则去调用实际数据接口
 * @date 2021-11-03
 */
#pragma once

#include <functional>
#include <optional>

#include "ytlib/cache/local_cache.hpp"

namespace ytlib {

/**
 * @brief CacheData初始化配置
 *
 */
struct CacheDataCfg {
  CacheDataCfg() {}

  LocalCacheCfg local_cache_cfg;
};

template <typename KeyType, typename ValType>
class CacheData {
 public:
  // 获取实际数据接口，输入key，输出val和是否获取成功
  typedef std::function<std::optional<ValType>(const KeyType&)> GetDataFun;

 public:
  CacheData(const CacheDataCfg& cfg) : cfg_(cfg),
                                       local_cache_(cfg_.local_cache_cfg) {
  }
  ~CacheData() {}

  void SetGetDataFun(const GetDataFun& get_data_fun) {
    get_data_fun_ = get_data_fun;
  }

  std::optional<ValType> Get(const KeyType& key) {
    auto ret = local_cache_.Get(key);
    if (ret) return ret;

    ret = get_data_fun_(key);

    if (ret) local_cache_.Update(key, *ret);

    return ret;
  }

 private:
  CacheDataCfg cfg_;

  GetDataFun get_data_fun_;

  LocalCache local_cache_;
};
}  // namespace ytlib
