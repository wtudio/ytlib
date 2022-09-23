#pragma once

#include <chrono>

#include <unifex/inplace_stop_token.hpp>
#include <unifex/tag_invoke.hpp>
#include <unifex/timed_single_thread_context.hpp>

namespace test5 {

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
  explicit DetachReceiver(const DetachHolder &holder, unifex::inplace_stop_token stto)
      : holder_ptr(new DetachHolder(holder)), stto_(stto) {}

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

  friend unifex::inplace_stop_token
  tag_invoke(unifex::tag_t<unifex::get_stop_token>, const DetachReceiver &r) noexcept {
    return r.stto_;
  }

  DetachHolder *holder_ptr;
  unifex::inplace_stop_token stto_;
};

/**
 * @brief 分离式启动一个Sender
 * @note 会丢弃执行结果
 * @tparam Sender
 * @param sender
 */
template <typename Sender>
requires unifex::sender<Sender>
auto StartDetached(Sender &&sender) -> std::shared_ptr<unifex::inplace_stop_source> {
  std::shared_ptr<unifex::inplace_stop_source> stsr = std::make_shared<unifex::inplace_stop_source>();
  DetachHolder holder;
  DetachReceiver r(holder, stsr->get_token());

  using OpType = decltype(unifex::connect((Sender &&) sender, std::move(r)));
  OpType *op = new OpType(unifex::connect((Sender &&) sender, std::move(r)));
  unifex::start(*op);

  holder.SetDeferFun([op] {
    delete op;
  });

  return stsr;
}

inline void Test5() {
  unifex::timed_single_thread_context context;
  auto work = [&]() -> unifex::task<void> {
    DBG_PRINT("111");
    co_await unifex::schedule_after(context.get_scheduler(), std::chrono::milliseconds(1000));
    DBG_PRINT("222");
    co_await unifex::schedule_after(context.get_scheduler(), std::chrono::milliseconds(1000));
    DBG_PRINT("333");
    co_return;
  };

  auto stsr = StartDetached(work());

  DBG_PRINT("start sleep");
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  DBG_PRINT("end sleep");

  stsr->request_stop();

  DBG_PRINT("start sleep");
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  DBG_PRINT("end sleep");
}
}  // namespace test5