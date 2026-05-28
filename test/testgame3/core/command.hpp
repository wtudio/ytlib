/**
 * @file command.hpp
 * @brief 飞船操作命令（NPC Skill 下发给 Ship 的低层动作）
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <variant>

#include "core/quote.hpp"
#include "core/types.hpp"

namespace ytlib::testgame3 {

struct ShipCmdMoveTo {
  Vec2 pos;
};

struct ShipCmdMine {
  FacilityId mine_area;  // 矿区 POI id；MVP 用 FacilityId 兼容
  ItemId item;
  int qty_target;
};

struct ShipCmdBuy {
  FacilityId station;
  Quote quote;  // 拿到的报价快照
  int qty;
};

struct ShipCmdSell {
  FacilityId station;
  Quote quote;
  int qty;
};

struct ShipCmdRepair {
  FacilityId station;
};

struct ShipCmdIdle {
  int ticks;
};

using ShipCommand =
    std::variant<ShipCmdMoveTo, ShipCmdMine, ShipCmdBuy, ShipCmdSell, ShipCmdRepair, ShipCmdIdle>;

}  // namespace ytlib::testgame3
