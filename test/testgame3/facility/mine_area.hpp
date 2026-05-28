/**
 * @file mine_area.hpp
 * @brief 矿区 POI：飞船在范围内可采矿
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include "core/types.hpp"
#include "facility/facility.hpp"

namespace ytlib::testgame3 {

class MineArea : public Facility {
 public:
  MineArea(FacilityId id, std::string name, Vec2 pos, ItemId ore, int yield_per_tick)
      : Facility(id, std::move(name), FacilityType::MineArea, pos, nullptr),
        ore_(ore),
        yield_per_tick_(yield_per_tick) {}

  ItemId Ore() const { return ore_; }
  int YieldPerTick() const { return yield_per_tick_; }

 private:
  ItemId ore_;
  int yield_per_tick_;
};

}  // namespace ytlib::testgame3
