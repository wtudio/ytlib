/**
 * @file shipyard.hpp
 * @brief 造船厂：消耗船体配件造新船
 *
 * MVP 简化：造船配方是固定的全局常量 BuildShipBlueprint，运行时直接消耗 + 通知 World 生船。
 *
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <utility>
#include <vector>

#include "facility/station_base.hpp"

namespace ytlib::testgame3 {

struct ShipBlueprint {
  uint32_t id;
  std::vector<std::pair<ItemId, int>> materials;  // 造一艘船所需材料
  Money cost;                                      // 造船费用（扣造船厂 wallet）
  int ticks_required;                              // 建造时间
  float ship_speed;
  int ship_cargo_capacity;
  int ship_hull_max;
};

class Shipyard : public StationBase {
 public:
  Shipyard(FacilityId id, std::string name, Vec2 pos, Faction* owner,
           const ItemRegistry* items, int capacity, Money wallet)
      : StationBase(id, std::move(name), FacilityType::Shipyard, pos, owner, items, capacity, wallet) {}

  void SetBlueprint(ShipBlueprint bp) { blueprint_ = std::move(bp); }
  const ShipBlueprint& Blueprint() const { return blueprint_; }

  // 队列里有几艘待造的
  int QueueDepth() const { return static_cast<int>(queue_.size()); }
  void Enqueue() { queue_.push_back(0); }

  enum class TickResult { Idle, Working, Built, StalledInput };

  TickResult Tick() {
    if (queue_.empty()) return TickResult::Idle;
    int& progress = queue_.front();
    if (progress == 0) {
      // 检查材料
      for (auto& [item, qty] : blueprint_.materials) {
        if (inventory_.Get(item) < qty) return TickResult::StalledInput;
      }
      for (auto& [item, qty] : blueprint_.materials) inventory_.Remove(item, qty);
      progress = 1;
      return TickResult::Working;
    }
    progress++;
    if (progress >= blueprint_.ticks_required) {
      queue_.pop_front();
      return TickResult::Built;
    }
    return TickResult::Working;
  }

 private:
  ShipBlueprint blueprint_;
  std::deque<int> queue_;  // 每个元素 = 当前进度
};

}  // namespace ytlib::testgame3
