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
    ++(counter_ptr_->n_);
  }

  void SubRef() {
    if (--(counter_ptr_->n_) == 0) {
      delete counter_ptr_->ptr_;
      delete counter_ptr_;
    }
  }

 public:
  // 构造函数，计数器为1
  explicit RefCounter(T* ptr) : counter_ptr_(new InnerCounter(ptr)) {}

  // 拷贝构造函数
  RefCounter(const RefCounter& rhs) : counter_ptr_(rhs.counter_ptr_) {
    AddRef();
  }

  // 移动构造函数
  RefCounter(RefCounter&& rhs) : counter_ptr_(rhs.counter_ptr_) {
    rhs.counter_ptr_ = nullptr;
  }

  // 拷贝赋值函数
  RefCounter& operator=(const RefCounter& rhs) {
    if (this == &rhs) return *this;

    SubRef();

    counter_ptr_ = rhs.counter_ptr_;
    AddRef();

    return *this;
  }

  // 移动赋值函数
  RefCounter& operator=(RefCounter&& rhs) {
    if (this == &rhs) return *this;

    SubRef();

    counter_ptr_ = rhs.counter_ptr_;
    rhs.counter_ptr_ = nullptr;

    return *this;
  }

  uint32_t Counter() const {
    return static_cast<uint32_t>(counter_ptr_->n_);
  }

  T* Get() const {
    return counter_ptr_->ptr_;
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