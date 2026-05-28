/**
 * @file station_base.hpp
 * @brief 站类设施基类：库存 + 钱包 + 库存反馈定价的市场实现
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <unordered_map>
#include <vector>

#include "core/inventory.hpp"
#include "core/item.hpp"
#include "core/market.hpp"
#include "core/wallet.hpp"
#include "facility/facility.hpp"

namespace ytlib::testgame3 {

// 单个商品的市场参数：站对该商品愿意买/卖的目标库存与定价系数。
struct ItemMarketConfig {
  int target_inventory;  // 期望的库存水平
  int max_inventory;     // 库存上限（超过即停止买入）
  float spread = 0.10f;  // 买卖差价百分比（卖价 = mid * (1+spread/2)，买价 = mid * (1-spread/2)）
  float swing = 0.50f;   // 价格随库存波动幅度（库存空时 mid = base*(1+swing)，满时 = base*(1-swing)）
  bool will_buy = true;
  bool will_sell = true;
  int max_trade_per_call = 50;  // 单次交易最多撮合数
};

class StationBase : public Facility, public IMarket {
 public:
  StationBase(FacilityId id, std::string name, FacilityType type, Vec2 pos, Faction* owner,
              const ItemRegistry* items, int inventory_capacity, Money initial_wallet)
      : Facility(id, std::move(name), type, pos, owner),
        items_(items),
        inventory_(inventory_capacity),
        wallet_(initial_wallet) {}

  Inventory& InventoryRef() { return inventory_; }
  const Inventory& InventoryRef() const { return inventory_; }
  Wallet& WalletRef() { return wallet_; }
  const Wallet& WalletRef() const { return wallet_; }

  void Configure(ItemId item, ItemMarketConfig cfg) { configs_[item] = cfg; }
  bool HasConfig(ItemId item) const { return configs_.count(item) > 0; }
  const std::unordered_map<ItemId, ItemMarketConfig>& Configs() const { return configs_; }

  // IMarket
  std::vector<Quote> GetQuotes(ItemId item) const override {
    std::vector<Quote> out;
    auto it = configs_.find(item);
    if (it == configs_.end()) return out;
    const auto& cfg = it->second;
    Money mid = ComputeMidPrice(item, cfg);
    int inv = inventory_.Get(item);

    if (cfg.will_buy && inv < cfg.max_inventory && wallet_.Balance() > 0) {
      Quote q;
      q.owner = id_;
      q.side = Quote::Buy;
      q.item = item;
      q.price = static_cast<Money>(mid * (1.0f - cfg.spread / 2.0f));
      if (q.price < 1) q.price = 1;
      int capacity_room = cfg.max_inventory - inv;
      int affordable = (q.price > 0) ? static_cast<int>(wallet_.Balance() / q.price) : 0;
      q.qty = std::min({cfg.max_trade_per_call, capacity_room, affordable});
      if (q.qty > 0) out.push_back(q);
    }

    if (cfg.will_sell && inv > 0) {
      Quote q;
      q.owner = id_;
      q.side = Quote::Sell;
      q.item = item;
      q.price = static_cast<Money>(mid * (1.0f + cfg.spread / 2.0f));
      if (q.price < 1) q.price = 1;
      q.qty = std::min(cfg.max_trade_per_call, inv);
      out.push_back(q);
    }
    return out;
  }

  // 飞船 counterparty 接受报价。如果 quote.side==Sell，站卖货给飞船（飞船买）；反之亦然。
  int Trade(const Quote& quote, int qty, FacilityId /*counterparty*/) override {
    if (quote.owner != id_) return 0;
    auto it = configs_.find(quote.item);
    if (it == configs_.end()) return 0;
    const auto& cfg = it->second;

    if (quote.side == Quote::Sell) {
      // 站卖货
      int avail = inventory_.Get(quote.item);
      int can = std::min({qty, avail, cfg.max_trade_per_call});
      if (can <= 0) return 0;
      inventory_.Remove(quote.item, can);
      wallet_.Credit(static_cast<Money>(quote.price) * can);
      return can;
    } else {
      // 站买货
      int can_inv = cfg.max_inventory - inventory_.Get(quote.item);
      int can_money = (quote.price > 0) ? static_cast<int>(wallet_.Balance() / quote.price) : 0;
      int can = std::min({qty, can_inv, can_money, cfg.max_trade_per_call});
      if (can <= 0) return 0;
      inventory_.Add(quote.item, can);
      wallet_.Debit(static_cast<Money>(quote.price) * can);
      return can;
    }
  }

 protected:
  Money ComputeMidPrice(ItemId item, const ItemMarketConfig& cfg) const {
    Money base = items_->Get(item).base_price;
    int inv = inventory_.Get(item);
    // 库存比例：< target 价高，> target 价低
    float ratio;
    if (cfg.max_inventory <= 0) {
      ratio = 0.0f;
    } else {
      // 归一到 [-1, 1]：inv == target -> 0；inv == max -> +1；inv == 0 -> -1
      if (inv >= cfg.target_inventory) {
        int span = cfg.max_inventory - cfg.target_inventory;
        ratio = (span > 0) ? static_cast<float>(inv - cfg.target_inventory) / span : 0.0f;
      } else {
        int span = cfg.target_inventory;
        ratio = (span > 0) ? -static_cast<float>(cfg.target_inventory - inv) / span : 0.0f;
      }
    }
    // 库存多 ratio>0 价低；库存少 ratio<0 价高
    float factor = 1.0f - ratio * cfg.swing;
    if (factor < 0.1f) factor = 0.1f;
    return static_cast<Money>(base * factor);
  }

  const ItemRegistry* items_;
  Inventory inventory_;
  Wallet wallet_;
  std::unordered_map<ItemId, ItemMarketConfig> configs_;
};

}  // namespace ytlib::testgame3
