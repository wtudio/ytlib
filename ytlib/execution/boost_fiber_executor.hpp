#pragma once

#include <unifex/execute.hpp>
#include <unifex/manual_lifetime.hpp>

#include "ytlib/boost_fiber/fiber_tools.hpp"

namespace ytlib {

template <typename Receiver>
requires unifex::receiver<Receiver>
struct FiberOperationState {
  template <typename Receiver2>
  requires std::constructible_from<Receiver, Receiver2>
  explicit FiberOperationState(FiberExecutor* fiber_executor_ptr, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : fiber_executor_ptr_(fiber_executor_ptr), receiver_(new Receiver((Receiver2 &&) r)) {}

  void start() noexcept {
    try {
      fiber_executor_ptr_->Post([receiver = receiver_]() {
        try {
          if (unifex::get_stop_token(*receiver).stop_requested()) {
            unifex::set_done(std::move(*receiver));
          } else {
            unifex::set_value(std::move(*receiver));
          }
        } catch (...) {
          unifex::set_error(std::move(*receiver), std::current_exception());
        }
      });
    } catch (...) {
      unifex::set_error(std::move(*receiver_), std::current_exception());
    }
  }

  std::shared_ptr<Receiver> receiver_;
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

template <typename Receiver>
requires unifex::receiver<Receiver>
struct FiberSchedulerAfterOperationState {
  template <typename Receiver2>
  requires std::constructible_from<Receiver, Receiver2>
  explicit FiberSchedulerAfterOperationState(FiberExecutor* fiber_executor_ptr, const std::chrono::steady_clock::duration& dt, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : fiber_executor_ptr_(fiber_executor_ptr), dt_(dt), receiver_(new Receiver((Receiver2 &&) r)) {}

  void start() noexcept {
    try {
      fiber_executor_ptr_->Post([dt = dt_, receiver = receiver_]() {
        try {
          boost::this_fiber::sleep_for(dt);
          if (unifex::get_stop_token(*receiver).stop_requested()) {
            unifex::set_done(std::move(*receiver));
          } else {
            unifex::set_value(std::move(*receiver));
          }
        } catch (...) {
          unifex::set_error(std::move(*receiver), std::current_exception());
        }
      });
    } catch (...) {
      unifex::set_error(std::move(*receiver_), std::current_exception());
    }
  }

  std::shared_ptr<Receiver> receiver_;
  FiberExecutor* fiber_executor_ptr_;
  std::chrono::steady_clock::duration dt_;
};

struct FiberSchedulerAfterTask {
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<>>;

  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  static constexpr bool sends_done = true;

  explicit FiberSchedulerAfterTask(FiberExecutor* fiber_executor_ptr, const std::chrono::steady_clock::duration& dt) noexcept
      : fiber_executor_ptr_(fiber_executor_ptr), dt_(dt) {}

  template <typename Receiver>
  FiberSchedulerAfterOperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver&& receiver) {
    return FiberSchedulerAfterOperationState<unifex::remove_cvref_t<Receiver>>(fiber_executor_ptr_, dt_, (Receiver &&) receiver);
  }

  FiberExecutor* fiber_executor_ptr_;
  std::chrono::steady_clock::duration dt_;
};

template <typename Receiver>
requires unifex::receiver<Receiver>
struct FiberSchedulerAtOperationState {
  template <typename Receiver2>
  requires std::constructible_from<Receiver, Receiver2>
  explicit FiberSchedulerAtOperationState(FiberExecutor* fiber_executor_ptr, const std::chrono::steady_clock::time_point& tp, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : fiber_executor_ptr_(fiber_executor_ptr), tp_(tp), receiver_(new Receiver((Receiver2 &&) r)) {}

  void start() noexcept {
    try {
      fiber_executor_ptr_->Post([tp = tp_, receiver = receiver_]() {
        try {
          boost::this_fiber::sleep_until(tp);
          if (unifex::get_stop_token(*receiver).stop_requested()) {
            unifex::set_done(std::move(*receiver));
          } else {
            unifex::set_value(std::move(*receiver));
          }
        } catch (...) {
          unifex::set_error(std::move(*receiver), std::current_exception());
        }
      });
    } catch (...) {
      unifex::set_error(std::move(*receiver_), std::current_exception());
    }
  }

  std::shared_ptr<Receiver> receiver_;
  FiberExecutor* fiber_executor_ptr_;
  std::chrono::steady_clock::time_point tp_;
};

struct FiberSchedulerAtTask {
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<>>;

  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  static constexpr bool sends_done = true;

  explicit FiberSchedulerAtTask(FiberExecutor* fiber_executor_ptr, const std::chrono::steady_clock::time_point& tp) noexcept
      : fiber_executor_ptr_(fiber_executor_ptr), tp_(tp) {}

  template <typename Receiver>
  FiberSchedulerAtOperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver&& receiver) {
    return FiberSchedulerAtOperationState<unifex::remove_cvref_t<Receiver>>(fiber_executor_ptr_, tp_, (Receiver &&) receiver);
  }

  FiberExecutor* fiber_executor_ptr_;
  std::chrono::steady_clock::time_point tp_;
};

class FiberScheduler {
 public:
  explicit FiberScheduler(const std::shared_ptr<FiberExecutor>& fiber_executor_ptr) noexcept
      : fiber_executor_ptr_(fiber_executor_ptr) {}

  FiberTask schedule() const noexcept {
    return FiberTask(fiber_executor_ptr_.get());
  }

  FiberSchedulerAfterTask schedule_after(const std::chrono::steady_clock::duration& dt) const noexcept {
    return FiberSchedulerAfterTask(fiber_executor_ptr_.get(), dt);
  }

  FiberSchedulerAtTask schedule_at(const std::chrono::steady_clock::time_point& tp) const noexcept {
    return FiberSchedulerAtTask(fiber_executor_ptr_.get(), tp);
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

class FiberContext {
 public:
  explicit FiberContext(const std::shared_ptr<FiberExecutor>& fiber_executor_ptr) noexcept
      : fiber_executor_ptr_(fiber_executor_ptr) {}

  ~FiberContext() noexcept {}

  FiberScheduler GetScheduler() const noexcept {
    return FiberScheduler(fiber_executor_ptr_);
  }

  std::shared_ptr<FiberExecutor> Executor() const noexcept {
    return fiber_executor_ptr_;
  }

 private:
  std::shared_ptr<FiberExecutor> fiber_executor_ptr_;
};

template <typename Result>
struct FiberWaitPromise {
  FiberWaitPromise() {}

  ~FiberWaitPromise() {
    if (state_ == state::value) {
      unifex::deactivate_union_member(value_);
    } else if (state_ == state::error) {
      unifex::deactivate_union_member(exception_);
    }
  }
  union {
    unifex::manual_lifetime<Result> value_;
    unifex::manual_lifetime<std::exception_ptr> exception_;
  };

  enum class state {
    incomplete,
    done,
    value,
    error
  };
  state state_ = state::incomplete;
};

template <typename Result>
struct FiberWaitReceiver {
  FiberWaitPromise<Result>& promise_;
  boost::fibers::promise<void>& fiber_promise_;

  template <typename... Values>
  void set_value(Values&&... values) && noexcept {
    try {
      unifex::activate_union_member(promise_.value_, (Values &&) values...);
      promise_.state_ = FiberWaitPromise<Result>::state::value;
    } catch (...) {
      unifex::activate_union_member(promise_.exception_, std::current_exception());
      promise_.state_ = FiberWaitPromise<Result>::state::error;
    }

    fiber_promise_.set_value();
  }

  void set_error(std::exception_ptr err) && noexcept {
    unifex::activate_union_member(promise_.exception_, std::move(err));
    promise_.state_ = FiberWaitPromise<Result>::state::error;
    fiber_promise_.set_value();
  }

  void set_error(std::error_code ec) && noexcept {
    std::move(*this).set_error(make_exception_ptr(std::system_error{ec, "sync_wait"}));
  }

  template <typename Error>
  void set_error(Error&& e) && noexcept {
    std::move(*this).set_error(make_exception_ptr((Error &&) e));
  }

  void set_done() && noexcept {
    promise_.state_ = FiberWaitPromise<Result>::state::done;
    fiber_promise_.set_value();
  }
};

template <typename Sender>
requires unifex::sender<Sender>
auto FiberWait(Sender&& sender)
    -> std::optional<unifex::sender_single_value_result_t<unifex::remove_cvref_t<Sender>>> {
  using Result = unifex::sender_single_value_result_t<unifex::remove_cvref_t<Sender>>;

  FiberWaitPromise<Result> promise;
  boost::fibers::promise<void> fiber_promise;
  boost::fibers::future<void> fiber_future(fiber_promise.get_future());

  auto operation = unifex::connect((Sender &&) sender, FiberWaitReceiver<Result>{promise, fiber_promise});
  unifex::start(operation);

  fiber_future.wait();

  switch (promise.state_) {
    case FiberWaitPromise<Result>::state::done:
      return std::nullopt;
    case FiberWaitPromise<Result>::state::value:
      return std::move(promise.value_).get();
    case FiberWaitPromise<Result>::state::error:
      std::rethrow_exception(promise.exception_.get());
    default:
      std::terminate();
  }
}

}  // namespace ytlib
