/**
 * @file recipe.hpp
 * @brief 配方定义与注册表
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "core/types.hpp"

namespace ytlib::testgame3 {

struct Recipe {
  RecipeId id;
  std::string name;
  std::vector<std::pair<ItemId, int>> inputs;   // (item, qty)
  std::vector<std::pair<ItemId, int>> outputs;  // (item, qty)
  int ticks_required;                            // 生产耗时
};

class RecipeRegistry {
 public:
  RecipeId Register(std::string name,
                    std::vector<std::pair<ItemId, int>> inputs,
                    std::vector<std::pair<ItemId, int>> outputs,
                    int ticks_required) {
    RecipeId id = static_cast<RecipeId>(recipes_.size() + 1);
    recipes_.push_back(Recipe{id, std::move(name), std::move(inputs), std::move(outputs), ticks_required});
    return id;
  }

  const Recipe& Get(RecipeId id) const { return recipes_.at(id - 1); }
  const std::vector<Recipe>& All() const { return recipes_; }

 private:
  std::vector<Recipe> recipes_;
};

}  // namespace ytlib::testgame3
