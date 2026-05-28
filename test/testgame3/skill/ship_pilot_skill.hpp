/**
 * @file ship_pilot_skill.hpp
 * @brief 飞船驾驶 Skill：根据船当前状态自动决策（采矿/卖货/修理）
 *
 * 决策优先级（从高到低）：
 *   1. 耐久过低 → 找最近的修理站买配件并修
 *   2. 货舱有东西 → 找最高买价的站去卖
 *   3. 货舱空 → 找最近的矿区采矿（greed 高的可能跨区）
 *
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <algorithm>
#include <limits>

#include "core/world.hpp"
#include "facility/factory_station.hpp"
#include "facility/ship.hpp"
#include "facility/station_base.hpp"
#include "facility/trade_station.hpp"
#include "npc/npc.hpp"
#include "skill/skill.hpp"

namespace ytlib::testgame3 {

class ShipPilotSkill : public ISkill {
 public:
  SkillType Type() const override { return SkillType::ShipPilot; }
  FacilityType TargetFacility() const override { return FacilityType::Ship; }

  void OnEvent(World& w, Facility& f, const Event& e, NPC& self) override {
    auto* ship = dynamic_cast<Ship*>(&f);
    if (!ship) return;
    if (ship->State() == ShipState::Broken) return;
    if (std::holds_alternative<EvShipIdle>(e) ||
        std::holds_alternative<EvCommandCompleted>(e) ||
        std::holds_alternative<EvCommandFailed>(e) ||
        std::holds_alternative<EvShipArrived>(e) ||
        std::holds_alternative<EvShipDamaged>(e)) {
      Decide(w, *ship, self);
    }
  }

  void OnOrder(World& w, Facility& f, const Order& /*o*/, NPC& self) override {
    auto* ship = dynamic_cast<Ship*>(&f);
    if (ship && !ship->HasCommand()) Decide(w, *ship, self);
  }

  void OnTick(World& w, Facility& f, GameTime /*t*/, NPC& self) override {
    auto* ship = dynamic_cast<Ship*>(&f);
    if (!ship) return;
    if (ship->State() == ShipState::Broken) return;
    if (!ship->HasCommand()) Decide(w, *ship, self);
  }

 private:
  void Decide(World& w, Ship& ship, NPC& self) {
    if (ship.HasCommand()) return;

    // 1. 修理判断
    float repair_threshold = 30.0f + 30.0f * (1.0f - self.Traits().risk);
    float hull_ratio = static_cast<float>(ship.Hull()) / ship.HullMax() * 100.0f;
    if (hull_ratio < repair_threshold) {
      if (TryRepair(w, ship)) return;
    }

    // 2. 货舱有货 → 卖
    if (ship.Cargo().Used() > 0) {
      if (TrySell(w, ship, self)) return;
    }

    // 3. 货舱空 → 采矿
    if (TryMine(w, ship, self)) return;

    // 4. 都失败 → 怠速
    ship.Enqueue(ShipCmdIdle{20});
  }

  bool TryRepair(World& w, Ship& ship) {
    StationBase* best = nullptr;
    float best_dist = std::numeric_limits<float>::infinity();
    for (auto* sb : w.AllStations()) {
      int hp = sb->InventoryRef().Get(w.Config().repair_item_hull_plate);
      int ep = sb->InventoryRef().Get(w.Config().repair_item_engine_part);
      if (hp == 0 || ep == 0) continue;
      float d = ship.Position().Distance(sb->Position());
      if (d < best_dist) {
        best = sb;
        best_dist = d;
      }
    }
    if (!best) return false;
    if (ship.Position().Distance(best->Position()) > w.Config().dock_radius) {
      ship.Enqueue(ShipCmdMoveTo{best->Position()});
    }
    ship.Enqueue(ShipCmdRepair{best->Id()});
    return true;
  }

  bool TrySell(World& w, Ship& ship, NPC& self) {
    // 对货舱里每种 item，找最高买价（greed 高的接受更远距离）
    float max_distance_bias = 50.0f + 200.0f * self.Traits().greed;
    ItemId best_item = 0;
    StationBase* best_station = nullptr;
    Quote best_quote;
    float best_score = -std::numeric_limits<float>::infinity();

    for (auto& [item, qty] : ship.Cargo().Items()) {
      for (auto* sb : w.AllStations()) {
        auto quotes = sb->GetQuotes(item);
        for (auto& q : quotes) {
          if (q.side != Quote::Buy || q.qty <= 0) continue;
          float d = ship.Position().Distance(sb->Position());
          if (d > max_distance_bias) continue;
          // 评分 = 单价 - 距离 * 0.1（贪婪低看重距离，高看重单价）
          float score = static_cast<float>(q.price) - d * (1.0f - self.Traits().greed);
          if (score > best_score) {
            best_score = score;
            best_item = item;
            best_station = sb;
            best_quote = q;
          }
        }
      }
    }
    if (!best_station) return false;
    int qty = std::min(ship.Cargo().Get(best_item), best_quote.qty);
    if (ship.Position().Distance(best_station->Position()) > w.Config().dock_radius) {
      ship.Enqueue(ShipCmdMoveTo{best_station->Position()});
    }
    ship.Enqueue(ShipCmdSell{best_station->Id(), best_quote, qty});
    return true;
  }

  bool TryMine(World& w, Ship& ship, NPC& self) {
    // 选最近的矿区（greed 高可能选更远但更值钱的矿）
    MineArea* best = nullptr;
    float best_score = -std::numeric_limits<float>::infinity();
    for (auto& m : w.Mines()) {
      float d = ship.Position().Distance(m->Position());
      Money p = w.Items().Get(m->Ore()).base_price;
      float score = static_cast<float>(p) * self.Traits().greed - d;
      if (score > best_score) {
        best_score = score;
        best = m.get();
      }
    }
    if (!best) return false;
    if (ship.Position().Distance(best->Position()) > w.Config().mine_radius) {
      ship.Enqueue(ShipCmdMoveTo{best->Position()});
    }
    ship.Enqueue(ShipCmdMine{best->Id(), best->Ore(), ship.Cargo().Capacity()});
    return true;
  }
};

}  // namespace ytlib::testgame3
