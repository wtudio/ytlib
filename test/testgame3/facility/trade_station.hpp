/**
 * @file trade_station.hpp
 * @brief 贸易站：纯做市，没有生产
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include "facility/station_base.hpp"

namespace ytlib::testgame3 {

class TradeStation : public StationBase {
 public:
  TradeStation(FacilityId id, std::string name, Vec2 pos, Faction* owner,
               const ItemRegistry* items, int capacity, Money wallet)
      : StationBase(id, std::move(name), FacilityType::Trade, pos, owner, items, capacity, wallet) {}
};

}  // namespace ytlib::testgame3
