/**
 * @file world.hpp
 * @brief World：实体容器、tick 驱动、事件路由、快照
 *
 * Tick 顺序：
 *   1. 推进所有 Ship 的命令状态机
 *   2. 推进所有 Factory 生产
 *   3. 推进所有 Shipyard 建造
 *   4. Drain EventBus，把事件分发给关心的 NPC（含 Commander）
 *   5. 调度所有 NPC 的 OnTick（自发决策）
 *   6. 满足条件则写快照
 *
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <chrono>
#include <memory>
#include <thread>
#include <unordered_map>
#include <variant>
#include <vector>

#include "core/event.hpp"
#include "core/item.hpp"
#include "core/logger.hpp"
#include "core/random.hpp"
#include "core/recipe.hpp"
#include "core/types.hpp"
#include "facility/factory_station.hpp"
#include "facility/faction.hpp"
#include "facility/mine_area.hpp"
#include "facility/ship.hpp"
#include "facility/shipyard.hpp"
#include "facility/trade_station.hpp"
#include "npc/npc.hpp"
#include "ytlib/misc/misc_macro.h"

namespace ytlib::testgame3 {

struct WorldConfig {
  uint64_t seed = 42;
  int tick_ms = 100;
  GameTime snapshot_every = 100;
  uint64_t total_ticks = 5000;
  bool fast = false;

  // 飞船相关
  float damage_prob_per_tick = 0.005f;  // Moving 时每 tick 受损概率
  int damage_min = 1;
  int damage_max = 5;
  float dock_radius = 3.0f;
  float mine_radius = 3.0f;

  // 修理：耐久每点 = 0.02 个 HullPlate + 0.02 个 EnginePart
  // MVP 简化：站消耗 1 HullPlate 恢复 50 hp、1 EnginePart 恢复 50 hp
  int hp_per_hull_plate = 50;
  int hp_per_engine_part = 50;

  ItemId repair_item_hull_plate = 0;
  ItemId repair_item_engine_part = 0;
};

class World {
 public:
  explicit World(WorldConfig cfg, const std::string& event_log,
                 const std::string& snapshot_log, const std::string& init_log)
      : cfg_(cfg),
        rng_(cfg.seed),
        logger_(event_log, snapshot_log, init_log) {}

  // === registries ===
  ItemRegistry& Items() { return items_; }
  const ItemRegistry& Items() const { return items_; }
  RecipeRegistry& Recipes() { return recipes_; }
  const RecipeRegistry& Recipes() const { return recipes_; }
  WorldRng& Rng() { return rng_; }
  EventBus& Events() { return events_; }
  Logger& Log() { return logger_; }
  const WorldConfig& Config() const { return cfg_; }

  void SetRepairItems(ItemId hull_plate, ItemId engine_part) {
    cfg_.repair_item_hull_plate = hull_plate;
    cfg_.repair_item_engine_part = engine_part;
  }

  // === entity creation ===
  Faction& AddFaction(std::string name, Money initial_wallet) {
    auto f = std::make_unique<Faction>(NextFid(), std::move(name), initial_wallet);
    Faction* p = f.get();
    factions_[p->Id()] = std::move(f);
    RegisterFacility(p);
    return *p;
  }

  Ship& AddShip(Faction& owner, std::string name, Vec2 pos,
                float speed, int cargo, int hull, Money wallet) {
    auto s = std::make_unique<Ship>(NextFid(), std::move(name), pos, &owner, speed, cargo, hull, wallet);
    Ship* p = s.get();
    ships_.push_back(std::move(s));
    RegisterFacility(p);
    owner.AddFacility(p);
    return *p;
  }

  FactoryStation& AddFactory(Faction& owner, std::string name, Vec2 pos,
                             int capacity, Money wallet) {
    auto f = std::make_unique<FactoryStation>(NextFid(), std::move(name), pos, &owner,
                                              &items_, &recipes_, capacity, wallet);
    FactoryStation* p = f.get();
    factories_.push_back(std::move(f));
    RegisterFacility(p);
    owner.AddFacility(p);
    return *p;
  }

  TradeStation& AddTradeStation(Faction& owner, std::string name, Vec2 pos,
                                int capacity, Money wallet) {
    auto t = std::make_unique<TradeStation>(NextFid(), std::move(name), pos, &owner, &items_, capacity, wallet);
    TradeStation* p = t.get();
    trades_.push_back(std::move(t));
    RegisterFacility(p);
    owner.AddFacility(p);
    return *p;
  }

  Shipyard& AddShipyard(Faction& owner, std::string name, Vec2 pos,
                        int capacity, Money wallet) {
    auto y = std::make_unique<Shipyard>(NextFid(), std::move(name), pos, &owner, &items_, capacity, wallet);
    Shipyard* p = y.get();
    shipyards_.push_back(std::move(y));
    RegisterFacility(p);
    owner.AddFacility(p);
    return *p;
  }

  MineArea& AddMineArea(std::string name, Vec2 pos, ItemId ore, int yield_per_tick) {
    auto m = std::make_unique<MineArea>(NextFid(), std::move(name), pos, ore, yield_per_tick);
    MineArea* p = m.get();
    mines_.push_back(std::move(m));
    RegisterFacility(p);
    return *p;
  }

  NPC& AddNpc(std::string name, NpcTraits traits) {
    auto n = std::make_unique<NPC>(NextNid(), std::move(name), traits);
    NPC* p = n.get();
    npcs_[p->Id()] = std::move(n);
    return *p;
  }

  // 把 NPC 分配到设施
  void AssignNpc(NPC& npc, Facility& f, Faction& employer) {
    npc.SetAssignment(&f);
    f.SetOperator(&npc);
    employer.AddNpc(npc.Id());
  }

  // === lookups ===
  Facility* GetFacility(FacilityId id) const {
    auto it = facility_index_.find(id);
    return (it == facility_index_.end()) ? nullptr : it->second;
  }
  NPC* GetNpc(NpcId id) const {
    auto it = npcs_.find(id);
    return (it == npcs_.end()) ? nullptr : it->second.get();
  }

  const std::vector<std::unique_ptr<Ship>>& Ships() const { return ships_; }
  const std::vector<std::unique_ptr<FactoryStation>>& Factories() const { return factories_; }
  const std::vector<std::unique_ptr<TradeStation>>& Trades() const { return trades_; }
  const std::vector<std::unique_ptr<Shipyard>>& Shipyards() const { return shipyards_; }
  const std::vector<std::unique_ptr<MineArea>>& Mines() const { return mines_; }
  const std::unordered_map<FacilityId, std::unique_ptr<Faction>>& Factions() const { return factions_; }

  // 收集所有站作为 IMarket 列表（飞船/Skill 用）
  std::vector<StationBase*> AllStations() const {
    std::vector<StationBase*> v;
    for (auto& p : factories_) v.push_back(p.get());
    for (auto& p : trades_) v.push_back(p.get());
    for (auto& p : shipyards_) v.push_back(p.get());
    return v;
  }

  GameTime Now() const { return now_; }

  // === main loop ===
  void Run() {
    WriteInitDump();
    DBG_PRINT("World run: %llu ticks", static_cast<unsigned long long>(cfg_.total_ticks));
    for (uint64_t i = 0; i < cfg_.total_ticks; ++i) {
      Tick();
      if (!cfg_.fast) {
        std::this_thread::sleep_for(std::chrono::milliseconds(cfg_.tick_ms));
      }
    }
    logger_.Flush();
  }

  void WriteInitDump() {
    auto& os = logger_.InitStream();
    os << "==================== 世界初始化 ====================\n";
    os << "随机种子: " << cfg_.seed << "\n";
    os << "总 tick 数: " << cfg_.total_ticks << "\n";
    os << "tick 间隔(ms): " << cfg_.tick_ms << (cfg_.fast ? " (fast)" : "") << "\n";
    os << "快照间隔: " << cfg_.snapshot_every << " ticks\n";
    os << "航行损坏概率: " << cfg_.damage_prob_per_tick << " / tick (每次 "
       << cfg_.damage_min << "-" << cfg_.damage_max << " 点耐久)\n";
    os << "停泊半径: " << cfg_.dock_radius << "  采矿半径: " << cfg_.mine_radius << "\n";
    os << "修理: 1 " << items_.Get(cfg_.repair_item_hull_plate).name
       << " 恢复 " << cfg_.hp_per_hull_plate << " 耐久; 1 "
       << items_.Get(cfg_.repair_item_engine_part).name
       << " 恢复 " << cfg_.hp_per_engine_part << " 耐久\n";

    os << "\n--- 商品 ---\n";
    for (auto& it : items_.All()) {
      os << "  [" << it.id << "] " << it.name << " 层级=" << TierName(it.tier)
         << " 基准价=" << it.base_price << " 单位体积=" << it.unit_volume << "\n";
    }

    os << "\n--- 配方 ---\n";
    for (auto& r : recipes_.All()) {
      os << "  [" << r.id << "] " << r.name << " 耗时=" << r.ticks_required << " ticks\n";
      os << "      输入:";
      for (auto& [item, qty] : r.inputs) os << " " << qty << " " << items_.Get(item).name;
      os << "\n      输出:";
      for (auto& [item, qty] : r.outputs) os << " " << qty << " " << items_.Get(item).name;
      os << "\n";
    }

    os << "\n--- 矿区 ---\n";
    for (auto& m : mines_) {
      os << "  " << m->Name() << " 坐标=(" << m->Position().x << "," << m->Position().y
         << ") 产出=" << items_.Get(m->Ore()).name
         << " 每 tick 产量=" << m->YieldPerTick() << "\n";
    }

    os << "\n--- 势力 ---\n";
    for (auto& [fid, f] : factions_) {
      os << "  势力 " << f->Name() << " 初始金库=" << f->WalletRef().Balance() << "\n";
      NPC* cmd = GetNpc(f->Commander());
      if (cmd) {
        os << "    主管NPC: " << cmd->Name()
           << " (风险偏好=" << cmd->Traits().risk
           << " 贪婪度=" << cmd->Traits().greed << ")\n";
      }
      os << "    资产:\n";
      for (Facility* asset : f->Facilities()) {
        DumpFacility(os, *asset);
      }
    }

    os << "\n--- 所有 NPC ---\n";
    for (auto& [nid, n] : npcs_) {
      os << "  [" << n->Id() << "] " << n->Name()
         << " 风险=" << n->Traits().risk << " 贪婪=" << n->Traits().greed;
      if (n->Assignment()) {
        os << " 任职于: " << n->Assignment()->Name();
      } else {
        os << " 未派遣";
      }
      os << "\n";
    }
    os << "====================================================\n";
    logger_.Flush();
  }

  void Tick() {
    ++now_;
    for (auto& s : ships_) AdvanceShip(*s);
    for (auto& f : factories_) AdvanceFactory(*f);
    for (auto& y : shipyards_) AdvanceShipyard(*y);
    DrainEvents();
    for (auto& [id, npc] : npcs_) npc->Tick(*this, now_);
    if (now_ % cfg_.snapshot_every == 0) WriteSnapshot();
  }

  void WriteSnapshot() {
    logger_.SnapshotHeader(now_);
    auto& os = logger_.SnapshotStream();
    for (auto& [id, f] : factions_) {
      os << "势力 " << f->Name() << " 金库=" << f->WalletRef().Balance() << "\n";
    }
    for (auto& s : ships_) {
      os << "  飞船 " << s->Name() << " 坐标=(" << s->Position().x << "," << s->Position().y
         << ") 耐久=" << s->Hull() << "/" << s->HullMax()
         << " 钱包=" << s->WalletRef().Balance()
         << " 货舱=" << s->Cargo().Used() << "/" << s->Cargo().Capacity()
         << " 状态=" << ShipStateName(s->State()) << "\n";
    }
    for (auto& f : factories_) {
      os << "  工厂 " << f->Name() << " 钱包=" << f->WalletRef().Balance()
         << " 库存=" << f->InventoryRef().Used() << "/" << f->InventoryRef().Capacity()
         << " 配方=" << f->ActiveRecipe() << " 进度=" << f->Progress() << "\n";
      DumpInventory(os, f->InventoryRef());
    }
    for (auto& t : trades_) {
      os << "  贸易站 " << t->Name() << " 钱包=" << t->WalletRef().Balance()
         << " 库存=" << t->InventoryRef().Used() << "/" << t->InventoryRef().Capacity() << "\n";
      DumpInventory(os, t->InventoryRef());
    }
    for (auto& y : shipyards_) {
      os << "  造船厂 " << y->Name() << " 钱包=" << y->WalletRef().Balance()
         << " 建造队列=" << y->QueueDepth()
         << " 库存=" << y->InventoryRef().Used() << "/" << y->InventoryRef().Capacity() << "\n";
      DumpInventory(os, y->InventoryRef());
    }
  }

 private:
  static const char* ShipStateName(ShipState s) {
    switch (s) {
      case ShipState::Idle: return "怠速";
      case ShipState::Moving: return "航行";
      case ShipState::Docked: return "停泊";
      case ShipState::Mining: return "采矿";
      case ShipState::Trading: return "交易";
      case ShipState::Repairing: return "修理";
      case ShipState::Broken: return "残骸";
    }
    return "未知";
  }

  static const char* TierName(ItemTier t) {
    switch (t) {
      case ItemTier::Ore: return "原料";
      case ItemTier::Intermediate: return "中间品";
      case ItemTier::Component: return "配件";
    }
    return "未知";
  }

  void DumpFacility(std::ostream& os, Facility& f) {
    switch (f.Type()) {
      case FacilityType::Ship: {
        auto& s = static_cast<Ship&>(f);
        os << "      飞船 " << s.Name() << " 坐标=(" << s.Position().x << "," << s.Position().y
           << ") 速度=" << s.Speed()
           << " 耐久=" << s.Hull() << "/" << s.HullMax()
           << " 货舱=" << s.Cargo().Capacity()
           << " 钱包=" << s.WalletRef().Balance();
        if (s.Operator()) os << " 船长: " << s.Operator()->Name();
        os << "\n";
        break;
      }
      case FacilityType::Factory: {
        auto& fs = static_cast<FactoryStation&>(f);
        os << "      工厂 " << fs.Name() << " 坐标=(" << fs.Position().x << ","
           << fs.Position().y << ") 库存上限=" << fs.InventoryRef().Capacity()
           << " 钱包=" << fs.WalletRef().Balance();
        if (fs.ActiveRecipe() != 0)
          os << " 当前配方=" << recipes_.Get(fs.ActiveRecipe()).name;
        if (fs.Operator()) os << " 厂长: " << fs.Operator()->Name();
        os << "\n";
        DumpInventoryIndent(os, fs.InventoryRef(), "        ");
        break;
      }
      case FacilityType::Trade: {
        auto& ts = static_cast<TradeStation&>(f);
        os << "      贸易站 " << ts.Name() << " 坐标=(" << ts.Position().x << ","
           << ts.Position().y << ") 库存上限=" << ts.InventoryRef().Capacity()
           << " 钱包=" << ts.WalletRef().Balance();
        if (ts.Operator()) os << " 做市员: " << ts.Operator()->Name();
        os << "\n";
        DumpInventoryIndent(os, ts.InventoryRef(), "        ");
        break;
      }
      case FacilityType::Shipyard: {
        auto& y = static_cast<Shipyard&>(f);
        const auto& bp = y.Blueprint();
        os << "      造船厂 " << y.Name() << " 坐标=(" << y.Position().x << ","
           << y.Position().y << ") 库存上限=" << y.InventoryRef().Capacity()
           << " 钱包=" << y.WalletRef().Balance();
        if (y.Operator()) os << " 船厂长: " << y.Operator()->Name();
        os << "\n";
        os << "        蓝图: 耗时=" << bp.ticks_required << " 资金成本=" << bp.cost
           << " 材料:";
        for (auto& [item, qty] : bp.materials) os << " " << qty << " " << items_.Get(item).name;
        os << " => 飞船(速度=" << bp.ship_speed << " 货舱=" << bp.ship_cargo_capacity
           << " 耐久=" << bp.ship_hull_max << ")\n";
        DumpInventoryIndent(os, y.InventoryRef(), "        ");
        break;
      }
      default:
        break;
    }
  }

  void DumpInventoryIndent(std::ostream& os, const Inventory& inv, const char* indent) {
    if (inv.Items().empty()) return;
    os << indent << "库存:";
    for (auto& [id, qty] : inv.Items()) {
      os << " " << items_.Get(id).name << "=" << qty;
    }
    os << "\n";
  }
  void DumpInventory(std::ostream& os, const Inventory& inv) {
    for (auto& [id, qty] : inv.Items()) {
      os << "    " << items_.Get(id).name << "=" << qty << "\n";
    }
  }

  FacilityId NextFid() { return ++next_fid_; }
  NpcId NextNid() { return ++next_nid_; }
  void RegisterFacility(Facility* f) { facility_index_[f->Id()] = f; }

  void DrainEvents() {
    while (!events_.Empty()) {
      PendingEvent pe = events_.Pop();
      RouteEvent(pe);
    }
  }

  void RouteEvent(const PendingEvent& pe) {
    if (pe.target == EventTarget::Global) return;
    if (pe.target == EventTarget::Facility) {
      Facility* f = GetFacility(pe.target_id);
      if (!f) return;
      if (f->Operator()) f->Operator()->DeliverEvent(*this, pe.event);
      // 上送 commander
      Faction* fac = f->Owner();
      if (fac && fac->Commander() != kNullNpc) {
        NPC* cmd = GetNpc(fac->Commander());
        if (cmd && cmd != f->Operator()) cmd->DeliverEvent(*this, pe.event);
      }
    } else if (pe.target == EventTarget::Faction) {
      auto it = factions_.find(pe.target_id);
      if (it == factions_.end()) return;
      NPC* cmd = GetNpc(it->second->Commander());
      if (cmd) cmd->DeliverEvent(*this, pe.event);
    }
  }

  void AdvanceShip(Ship& s) {
    if (s.State() == ShipState::Broken) return;

    if (!s.HasCommand()) {
      if (s.State() != ShipState::Idle) {
        s.SetState(ShipState::Idle);
        events_.PublishToFacility(s.Id(), EvShipIdle{s.Id()});
      }
      return;
    }

    ShipCommand& cmd = s.CurrentCommand();
    uint64_t cmd_id = s.CurrentCommandId();

    std::visit([&](auto& c) {
      using T = std::decay_t<decltype(c)>;
      if constexpr (std::is_same_v<T, ShipCmdMoveTo>) {
        s.SetState(ShipState::Moving);
        bool arrived = s.StepToward(c.pos, 0.5f);
        // 损坏判定
        if (rng_.Chance(cfg_.damage_prob_per_tick)) {
          int dmg = rng_.IntRange(cfg_.damage_min, cfg_.damage_max);
          s.ApplyDamage(dmg);
          logger_.Eventf(now_, "%s 受损 -%d 耐久 (剩余 %d/%d)",
                         s.Name().c_str(), dmg, s.Hull(), s.HullMax());
          events_.PublishToFacility(s.Id(), EvShipDamaged{s.Id(), dmg, s.Hull()});
          if (s.Hull() == 0) {
            s.SetState(ShipState::Broken);
            s.ClearCommands();
            logger_.Eventf(now_, "%s 报废 于 (%.1f,%.1f)",
                           s.Name().c_str(), s.Position().x, s.Position().y);
            events_.PublishToFacility(s.Id(), EvShipDestroyed{s.Id(), s.Position()});
            return;
          }
        }
        if (arrived) {
          // 查看终点是否是 facility
          FacilityId station_id = NearestFacilityAt(s.Position(), cfg_.dock_radius);
          logger_.Eventf(now_, "%s 抵达 (%.1f,%.1f)%s",
                         s.Name().c_str(), s.Position().x, s.Position().y,
                         station_id ? " 设施附近" : "");
          events_.PublishToFacility(s.Id(), EvShipArrived{s.Id(), station_id, s.Position()});
          events_.PublishToFacility(s.Id(), EvCommandCompleted{s.Id(), cmd_id});
          s.PopCommand();
          if (!s.HasCommand()) s.SetState(ShipState::Idle);
        }
      } else if constexpr (std::is_same_v<T, ShipCmdMine>) {
        MineArea* m = FindMine(c.mine_area);
        if (!m || s.Position().Distance(m->Position()) > cfg_.mine_radius) {
          logger_.Eventf(now_, "%s 采矿失败：未在矿区范围内", s.Name().c_str());
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "not at mine area"});
          s.PopCommand();
          return;
        }
        s.SetState(ShipState::Mining);
        int added = s.Cargo().Add(m->Ore(), m->YieldPerTick());
        if (added > 0) {
          logger_.Eventf(now_, "%s 采矿 +%d %s (货舱 %d/%d)",
                         s.Name().c_str(), added,
                         items_.Get(m->Ore()).name.c_str(),
                         s.Cargo().Used(), s.Cargo().Capacity());
        }
        bool full = (s.Cargo().Free() == 0);
        bool target_done = (c.qty_target > 0 && s.Cargo().Get(c.item) >= c.qty_target);
        if (full || target_done || added == 0) {
          events_.PublishToFacility(s.Id(), EvCommandCompleted{s.Id(), cmd_id});
          s.PopCommand();
          if (!s.HasCommand()) s.SetState(ShipState::Idle);
        }
      } else if constexpr (std::is_same_v<T, ShipCmdBuy>) {
        Facility* f = GetFacility(c.station);
        if (!f || s.Position().Distance(f->Position()) > cfg_.dock_radius) {
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "not docked"});
          s.PopCommand();
          return;
        }
        auto* mkt = dynamic_cast<IMarket*>(f);
        auto* sb = dynamic_cast<StationBase*>(f);
        if (!mkt || !sb) {
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "no market"});
          s.PopCommand();
          return;
        }
        // 飞船买：站卖（c.quote.side == Sell）
        // 检查飞船钱、货舱
        int want = std::min(c.qty, s.Cargo().Free());
        int afford = (c.quote.price > 0) ? static_cast<int>(s.WalletRef().Balance() / c.quote.price) : 0;
        want = std::min(want, afford);
        if (want <= 0) {
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "cannot afford"});
          s.PopCommand();
          return;
        }
        s.SetState(ShipState::Trading);
        int filled = mkt->Trade(c.quote, want, s.Id());
        if (filled > 0) {
          s.WalletRef().Debit(static_cast<Money>(c.quote.price) * filled);
          s.Cargo().Add(c.quote.item, filled);
          logger_.Eventf(now_, "%s 买入 %d %s @ %lld 自 %s (钱包=%lld)",
                         s.Name().c_str(), filled,
                         items_.Get(c.quote.item).name.c_str(),
                         (long long)c.quote.price, f->Name().c_str(),
                         (long long)s.WalletRef().Balance());
          events_.PublishToFacility(f->Id(),
              EvTradeExecuted{f->Id(), s.Id(), c.quote.item, Quote::Sell, c.quote.price, filled});
        } else {
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "trade rejected"});
        }
        events_.PublishToFacility(s.Id(), EvCommandCompleted{s.Id(), cmd_id});
        s.PopCommand();
      } else if constexpr (std::is_same_v<T, ShipCmdSell>) {
        Facility* f = GetFacility(c.station);
        if (!f || s.Position().Distance(f->Position()) > cfg_.dock_radius) {
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "not docked"});
          s.PopCommand();
          return;
        }
        auto* mkt = dynamic_cast<IMarket*>(f);
        if (!mkt) {
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "no market"});
          s.PopCommand();
          return;
        }
        int avail = s.Cargo().Get(c.quote.item);
        int want = std::min(c.qty, avail);
        if (want <= 0) {
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "no cargo"});
          s.PopCommand();
          return;
        }
        s.SetState(ShipState::Trading);
        int filled = mkt->Trade(c.quote, want, s.Id());
        if (filled > 0) {
          s.Cargo().Remove(c.quote.item, filled);
          s.WalletRef().Credit(static_cast<Money>(c.quote.price) * filled);
          logger_.Eventf(now_, "%s 卖出 %d %s @ %lld 至 %s (钱包=%lld)",
                         s.Name().c_str(), filled,
                         items_.Get(c.quote.item).name.c_str(),
                         (long long)c.quote.price, f->Name().c_str(),
                         (long long)s.WalletRef().Balance());
          events_.PublishToFacility(f->Id(),
              EvTradeExecuted{f->Id(), s.Id(), c.quote.item, Quote::Buy, c.quote.price, filled});
        } else {
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "trade rejected"});
        }
        events_.PublishToFacility(s.Id(), EvCommandCompleted{s.Id(), cmd_id});
        s.PopCommand();
      } else if constexpr (std::is_same_v<T, ShipCmdRepair>) {
        Facility* f = GetFacility(c.station);
        if (!f || s.Position().Distance(f->Position()) > cfg_.dock_radius) {
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "not docked"});
          s.PopCommand();
          return;
        }
        auto* sb = dynamic_cast<StationBase*>(f);
        if (!sb) {
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "not a station"});
          s.PopCommand();
          return;
        }
        s.SetState(ShipState::Repairing);
        int missing = s.HullMax() - s.Hull();
        int hp_per_set = cfg_.hp_per_hull_plate + cfg_.hp_per_engine_part;
        int sets_needed = (missing + hp_per_set - 1) / hp_per_set;
        int sets_avail = std::min(sb->InventoryRef().Get(cfg_.repair_item_hull_plate),
                                  sb->InventoryRef().Get(cfg_.repair_item_engine_part));
        int sets = std::min(sets_needed, sets_avail);
        if (sets <= 0) {
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "no repair parts"});
          s.PopCommand();
          return;
        }
        // 简化：按零售价支付
        const auto* hp_cfg = FindConfig(*sb, cfg_.repair_item_hull_plate);
        const auto* ep_cfg = FindConfig(*sb, cfg_.repair_item_engine_part);
        Money hp_price = items_.Get(cfg_.repair_item_hull_plate).base_price;
        Money ep_price = items_.Get(cfg_.repair_item_engine_part).base_price;
        Money cost = (hp_price + ep_price) * sets;
        if (s.WalletRef().Balance() < cost) {
          // 削减到付得起
          if (hp_price + ep_price > 0) {
            int affordable = static_cast<int>(s.WalletRef().Balance() / (hp_price + ep_price));
            sets = std::min(sets, affordable);
            cost = (hp_price + ep_price) * sets;
          } else {
            sets = 0;
          }
        }
        if (sets <= 0) {
          events_.PublishToFacility(s.Id(), EvCommandFailed{s.Id(), cmd_id, "cannot afford repair"});
          s.PopCommand();
          return;
        }
        sb->InventoryRef().Remove(cfg_.repair_item_hull_plate, sets);
        sb->InventoryRef().Remove(cfg_.repair_item_engine_part, sets);
        sb->WalletRef().Credit(cost);
        s.WalletRef().Debit(cost);
        int restored = sets * hp_per_set;
        s.SetHull(s.Hull() + restored);
        logger_.Eventf(now_, "%s 修理 +%d 耐久 于 %s (花费 %lld)",
                       s.Name().c_str(), restored, f->Name().c_str(), (long long)cost);
        events_.PublishToFacility(s.Id(), EvCommandCompleted{s.Id(), cmd_id});
        s.PopCommand();
        (void)hp_cfg;
        (void)ep_cfg;
      } else if constexpr (std::is_same_v<T, ShipCmdIdle>) {
        s.SetState(ShipState::Idle);
        if (c.ticks > 0) c.ticks--;
        if (c.ticks <= 0) {
          events_.PublishToFacility(s.Id(), EvCommandCompleted{s.Id(), cmd_id});
          s.PopCommand();
        }
      }
    }, cmd);
  }

  void AdvanceFactory(FactoryStation& f) {
    auto r = f.ProductionTick();
    switch (r) {
      case FactoryStation::TickResult::Produced: {
        const Recipe& rec = recipes_.Get(f.ActiveRecipe());
        std::string outputs;
        for (auto& [item, qty] : rec.outputs) {
          if (!outputs.empty()) outputs += ", ";
          outputs += std::to_string(qty) + " " + items_.Get(item).name;
        }
        std::string inputs;
        for (auto& [item, qty] : rec.inputs) {
          if (!inputs.empty()) inputs += ", ";
          inputs += "-" + std::to_string(qty) + " " + items_.Get(item).name +
                    "(剩" + std::to_string(f.InventoryRef().Get(item)) + ")";
        }
        logger_.Eventf(now_, "%s 产出 %s [配方:%s 消耗:%s 库存:%d/%d]",
                       f.Name().c_str(), outputs.c_str(), rec.name.c_str(),
                       inputs.c_str(),
                       f.InventoryRef().Used(), f.InventoryRef().Capacity());
        events_.PublishToFacility(f.Id(), EvFactoryProduced{f.Id(), f.ActiveRecipe()});
        break;
      }
      case FactoryStation::TickResult::StalledInput:
        events_.PublishToFacility(f.Id(), EvProductionStalled{f.Id(), f.ActiveRecipe(), "input"});
        break;
      case FactoryStation::TickResult::StalledOutput:
        events_.PublishToFacility(f.Id(), EvProductionStalled{f.Id(), f.ActiveRecipe(), "output"});
        break;
      default:
        break;
    }
  }

  void AdvanceShipyard(Shipyard& y) {
    auto r = y.Tick();
    if (r == Shipyard::TickResult::Built) {
      const auto& bp = y.Blueprint();
      Faction* owner = y.Owner();
      Ship& new_ship = AddShip(*owner, owner->Name() + "新船" + std::to_string(NextFid()),
                               y.Position(), bp.ship_speed, bp.ship_cargo_capacity,
                               bp.ship_hull_max, 100);
      logger_.Eventf(now_, "%s 建成 新飞船 %s",
                     y.Name().c_str(), new_ship.Name().c_str());
      events_.PublishToFacility(y.Id(), EvShipBuilt{y.Id(), new_ship.Id()});
    } else if (r == Shipyard::TickResult::StalledInput) {
      events_.PublishToFacility(y.Id(), EvProductionStalled{y.Id(), 0, "shipyard input"});
    }
  }

  MineArea* FindMine(FacilityId id) {
    for (auto& m : mines_) if (m->Id() == id) return m.get();
    return nullptr;
  }

  FacilityId NearestFacilityAt(Vec2 pos, float radius) {
    for (auto& [fid, f] : facility_index_) {
      if (f->Type() == FacilityType::Faction) continue;
      if (f->Type() == FacilityType::Ship) continue;
      if (pos.Distance(f->Position()) <= radius) return fid;
    }
    return kNullFacility;
  }

  const ItemMarketConfig* FindConfig(const StationBase& sb, ItemId id) {
    auto it = sb.Configs().find(id);
    return (it == sb.Configs().end()) ? nullptr : &it->second;
  }

  WorldConfig cfg_;
  WorldRng rng_;
  Logger logger_;
  EventBus events_;
  ItemRegistry items_;
  RecipeRegistry recipes_;

  std::vector<std::unique_ptr<Ship>> ships_;
  std::vector<std::unique_ptr<FactoryStation>> factories_;
  std::vector<std::unique_ptr<TradeStation>> trades_;
  std::vector<std::unique_ptr<Shipyard>> shipyards_;
  std::vector<std::unique_ptr<MineArea>> mines_;
  std::unordered_map<FacilityId, std::unique_ptr<Faction>> factions_;
  std::unordered_map<NpcId, std::unique_ptr<NPC>> npcs_;
  std::unordered_map<FacilityId, Facility*> facility_index_;

  FacilityId next_fid_ = 0;
  NpcId next_nid_ = 0;
  GameTime now_ = 0;
};

}  // namespace ytlib::testgame3
