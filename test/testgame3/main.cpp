/**
 * @file main.cpp
 * @brief testgame3：太空经济模拟 MVP demo 场景
 * @author wtudio
 * @date 2026-05-28
 */
#include <cstring>
#include <memory>
#include <string>

#include "core/world.hpp"
#include "skill/faction_commander_skill.hpp"
#include "skill/factory_manager_skill.hpp"
#include "skill/ship_pilot_skill.hpp"
#include "skill/trader_skill.hpp"
#include "ytlib/misc/misc_macro.h"

using namespace ytlib::testgame3;

namespace {

struct ItemIds {
  ItemId iron_ore;
  ItemId silicon_ore;
  ItemId steel_plate;
  ItemId chip;
  ItemId engine_part;
  ItemId hull_plate;
};

struct RecipeIds {
  RecipeId steel;
  RecipeId chip;
  RecipeId engine_part;
  RecipeId hull_plate;
};

ItemIds RegisterItems(World& w) {
  ItemIds ids;
  ids.iron_ore = w.Items().Register("铁矿石", ItemTier::Ore, 5);
  ids.silicon_ore = w.Items().Register("硅矿石", ItemTier::Ore, 7);
  ids.steel_plate = w.Items().Register("钢板", ItemTier::Intermediate, 25);
  ids.chip = w.Items().Register("芯片", ItemTier::Intermediate, 40);
  ids.engine_part = w.Items().Register("引擎部件", ItemTier::Component, 120);
  ids.hull_plate = w.Items().Register("船壳板", ItemTier::Component, 90);
  return ids;
}

RecipeIds RegisterRecipes(World& w, const ItemIds& i) {
  RecipeIds r;
  r.steel = w.Recipes().Register("炼钢", {{i.iron_ore, 3}}, {{i.steel_plate, 1}}, 10);
  r.chip = w.Recipes().Register("制芯片", {{i.silicon_ore, 2}}, {{i.chip, 1}}, 12);
  r.engine_part = w.Recipes().Register("造引擎部件",
                                       {{i.steel_plate, 2}, {i.chip, 1}}, {{i.engine_part, 1}}, 20);
  r.hull_plate = w.Recipes().Register("造船壳板", {{i.steel_plate, 3}}, {{i.hull_plate, 1}}, 15);
  return r;
}

void ConfigureTradeStation(StationBase& s, const ItemIds& i) {
  ItemMarketConfig ore{/*target*/ 200, /*max*/ 400};
  ItemMarketConfig mid{/*target*/ 150, /*max*/ 300};
  ItemMarketConfig comp{/*target*/ 50, /*max*/ 100};
  s.Configure(i.iron_ore, ore);
  s.Configure(i.silicon_ore, ore);
  s.Configure(i.steel_plate, mid);
  s.Configure(i.chip, mid);
  s.Configure(i.engine_part, comp);
  s.Configure(i.hull_plate, comp);
}

void ConfigureFactory(StationBase& s, const ItemIds& i) {
  ItemMarketConfig ore_buy{/*target*/ 100, /*max*/ 300};
  ore_buy.will_sell = false;
  ItemMarketConfig mid_sell{/*target*/ 50, /*max*/ 200};
  mid_sell.will_buy = false;
  ItemMarketConfig comp_sell{/*target*/ 30, /*max*/ 100};
  comp_sell.will_buy = false;
  s.Configure(i.iron_ore, ore_buy);
  s.Configure(i.silicon_ore, ore_buy);
  s.Configure(i.steel_plate, mid_sell);
  s.Configure(i.chip, mid_sell);
  s.Configure(i.engine_part, comp_sell);
  s.Configure(i.hull_plate, comp_sell);
}

void ConfigureShipyard(StationBase& s, const ItemIds& i) {
  ItemMarketConfig comp_buy{/*target*/ 30, /*max*/ 100};
  comp_buy.will_sell = false;
  ItemMarketConfig mid_buy{/*target*/ 50, /*max*/ 200};
  mid_buy.will_sell = false;
  s.Configure(i.engine_part, comp_buy);
  s.Configure(i.hull_plate, comp_buy);
  s.Configure(i.steel_plate, mid_buy);
  s.Configure(i.chip, mid_buy);
}

ShipBlueprint MakeBlueprint(const ItemIds& i) {
  ShipBlueprint bp;
  bp.id = 1;
  bp.materials = {{i.engine_part, 3}, {i.hull_plate, 5}, {i.steel_plate, 10}};
  bp.cost = 500;
  bp.ticks_required = 50;
  bp.ship_speed = 2.0f;
  bp.ship_cargo_capacity = 30;
  bp.ship_hull_max = 100;
  return bp;
}

NPC& MakePilot(World& w, std::string name, NpcTraits t) {
  NPC& n = w.AddNpc(std::move(name), t);
  n.AddSkill(std::make_unique<ShipPilotSkill>());
  return n;
}

NPC& MakeFactoryManager(World& w, std::string name) {
  NPC& n = w.AddNpc(std::move(name), NpcTraits{0.5f, 0.5f});
  n.AddSkill(std::make_unique<FactoryManagerSkill>());
  return n;
}

NPC& MakeTrader(World& w, std::string name) {
  NPC& n = w.AddNpc(std::move(name), NpcTraits{0.3f, 0.3f});
  n.AddSkill(std::make_unique<TraderSkill>());
  return n;
}

NPC& MakeCommander(World& w, std::string name, NpcTraits t) {
  NPC& n = w.AddNpc(std::move(name), t);
  n.AddSkill(std::make_unique<FactionCommanderSkill>());
  return n;
}

}  // namespace

int main(int argc, char** argv) {
  DBG_PRINT("-------------------start game-------------------");

  WorldConfig cfg;
  cfg.seed = 42;
  cfg.tick_ms = 0;
  cfg.fast = true;
  cfg.total_ticks = 5000;
  cfg.snapshot_every = 200;
  cfg.damage_prob_per_tick = 0.02f;

  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--slow") == 0) {
      cfg.fast = false;
      cfg.tick_ms = 100;
    } else if (std::strcmp(argv[i], "--short") == 0) {
      cfg.total_ticks = 500;
    }
  }

  // 我们要在 cfg 里设置 repair item ids，但 ids 还没注册，因此先注册再用 setter
  World w(cfg, "testgame3_events.log", "testgame3_snapshots.log", "testgame3_init.log");

  ItemIds items = RegisterItems(w);
  RecipeIds recs = RegisterRecipes(w, items);
  w.SetRepairItems(items.hull_plate, items.engine_part);

  MineArea& iron_mine = w.AddMineArea("铁矿带", {-80, 0}, items.iron_ore, 3);
  MineArea& silicon_mine = w.AddMineArea("硅矿带", {80, 0}, items.silicon_ore, 2);
  (void)iron_mine;
  (void)silicon_mine;

  Faction& neutral = w.AddFaction("中立", 10000);
  TradeStation& neutral_trade = w.AddTradeStation(neutral, "中立贸易站", {0, 0}, 2000, 5000);
  ConfigureTradeStation(neutral_trade, items);
  neutral_trade.InventoryRef().Add(items.iron_ore, 100);
  neutral_trade.InventoryRef().Add(items.silicon_ore, 100);
  neutral_trade.InventoryRef().Add(items.steel_plate, 30);
  neutral_trade.InventoryRef().Add(items.chip, 30);
  neutral_trade.InventoryRef().Add(items.engine_part, 20);
  neutral_trade.InventoryRef().Add(items.hull_plate, 20);
  NPC& neutral_cmd = MakeCommander(w, "中立主管", {0.3f, 0.0f});
  neutral.SetCommander(neutral_cmd.Id());
  w.AssignNpc(neutral_cmd, neutral, neutral);
  NPC& neutral_trader = MakeTrader(w, "中立做市员");
  w.AssignNpc(neutral_trader, neutral_trade, neutral);

  auto build_faction = [&](const std::string& fname, Vec2 base_pos, NpcTraits cmd_traits,
                           NpcTraits pilot1_traits, NpcTraits pilot2_traits, RecipeId recipe) {
    Faction& fac = w.AddFaction(fname, 3000);

    FactoryStation& factory = w.AddFactory(fac, fname + "工厂",
                                           Vec2{base_pos.x, base_pos.y + 10}, 1500, 2000);
    ConfigureFactory(factory, items);
    factory.SetRecipe(recipe);
    factory.InventoryRef().Add(items.iron_ore, 40);
    factory.InventoryRef().Add(items.silicon_ore, 40);

    Shipyard& yard = w.AddShipyard(fac, fname + "造船厂",
                                   Vec2{base_pos.x, base_pos.y - 10}, 1000, 1000);
    ConfigureShipyard(yard, items);
    yard.SetBlueprint(MakeBlueprint(items));

    Ship& ship1 = w.AddShip(fac, fname + "号-甲船", base_pos, 2.0f, 30, 100, 300);
    Ship& ship2 = w.AddShip(fac, fname + "号-乙船", base_pos, 2.0f, 30, 100, 300);

    NPC& cmd = MakeCommander(w, fname + "主管", cmd_traits);
    fac.SetCommander(cmd.Id());
    w.AssignNpc(cmd, fac, fac);

    NPC& fm = MakeFactoryManager(w, fname + "厂长");
    w.AssignNpc(fm, factory, fac);
    NPC& ym = MakeFactoryManager(w, fname + "船厂长");
    w.AssignNpc(ym, yard, fac);

    NPC& pilot1 = MakePilot(w, fname + "甲船长", pilot1_traits);
    w.AssignNpc(pilot1, ship1, fac);
    NPC& pilot2 = MakePilot(w, fname + "乙船长", pilot2_traits);
    w.AssignNpc(pilot2, ship2, fac);
  };

  build_faction("红星", {-40, 30}, {0.6f, 0.7f},
                {0.8f, 0.7f}, {0.3f, 0.4f}, recs.steel);
  build_faction("蓝日", {40, -30}, {0.5f, 0.5f},
                {0.7f, 0.6f}, {0.4f, 0.5f}, recs.chip);

  w.Run();

  DBG_PRINT("********************end game*******************");
  return 0;
}
