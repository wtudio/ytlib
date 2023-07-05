/**
 * @file atomic_object_buffer.hpp
 * @brief 提供原子操作的模板类
 * @note 基于
 * @author WT
 * @date 2023-07-05
 */
#pragma once

#include <atomic>
#include <memory>

namespace ytlib {

template <class T>
class AtomicObjectBuffer {
 public:
  AtomicObjectBuffer() = default;
  ~AtomicObjectBuffer() { Update(nullptr); }

  AtomicObjectBuffer(const AtomicObjectBuffer&) = delete;             ///< no copy
  AtomicObjectBuffer& operator=(const AtomicObjectBuffer&) = delete;  ///< no copy

  void Update(T* ptr) {
    T* cur_ptr = std::atomic_exchange(&object_ptr_, ptr);
    if (cur_ptr != nullptr) delete cur_ptr;
  }

  T* TakeAndUpdate(T* ptr) {
    return std::atomic_exchange(&object_ptr_, ptr);
  }

  T* Take() {
    return std::atomic_exchange(&object_ptr_, nullptr);
  }

 private:
  std::atomic<T*> object_ptr_ = nullptr;
};

}  // namespace ytlib
