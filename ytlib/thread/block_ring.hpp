/**
 * @file block_ring.hpp
 * @brief BlockRing
 * @details 线程安全的阻塞环形队列
 * 分为三阶段：预写、写完、读
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
  BlockRing() {}
  virtual ~BlockRing() {}

 protected:
};
}  // namespace ytlib