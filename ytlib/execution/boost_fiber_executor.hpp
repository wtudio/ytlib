#pragma once

#include <unifex/execute.hpp>

#include "ytlib/boost_fiber/fiber_tools.hpp"

namespace ytlib {

template <typename Receiver>
requires unifex::receiver<Receiver>
struct FiberOperationState {
  template <typename Receiver2>
  requires std::constructible_from<Receiver, Receiver2>
  explicit FiberOperationState(FiberExecutor* fiber_executor_ptr, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : fiber_executor_ptr_(fiber_executor_ptr), receiver_((Receiver2 &&) r) {}

  // 必选实现：void start() noexcept
  void start() noexcept {
    fiber_executor_ptr_->Post([receiver_ = std::move(receiver_)]() mutable {
      try {
        if (unifex::get_stop_token(receiver_).stop_requested()) {
          unifex::set_done(std::move(receiver_));
        } else {
          unifex::set_value(std::move(receiver_));
        }
      } catch (...) {
        unifex::set_error(std::move(receiver_), std::current_exception());
      }
    });
  }

  Receiver receiver_;
  FiberExecutor* fiber_executor_ptr_;
};

struct FiberTask {
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<>>;

  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  static constexpr bool sends_done = true;

  explicit FiberTask(FiberExecutor* fiber_executor_ptr) noexcept
      : fiber_executor_ptr_(fiber_executor_ptr) {}

  template <typename Receiver>
  FiberOperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver&& receiver) {
    return FiberOperationState<unifex::remove_cvref_t<Receiver>>(fiber_executor_ptr_, (Receiver &&) receiver);
  }

  FiberExecutor* fiber_executor_ptr_;
};

// Scheduler
class FiberScheduler {
 public:
  explicit FiberScheduler(const std::shared_ptr<FiberExecutor>& fiber_executor_ptr) noexcept
      : fiber_executor_ptr_(fiber_executor_ptr) {}

  FiberTask schedule() const noexcept {
    return FiberTask(fiber_executor_ptr_.get());
  }

  friend bool operator==(FiberScheduler a, FiberScheduler b) noexcept {
    return a.fiber_executor_ptr_ == b.fiber_executor_ptr_;
  }

  friend bool operator!=(FiberScheduler a, FiberScheduler b) noexcept {
    return a.fiber_executor_ptr_ != b.fiber_executor_ptr_;
  }

 private:
  std::shared_ptr<FiberExecutor> fiber_executor_ptr_;
};

// Context
class FiberContext {
 public:
  explicit FiberContext(const std::shared_ptr<FiberExecutor>& fiber_executor_ptr) noexcept
      : fiber_executor_ptr_(fiber_executor_ptr) {}

  ~FiberContext() noexcept {}

  FiberScheduler get_scheduler() const noexcept {
    return FiberScheduler(fiber_executor_ptr_);
  }

  std::shared_ptr<FiberExecutor> Executor() const noexcept {
    return fiber_executor_ptr_;
  }

 private:
  std::shared_ptr<FiberExecutor> fiber_executor_ptr_;
};

}  // namespace ytlib
