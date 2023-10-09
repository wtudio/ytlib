/**
 * @file atomic_pointer_buffer.hpp
 * @brief 使用原子操作实现的无锁指针缓存
 * @note 基于
 * @author WT
 * @date 2023-07-05
 */
#pragma once

#include <atomic>

namespace ytlib {

template <class T>
class AtomicPointerBuffer {
 public:
  AtomicPointerBuffer() = default;
  ~AtomicPointerBuffer() { Update(nullptr); }

  AtomicPointerBuffer(const AtomicPointerBuffer&) = delete;
  AtomicPointerBuffer& operator=(const AtomicPointerBuffer&) = delete;

  void Update(T* ptr) {
    T* cur_ptr = std::atomic_exchange(&ptr_, ptr);
    if (cur_ptr != nullptr) delete cur_ptr;
  }

  T* TakeAndUpdate(T* ptr) {
    return std::atomic_exchange(&ptr_, ptr);
  }

  T* Take() {
    return std::atomic_exchange(&ptr_, nullptr);
  }

 private:
  std::atomic<T*> ptr_ = nullptr;
};

}  // namespace ytlib
