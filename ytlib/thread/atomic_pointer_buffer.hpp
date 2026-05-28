/**
 * @file atomic_pointer_buffer.hpp
 * @brief 使用原子操作实现的无锁指针缓存
 * @note 持有一个指针，线程安全地更新/取出，原指针在被替换时由本类负责释放
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

  /// 用 ptr 替换当前指针，原指针被本类 delete
  void Update(T* ptr) {
    T* cur_ptr = ptr_.exchange(ptr);
    if (cur_ptr != nullptr) delete cur_ptr;
  }

  /// 用 ptr 替换当前指针，原指针返回给调用方，由调用方负责释放
  T* TakeAndUpdate(T* ptr) {
    return ptr_.exchange(ptr);
  }

  /// 取出当前指针，缓存被置为 nullptr，由调用方负责释放
  T* Take() {
    return ptr_.exchange(nullptr);
  }

 private:
  std::atomic<T*> ptr_ = nullptr;
};

}  // namespace ytlib
