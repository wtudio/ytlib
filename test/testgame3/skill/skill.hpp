/**
 * @file skill.hpp
 * @brief Skill 接口：操作 Facility 的能力
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include "core/event.hpp"
#include "core/order.hpp"
#include "core/types.hpp"

namespace ytlib::testgame3 {

class World;
class Facility;
class NPC;

class ISkill {
 public:
  virtual ~ISkill() = default;
  virtual SkillType Type() const = 0;
  virtual FacilityType TargetFacility() const = 0;

  virtual void OnEvent(World& w, Facility& f, const Event& e, NPC& self) = 0;
  virtual void OnOrder(World& w, Facility& f, const Order& o, NPC& self) = 0;
  virtual void OnTick(World& w, Facility& f, GameTime t, NPC& self) = 0;
};

}  // namespace ytlib::testgame3
