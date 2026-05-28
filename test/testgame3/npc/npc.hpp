/**
 * @file npc.hpp
 * @brief NPC：身份 + traits + skills，事件/命令的接收处
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>

#include "core/event.hpp"
#include "core/order.hpp"
#include "core/types.hpp"
#include "facility/facility.hpp"
#include "npc/memory.hpp"
#include "npc/traits.hpp"
#include "skill/skill.hpp"

namespace ytlib::testgame3 {

class Facility;
class World;

class NPC {
 public:
  NPC(NpcId id, std::string name, NpcTraits traits)
      : id_(id), name_(std::move(name)), traits_(traits) {}

  NpcId Id() const { return id_; }
  const std::string& Name() const { return name_; }
  const NpcTraits& Traits() const { return traits_; }

  void AddSkill(std::unique_ptr<ISkill> s) {
    SkillType t = s->Type();
    skills_[t] = std::move(s);
  }
  ISkill* FindSkill(FacilityType ft) {
    for (auto& [k, v] : skills_) {
      if (v->TargetFacility() == ft) return v.get();
    }
    return nullptr;
  }

  Facility* Assignment() const { return assignment_; }
  void SetAssignment(Facility* f) { assignment_ = f; }

  void DeliverEvent(World& w, const Event& e);
  void DeliverOrder(World& w, Order o) { orders_inbox_.push_back(std::move(o)); }
  void Tick(World& w, GameTime t);

  std::deque<Order>& Inbox() { return orders_inbox_; }

 private:
  NpcId id_;
  std::string name_;
  NpcTraits traits_;
  std::unordered_map<SkillType, std::unique_ptr<ISkill>> skills_;
  Facility* assignment_ = nullptr;
  std::deque<Order> orders_inbox_;
  std::deque<MemoryEntry> memory_;  // MVP 不读
};

inline void NPC::DeliverEvent(World& w, const Event& e) {
  if (!assignment_) return;
  ISkill* s = FindSkill(assignment_->Type());
  if (!s) return;
  s->OnEvent(w, *assignment_, e, *this);
}

inline void NPC::Tick(World& w, GameTime t) {
  if (!assignment_) return;
  ISkill* s = FindSkill(assignment_->Type());
  if (!s) return;
  // 处理 inbox 里的 order
  while (!orders_inbox_.empty()) {
    Order o = std::move(orders_inbox_.front());
    orders_inbox_.pop_front();
    s->OnOrder(w, *assignment_, o, *this);
  }
  s->OnTick(w, *assignment_, t, *this);
}

}  // namespace ytlib::testgame3
