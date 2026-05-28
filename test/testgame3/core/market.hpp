/**
 * @file market.hpp
 * @brief 市场接口（方案 D：库存反馈报价，可升级到方案 C 订单簿）
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <vector>

#include "core/quote.hpp"
#include "core/types.hpp"

namespace ytlib::testgame3 {

class IMarket {
 public:
  virtual ~IMarket() = default;

  // 查询该 item 在此市场上的所有当前报价（买单/卖单）。
  virtual std::vector<Quote> GetQuotes(ItemId item) const = 0;

  // 飞船 counterparty 用 quote 成交 qty 单位。返回实际成交数量。
  // 失败（库存不足/钱不足/quote 过期等）返回 0 或部分成交数量。
  virtual int Trade(const Quote& quote, int qty, FacilityId counterparty) = 0;
};

}  // namespace ytlib::testgame3
