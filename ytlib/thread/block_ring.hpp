/**
 * @file block_ring.hpp
 * @brief 阻塞环形队列
 * @note 线程安全的阻塞环形队列
 * @author WT
 * @date 2021-05-06
 */
#pragma once

#include <cinttypes>

namespace ytlib {
/**
 * @brief 线程安全的阻塞环形队列
 */
template <class T, uint32_t BUF_SIZE>
class BlockRing {
 public:
  BlockRing() = default;
  virtual ~BlockRing() = default;

 protected:
};
}  // namespace ytlib