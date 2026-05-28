/**
 * @file item.hpp
 * @brief 商品定义与注册表
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "core/types.hpp"

namespace ytlib::testgame3 {

enum class ItemTier : uint8_t { Ore, Intermediate, Component };

struct Item {
  ItemId id;
  std::string name;
  ItemTier tier;
  Money base_price;  // 价格波动的基准价
  int unit_volume;   // 占用货舱/库存的单位体积
};

class ItemRegistry {
 public:
  ItemId Register(std::string name, ItemTier tier, Money base_price, int unit_volume = 1) {
    ItemId id = static_cast<ItemId>(items_.size() + 1);
    items_.push_back(Item{id, std::move(name), tier, base_price, unit_volume});
    return id;
  }

  const Item& Get(ItemId id) const { return items_.at(id - 1); }
  const std::vector<Item>& All() const { return items_; }
  size_t Count() const { return items_.size(); }

 private:
  std::vector<Item> items_;
};

}  // namespace ytlib::testgame3
