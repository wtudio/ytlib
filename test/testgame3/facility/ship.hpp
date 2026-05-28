/**
 * @file ship.hpp
 * @brief 飞船：位置/航向/货舱/耐久/状态机
 *
 * Ship 自身只暴露低层原语（移动一步、扣耐久、出入货舱等），状态机推进由 World 驱动，
 * 避免与 World 之间的循环依赖。
 *
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <deque>
#include <string>

#include "core/command.hpp"
#include "core/inventory.hpp"
#include "core/types.hpp"
#include "core/wallet.hpp"
#include "facility/facility.hpp"

namespace ytlib::testgame3 {

enum class ShipState : uint8_t {
  Idle,
  Moving,
  Docked,
  Mining,
  Trading,
  Repairing,
  Broken,  // 耐久 0，作为残骸保留至 World 清理
};

class Ship : public Facility {
 public:
  Ship(FacilityId id, std::string name, Vec2 pos, Faction* owner,
       float speed, int cargo_capacity, int hull_max, Money initial_wallet)
      : Facility(id, std::move(name), FacilityType::Ship, pos, owner),
        speed_(speed),
        cargo_(cargo_capacity),
        hull_max_(hull_max),
        hull_(hull_max),
        wallet_(initial_wallet) {}

  // === state ===
  ShipState State() const { return state_; }
  void SetState(ShipState s) { state_ = s; }
  int Hull() const { return hull_; }
  int HullMax() const { return hull_max_; }
  void SetHull(int h) { hull_ = h < 0 ? 0 : (h > hull_max_ ? hull_max_ : h); }
  float Speed() const { return speed_; }
  Inventory& Cargo() { return cargo_; }
  const Inventory& Cargo() const { return cargo_; }
  Wallet& WalletRef() { return wallet_; }
  const Wallet& WalletRef() const { return wallet_; }

  // === command queue ===
  uint64_t Enqueue(ShipCommand c) {
    uint64_t cmd_id = ++next_cmd_id_;
    cmds_.push_back(std::move(c));
    cmd_ids_.push_back(cmd_id);
    return cmd_id;
  }
  void ClearCommands() {
    cmds_.clear();
    cmd_ids_.clear();
  }
  bool HasCommand() const { return !cmds_.empty(); }
  ShipCommand& CurrentCommand() { return cmds_.front(); }
  uint64_t CurrentCommandId() const { return cmd_ids_.front(); }
  void PopCommand() {
    cmds_.pop_front();
    cmd_ids_.pop_front();
  }

  // === movement helper ===
  // 朝 target 移动一步；返回 true 表示已到达 (距离 < arrive_radius)
  bool StepToward(Vec2 target, float arrive_radius) {
    Vec2 delta = target - pos_;
    float dist = delta.Length();
    if (dist <= arrive_radius) {
      pos_ = target;
      return true;
    }
    if (dist <= speed_) {
      pos_ = target;
      return true;
    }
    pos_ = pos_ + delta.Normalized() * speed_;
    return false;
  }

  void ApplyDamage(int amount) {
    if (amount <= 0) return;
    hull_ -= amount;
    if (hull_ < 0) hull_ = 0;
  }

 private:
  float speed_;
  Inventory cargo_;
  int hull_max_;
  int hull_;
  Wallet wallet_;
  ShipState state_ = ShipState::Idle;
  std::deque<ShipCommand> cmds_;
  std::deque<uint64_t> cmd_ids_;
  uint64_t next_cmd_id_ = 0;
};

}  // namespace ytlib::testgame3
