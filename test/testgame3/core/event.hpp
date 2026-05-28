/**
 * @file event.hpp
 * @brief 事件类型与事件总线
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <queue>
#include <string>
#include <variant>
#include <vector>

#include "core/quote.hpp"
#include "core/types.hpp"

namespace ytlib::testgame3 {

struct EvShipArrived {
  FacilityId ship;
  FacilityId station_or_zero;
  Vec2 pos;
};

struct EvShipDamaged {
  FacilityId ship;
  int hull_delta;
  int hull_now;
};

struct EvShipDestroyed {
  FacilityId ship;
  Vec2 last_pos;
};

struct EvCommandCompleted {
  FacilityId ship;
  uint64_t cmd_id;
};

struct EvCommandFailed {
  FacilityId ship;
  uint64_t cmd_id;
  std::string reason;
};

struct EvTradeExecuted {
  FacilityId station;
  FacilityId counterparty;
  ItemId item;
  Quote::Side side_for_station;
  Money price;
  int qty_filled;
};

struct EvFactoryProduced {
  FacilityId factory;
  RecipeId recipe;
};

struct EvProductionStalled {
  FacilityId factory;
  RecipeId recipe;
  std::string reason;
};

struct EvShipBuilt {
  FacilityId shipyard;
  FacilityId new_ship;
};

struct EvWalletLow {
  FacilityId facility;
  Money current;
};

struct EvShipIdle {
  FacilityId ship;
};

using Event = std::variant<
    EvShipArrived, EvShipDamaged, EvShipDestroyed, EvCommandCompleted,
    EvCommandFailed, EvTradeExecuted, EvFactoryProduced, EvProductionStalled,
    EvShipBuilt, EvWalletLow, EvShipIdle>;

// 投递目标
enum class EventTarget : uint8_t {
  Facility,  // 投递给该设施 owner NPC + 所属 Faction commander
  Faction,   // 仅投递给指定 faction commander
  Global,    // 仅记录日志
};

struct PendingEvent {
  EventTarget target;
  FacilityId target_id;
  Event event;
};

class EventBus {
 public:
  void PublishToFacility(FacilityId fid, Event e) {
    queue_.push(PendingEvent{EventTarget::Facility, fid, std::move(e)});
  }
  void PublishToFaction(FacilityId faction_id, Event e) {
    queue_.push(PendingEvent{EventTarget::Faction, faction_id, std::move(e)});
  }
  void PublishGlobal(Event e) {
    queue_.push(PendingEvent{EventTarget::Global, kNullFacility, std::move(e)});
  }

  bool Empty() const { return queue_.empty(); }
  PendingEvent Pop() {
    PendingEvent p = std::move(queue_.front());
    queue_.pop();
    return p;
  }

 private:
  std::queue<PendingEvent> queue_;
};

}  // namespace ytlib::testgame3
