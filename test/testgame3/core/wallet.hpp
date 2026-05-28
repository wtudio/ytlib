/**
 * @file wallet.hpp
 * @brief 钱包
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include "core/types.hpp"

namespace ytlib::testgame3 {

class Wallet {
 public:
  explicit Wallet(Money initial = 0) : balance_(initial) {}

  Money Balance() const { return balance_; }
  void Credit(Money amount) { balance_ += amount; }

  // 扣款；不足则失败。
  bool Debit(Money amount) {
    if (amount < 0) return false;
    if (balance_ < amount) return false;
    balance_ -= amount;
    return true;
  }

 private:
  Money balance_;
};

}  // namespace ytlib::testgame3
