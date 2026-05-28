/**
 * @file order.hpp
 * @brief Commander 下达给下属 NPC 的高层意图
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <variant>

#include "core/types.hpp"

namespace ytlib::testgame3 {

struct OrderHuntProfit {
  Money budget_hint;  // 参考预算（0 = 自决）
};

struct OrderOperateFactory {
  RecipeId preferred;
};

struct OrderMakeMarket {
};

struct OrderBuildShip {
  uint32_t blueprint_id;
};

using Order =
    std::variant<OrderHuntProfit, OrderOperateFactory, OrderMakeMarket, OrderBuildShip>;

}  // namespace ytlib::testgame3
