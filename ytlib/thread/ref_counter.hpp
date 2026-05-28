/**
 * @file ref_counter.hpp
 * @brief 引用计数指针
 * @note 简易引用计数智能指针，仅支持强引用
 * @author WT
 * @date 2021-05-06
 */
#pragma once

#include <atomic>

namespace ytlib {

template <class T>
class RefCounter {
 private:
  struct InnerCounter {
    InnerCounter(T* ptr) : n_(1), ptr_(ptr) {}
    ~InnerCounter() = default;

    InnerCounter(const InnerCounter&) = delete;
    InnerCounter& operator=(const InnerCounter&) = delete;

    std::atomic_uint32_t n_;
    T* ptr_;
  };

  InnerCounter* counter_ptr_;

  void AddRef() {
    counter_ptr_->n_.fetch_add(1, std::memory_order_relaxed);
  }

  void SubRef() {
    if (counter_ptr_->n_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
      delete counter_ptr_->ptr_;
      delete counter_ptr_;
    }
  }

 public:
  // 构造函数，计数器为1
  explicit RefCounter(T* ptr) : counter_ptr_(new InnerCounter(ptr)) {}

  // 拷贝构造函数
  RefCounter(const RefCounter& rhs) : counter_ptr_(rhs.counter_ptr_) {
    if (counter_ptr_ != nullptr) AddRef();
  }

  // 移动构造函数
  RefCounter(RefCounter&& rhs) noexcept : counter_ptr_(rhs.counter_ptr_) {
    rhs.counter_ptr_ = nullptr;
  }

  // 拷贝赋值函数
  RefCounter& operator=(const RefCounter& rhs) {
    if (this == &rhs) return *this;

    if (counter_ptr_ != nullptr) SubRef();

    counter_ptr_ = rhs.counter_ptr_;
    if (counter_ptr_ != nullptr) AddRef();

    return *this;
  }

  // 移动赋值函数
  RefCounter& operator=(RefCounter&& rhs) noexcept {
    if (this == &rhs) return *this;

    if (counter_ptr_ != nullptr) SubRef();

    counter_ptr_ = rhs.counter_ptr_;
    rhs.counter_ptr_ = nullptr;

    return *this;
  }

  // 当前引用数。仅作观察用途，并发下立刻可能过期
  uint32_t Counter() const {
    if (counter_ptr_ == nullptr) return 0;
    return counter_ptr_->n_.load(std::memory_order_relaxed);
  }

  // 获取裸指针。被 move 之后调用为 nullptr
  T* Get() const {
    return counter_ptr_ != nullptr ? counter_ptr_->ptr_ : nullptr;
  }

  // 析构函数，计数器减1，为0则析构Counter
  ~RefCounter() {
    if (counter_ptr_ != nullptr) SubRef();
  }
};

template <class T, class... Values>
static RefCounter<T> MakeRefCounter(Values&&... values) {
  return RefCounter<T>(new T(std::forward<Values>(values)...));
}

}  // namespace ytlib