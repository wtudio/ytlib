/**
 * @file inventory.hpp
 * @brief 库存容器
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <unordered_map>

#include "core/types.hpp"

namespace ytlib::testgame3 {

class Inventory {
 public:
  explicit Inventory(int capacity = 1000000) : capacity_(capacity) {}

  int Get(ItemId id) const {
    auto it = items_.find(id);
    return (it == items_.end()) ? 0 : it->second;
  }

  int Used() const { return used_; }
  int Capacity() const { return capacity_; }
  int Free() const { return capacity_ - used_; }
  void SetCapacity(int cap) { capacity_ = cap; }

  // 尝试加入 qty 单位的 item，受容量限制。返回实际加入数量。
  int Add(ItemId id, int qty) {
    if (qty <= 0) return 0;
    int can = (qty < (capacity_ - used_)) ? qty : (capacity_ - used_);
    if (can <= 0) return 0;
    items_[id] += can;
    used_ += can;
    return can;
  }

  // 尝试移除 qty 单位的 item。返回实际移除数量。
  int Remove(ItemId id, int qty) {
    if (qty <= 0) return 0;
    auto it = items_.find(id);
    if (it == items_.end()) return 0;
    int can = (qty < it->second) ? qty : it->second;
    it->second -= can;
    used_ -= can;
    if (it->second == 0) items_.erase(it);
    return can;
  }

  const std::unordered_map<ItemId, int>& Items() const { return items_; }

 private:
  std::unordered_map<ItemId, int> items_;
  int used_ = 0;
  int capacity_;
};

}  // namespace ytlib::testgame3
