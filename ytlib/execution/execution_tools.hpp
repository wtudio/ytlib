#pragma once

#include <unifex/execute.hpp>

namespace ytlib {

class DetachHolder {
 private:
  struct InnerCounter {
    InnerCounter() : n_(1) {}
    ~InnerCounter() {}

    InnerCounter(const InnerCounter &) = delete;             ///< no copy
    InnerCounter &operator=(const InnerCounter &) = delete;  ///< no copy

    std::atomic_uint32_t n_;
    std::function<void()> f_;
  };

  InnerCounter *counter_ptr_;

 public:
  DetachHolder() : counter_ptr_(new InnerCounter()) {}

  DetachHolder(const DetachHolder &rhs) : counter_ptr_(rhs.counter_ptr_) {
    ++(counter_ptr_->n_);
  }

  DetachHolder(DetachHolder &&rhs) : counter_ptr_(rhs.counter_ptr_) {
    rhs.counter_ptr_ = nullptr;
  }

  DetachHolder &operator=(const DetachHolder &rhs) = delete;
  DetachHolder &operator=(const DetachHolder &&rhs) = delete;

  ~DetachHolder() {
    if (counter_ptr_ != nullptr && --(counter_ptr_->n_) == 0) {
      counter_ptr_->f_();
      delete counter_ptr_;
    }
  }

  void SetDeferFun(std::function<void()> &&defer_fun) {
    counter_ptr_->f_ = std::move(defer_fun);
  }
};

struct DetachReceiver {
  explicit DetachReceiver(const DetachHolder &holder)
      : holder_ptr(new DetachHolder(holder)) {}

  template <typename... Values>
  void set_value(Values &&...values) noexcept {
    delete holder_ptr;
  }

  [[noreturn]] void set_error(std::exception_ptr) noexcept {
    delete holder_ptr;
    std::terminate();
  }

  void set_done() noexcept {
    delete holder_ptr;
  }

  DetachHolder *holder_ptr;
};

template <typename Sender>
requires unifex::sender<Sender>
void StartDetached(Sender &&sender) {
  DetachHolder holder;
  DetachReceiver r(holder);

  using OpType = decltype(unifex::connect((Sender &&) sender, std::move(r)));
  OpType *op = new OpType(unifex::connect((Sender &&) sender, std::move(r)));
  unifex::start(*op);

  holder.SetDeferFun([op] {
    delete op;
  });
}

}  // namespace ytlib
