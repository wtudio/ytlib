/**
 * @file facility.hpp
 * @brief 所有可操作设施的基类
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <string>

#include "core/types.hpp"

namespace ytlib::testgame3 {

class Faction;  // forward
class NPC;      // forward

class Facility {
 public:
  Facility(FacilityId id, std::string name, FacilityType type, Vec2 pos, Faction* owner)
      : id_(id), name_(std::move(name)), type_(type), pos_(pos), owner_(owner) {}

  virtual ~Facility() = default;

  FacilityId Id() const { return id_; }
  const std::string& Name() const { return name_; }
  FacilityType Type() const { return type_; }
  Vec2 Position() const { return pos_; }
  Faction* Owner() const { return owner_; }
  NPC* Operator() const { return operator_; }

  void SetOperator(NPC* op) { operator_ = op; }
  void SetPosition(Vec2 p) { pos_ = p; }

 protected:
  FacilityId id_;
  std::string name_;
  FacilityType type_;
  Vec2 pos_;
  Faction* owner_;       // nullptr 表示无主（如矿区）
  NPC* operator_ = nullptr;  // 当前操作此设施的 NPC
};

}  // namespace ytlib::testgame3
