/**
 * @file factory_manager_skill.hpp
 * @brief 工厂管理 Skill：维护配方运行，挂买卖配置
 *
 * MVP 实现：被动响应事件 —— 生产卡住（库存满）时不切换；接到 Order 切换配方。
 *
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include "core/world.hpp"
#include "facility/factory_station.hpp"
#include "skill/skill.hpp"

namespace ytlib::testgame3 {

class FactoryManagerSkill : public ISkill {
 public:
  SkillType Type() const override { return SkillType::FactoryManager; }
  FacilityType TargetFacility() const override { return FacilityType::Factory; }

  void OnEvent(World& /*w*/, Facility& /*f*/, const Event& /*e*/, NPC& /*self*/) override {
    // MVP：被动，不响应
  }

  void OnOrder(World& /*w*/, Facility& f, const Order& o, NPC& /*self*/) override {
    auto* fs = dynamic_cast<FactoryStation*>(&f);
    if (!fs) return;
    if (auto* op = std::get_if<OrderOperateFactory>(&o)) {
      if (op->preferred != 0) fs->SetRecipe(op->preferred);
    }
  }

  void OnTick(World& /*w*/, Facility& /*f*/, GameTime /*t*/, NPC& /*self*/) override {
    // MVP：被动
  }
};

}  // namespace ytlib::testgame3
