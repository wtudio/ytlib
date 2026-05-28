/**
 * @file trader_skill.hpp
 * @brief 贸易站做市 Skill：MVP 完全被动，依靠 StationBase 的库存反馈定价自动做市
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include "skill/skill.hpp"

namespace ytlib::testgame3 {

class TraderSkill : public ISkill {
 public:
  SkillType Type() const override { return SkillType::Trader; }
  FacilityType TargetFacility() const override { return FacilityType::Trade; }

  void OnEvent(World& /*w*/, Facility& /*f*/, const Event& /*e*/, NPC& /*self*/) override {}
  void OnOrder(World& /*w*/, Facility& /*f*/, const Order& /*o*/, NPC& /*self*/) override {}
  void OnTick(World& /*w*/, Facility& /*f*/, GameTime /*t*/, NPC& /*self*/) override {}
};

}  // namespace ytlib::testgame3
