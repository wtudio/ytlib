/**
 * @file quote.hpp
 * @brief 市场报价数据结构
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include "core/types.hpp"

namespace ytlib::testgame3 {

struct Quote {
  enum Side : uint8_t { Buy, Sell };

  FacilityId owner = kNullFacility;  // 报价归属（站自己 / 未来势力挂单）
  Side side = Sell;
  ItemId item = 0;
  Money price = 0;
  int qty = 0;  // 该报价当前可成交数量
};

}  // namespace ytlib::testgame3
