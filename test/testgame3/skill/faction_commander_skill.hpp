/**
 * @file faction_commander_skill.hpp
 * @brief 势力主管 Skill：监控势力资产，必要时下造船单
 *
 * MVP 行为：
 *   - 收到 EvShipDestroyed：若拥有 shipyard 且金库够，给 shipyard 下一艘船
 *   - OnTick 每 200 tick：盘点资金 → 给闲船下 OrderHuntProfit
 *
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <variant>

#include "core/world.hpp"
#include "facility/faction.hpp"
#include "facility/shipyard.hpp"
#include "skill/skill.hpp"

namespace ytlib::testgame3 {

class FactionCommanderSkill : public ISkill {
 public:
  SkillType Type() const override { return SkillType::FactionCommander; }
  FacilityType TargetFacility() const override { return FacilityType::Faction; }

  void OnEvent(World& w, Facility& f, const Event& e, NPC& /*self*/) override {
    auto* fac = dynamic_cast<Faction*>(&f);
    if (!fac) return;
    if (std::holds_alternative<EvShipDestroyed>(e)) {
      // 尝试给自己的造船厂下造船单
      for (Facility* asset : fac->Facilities()) {
        if (asset->Type() == FacilityType::Shipyard) {
          auto* y = static_cast<Shipyard*>(asset);
          if (fac->WalletRef().Balance() >= y->Blueprint().cost) {
            fac->WalletRef().Debit(y->Blueprint().cost);
            y->WalletRef().Credit(y->Blueprint().cost);
            y->Enqueue();
            w.Log().Eventf(w.Now(), "势力-%s 委托建造 新飞船 于 %s (建造队列=%d)",
                           fac->Name().c_str(), y->Name().c_str(), y->QueueDepth());
          }
          return;
        }
      }
    }
  }

  void OnOrder(World& /*w*/, Facility& /*f*/, const Order& /*o*/, NPC& /*self*/) override {}

  void OnTick(World& /*w*/, Facility& /*f*/, GameTime /*t*/, NPC& /*self*/) override {
    // MVP：飞船的 ShipPilotSkill 自己 OnTick 会触发 Decide，Commander 不必每 tick 干预
  }
};

}  // namespace ytlib::testgame3
