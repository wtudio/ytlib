/**
 * @file faction.hpp
 * @brief 势力：拥有资产、NPC、金库，是 Commander NPC 操作的"元设施"
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <vector>

#include "core/wallet.hpp"
#include "facility/facility.hpp"

namespace ytlib::testgame3 {

class Faction : public Facility {
 public:
  Faction(FacilityId id, std::string name, Money initial_wallet)
      : Facility(id, std::move(name), FacilityType::Faction, {0, 0}, nullptr),
        wallet_(initial_wallet) {
    owner_ = this;  // 自指：方便统一处理
  }

  Wallet& WalletRef() { return wallet_; }
  const Wallet& WalletRef() const { return wallet_; }

  void AddFacility(Facility* f) { facilities_.push_back(f); }
  void AddNpc(NpcId id) { npcs_.push_back(id); }
  void SetCommander(NpcId id) { commander_ = id; }

  const std::vector<Facility*>& Facilities() const { return facilities_; }
  const std::vector<NpcId>& Npcs() const { return npcs_; }
  NpcId Commander() const { return commander_; }

 private:
  Wallet wallet_;
  std::vector<Facility*> facilities_;
  std::vector<NpcId> npcs_;
  NpcId commander_ = kNullNpc;
};

}  // namespace ytlib::testgame3
