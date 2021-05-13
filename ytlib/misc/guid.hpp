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

namespace ytlib {

// GUID相关配置，总位数不要超过64
enum {
  GUID_MAC_BIT = 18,                 // 机器/线程编号占位数。不要设置超过32位
  GUID_MAC_NUM = 1 << GUID_MAC_BIT,  // 最大机器/线程数

  GUID_OBJ_BIT = 7,                  // obj编号占位数。不要设置超过32位
  GUID_OBJ_NUM = 1 << GUID_OBJ_BIT,  // 最大obj类别数

  GUID_TIME0 = 1609430400,  // 初始时间：2021-01-01 00:00:00
  GUID_TIME_BIT = 29,       // 时间戳使用29位，大概够用十几年。不要设置超过32位

  GUID_INST_BIT = 10,                  // 每秒实例编号占位数。不要设置超过32位
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

// GuidGener，非线程安全，应用于单线程场景下
class GuidGener {
 public:
  virtual ~GuidGener() {
    if (guid_buf_ != nullptr) delete[] guid_buf_;
  }

  // 单例
  static GuidGener &Ins() {
    static thread_local GuidGener instance;
    return instance;
  }

  // 设置初始mac值，mac值不应超过GUID_MAC_BIT位，mac值应能在整个guid系统内部唯一标识一个线程
  void Init(uint32_t mac_id) {
    assert(mac_id < GUID_MAC_NUM);

    guid_buf_ = new Guid[GUID_OBJ_NUM];
    for (uint32_t ii = 0; ii < GUID_OBJ_NUM; ++ii) {
      guid_buf_[ii].id = 0;
      guid_buf_[ii].mac = mac_id;
      guid_buf_[ii].obj = ii;
    }
  };

  // 根据obj_id获取guid，obj_id值不应超过GUID_OBJ_NUM，obj_id应能在当前线程下唯一标识一种实例
  Guid GetGuid(uint32_t obj_id) {
    Guid &guid = guid_buf_[obj_id];
    uint32_t cur_t = (uint32_t)time(0) - GUID_TIME0;

    if (cur_t > guid.t) {
      guid.t = cur_t;
      guid.ins = 0;
    } else if (guid.ins < GUID_INST_NUM) {
      ++guid.ins;
    } else {
      ++guid.t;
      guid.ins = 0;
    }

    return guid;
  }

 protected:
  GuidGener() {
    static_assert(GUID_MAC_BIT + GUID_OBJ_BIT + GUID_TIME_BIT + GUID_INST_BIT <= 64, "guid size should be less than 64 bits");
  }

  Guid *guid_buf_ = nullptr;
};

// 对应一个mac+obj，生成guid，应用于单线程场景下
class ObjGuidGener {
 public:
  ObjGuidGener() {}
  ~ObjGuidGener() {}

  void Init(uint32_t obj_id) {
    assert(obj_id < GUID_OBJ_NUM);
    obj_id_ = obj_id;
  }

  // 获取guid
  Guid GetGuid() {
    return GuidGener::Ins().GetGuid(obj_id_);
  }

 private:
  uint32_t obj_id_ = 0;
};

}  // namespace ytlib
