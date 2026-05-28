/**
 * @file factory_station.hpp
 * @brief 工厂站：消耗原料生产中间品/部件
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include "core/recipe.hpp"
#include "facility/station_base.hpp"

namespace ytlib::testgame3 {

class FactoryStation : public StationBase {
 public:
  FactoryStation(FacilityId id, std::string name, Vec2 pos, Faction* owner,
                 const ItemRegistry* items, const RecipeRegistry* recipes,
                 int capacity, Money wallet)
      : StationBase(id, std::move(name), FacilityType::Factory, pos, owner, items, capacity, wallet),
        recipes_(recipes) {}

  void SetRecipe(RecipeId rid) {
    active_recipe_ = rid;
    progress_ = 0;
  }
  RecipeId ActiveRecipe() const { return active_recipe_; }
  int Progress() const { return progress_; }
  void SetEnabled(bool e) { enabled_ = e; }
  bool Enabled() const { return enabled_; }

  // 工厂生产 tick；返回值：0 = 无变化；1 = 完成一次生产；-1 = 卡住（原料不足/库存满）
  enum class TickResult { Idle, Working, Produced, StalledInput, StalledOutput };
  TickResult ProductionTick() {
    if (!enabled_ || active_recipe_ == 0) return TickResult::Idle;
    const Recipe& r = recipes_->Get(active_recipe_);

    if (progress_ == 0) {
      // 检查原料
      for (auto& [item, qty] : r.inputs) {
        if (inventory_.Get(item) < qty) return TickResult::StalledInput;
      }
      // 检查产出空间
      int out_size = 0;
      for (auto& [item, qty] : r.outputs) out_size += qty;
      if (inventory_.Free() < out_size) return TickResult::StalledOutput;
      // 扣原料，开工
      for (auto& [item, qty] : r.inputs) inventory_.Remove(item, qty);
      progress_ = 1;
      return TickResult::Working;
    }
    progress_++;
    if (progress_ >= r.ticks_required) {
      for (auto& [item, qty] : r.outputs) inventory_.Add(item, qty);
      progress_ = 0;
      return TickResult::Produced;
    }
    return TickResult::Working;
  }

 private:
  const RecipeRegistry* recipes_;
  RecipeId active_recipe_ = 0;
  int progress_ = 0;
  bool enabled_ = true;
};

}  // namespace ytlib::testgame3
