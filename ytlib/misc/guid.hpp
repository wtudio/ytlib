/**
 * @file guid.hpp
 * @brief 通用的local guid生成
 * @details 采用四个元素来生成一个guid：
 * 1)、mac值：能在整个系统内部唯一标识一个线程，可以由机器mac/服务器编号/进程号/线程号等生成
 * 2)、obj id值：能在当前线程内唯一标识一种类型的实例
 * 3)、time：当前秒级时间戳
 * 4)、ins：当前实例编号
 * @author WT
 * @email 905976782@qq.com
 * @date 2021-05-10
 */
#pragma once

#include <cinttypes>
#include <ctime>
#include <map>

namespace ytlib {

// GUID相关配置，总位数不要超过64
enum {
  GUID_MAC_BIT = 10,                 // 机器/线程编号占位数。不要设置超过32位
  GUID_MAC_NUM = 1 << GUID_MAC_BIT,  // 最大机器/线程数

  GUID_OBJ_BIT = 12,                 // obj编号占位数。不要设置超过32位
  GUID_OBJ_NUM = 1 << GUID_OBJ_BIT,  // 最大obj类别数

  GUID_TIME0 = 1564210961,  // 初始时间
  GUID_TIME_BIT = 29,       // 时间戳使用29位，大概够用十几年。不要设置超过32位

  GUID_INST_BIT = 13,                  // 每秒实例编号占位数。不要设置超过32位
  GUID_INST_NUM = 1 << GUID_INST_BIT,  // 每秒最大实例数

};

// GUID实例
union Guid {
  struct
  {
    uint64_t ins : GUID_INST_BIT;
    uint64_t t : GUID_TIME_BIT;
    uint64_t obj : GUID_OBJ_BIT;
    uint64_t mac : GUID_MAC_BIT;
  };
  uint64_t id;
};

// 对应一个mac+obj，生成guid，应用于单线程场景下
class GuidGener {
  friend class GuidGenerFactory;

 public:
  // 获取guid
  Guid GetGuid() {
    uint32_t cur_t = (uint32_t)time(0) - GUID_TIME0;

    if (cur_t > guid_.t) {
      guid_.t = cur_t;
      guid_.ins = 0;
    }

    if (guid_.ins < GUID_INST_NUM) {
      ++guid_.ins;
    } else {
      ++guid_.t;
      guid_.ins = 0;
    }

    return guid_;
  }

 private:
  GuidGener(uint32_t mac_id, uint32_t obj_id) {
    guid_.id = 0;
    guid_.mac = mac_id;
    guid_.obj = obj_id;
  }
  ~GuidGener() {}

  Guid guid_;
};

// GuidGener工厂类，应用于单线程场景下
class GuidGenerFactory {
 public:
  ~GuidGenerFactory() {
    for (auto& itr : guid_gener_buf_)
      delete itr.second;
  }

  // 线程级单例
  static GuidGenerFactory& Ins() {
    static thread_local GuidGenerFactory instance;
    return instance;
  }

  // 设置初始mac值，mac值不应超过GUID_MAC_BIT位，mac值应能在整个guid系统内部唯一标识一个线程
  void InitInThread(uint32_t mac_id) {
    mac_id_ = mac_id;
  };

  // 根据obj_id获取GuidGener，obj_id值不应超过GUID_OBJ_NUM，obj_id应能在当前线程下唯一标识一种实例
  GuidGener* GetGuidGener(uint32_t obj_id) {
    auto finditr = guid_gener_buf_.find(obj_id);
    if (finditr != guid_gener_buf_.end())
      return finditr->second;

    GuidGener* gener = new GuidGener(mac_id_, obj_id);
    guid_gener_buf_.emplace(obj_id, gener);
    return gener;
  }

 private:
  GuidGenerFactory() {
    static_assert(GUID_MAC_BIT + GUID_OBJ_BIT + GUID_TIME_BIT + GUID_INST_BIT <= 64, "guid size should be less than 64 bits");
  }

  uint32_t mac_id_;
  std::map<uint32_t, GuidGener*> guid_gener_buf_;
};

}  // namespace ytlib
