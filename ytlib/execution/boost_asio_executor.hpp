#pragma once

#include <unifex/execute.hpp>

#include "ytlib/boost_asio/asio_tools.hpp"

namespace ytlib {

template <typename Receiver>
requires unifex::receiver<Receiver>
struct AsioOperationState {
  template <typename Receiver2>
  requires std::constructible_from<Receiver, Receiver2>
  explicit AsioOperationState(AsioExecutor* asio_executor_ptr, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : asio_executor_ptr_(asio_executor_ptr), receiver_((Receiver2 &&) r) {}

  void start() noexcept {
    try {
      boost::asio::dispatch(*(asio_executor_ptr_->IO()), [receiver_ = std::move(receiver_)]() mutable {
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
    } catch (...) {
      unifex::set_error(std::move(receiver_), std::current_exception());
    }
  }

  Receiver receiver_;
  AsioExecutor* asio_executor_ptr_;
};

struct AsioTask {
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<>>;

  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  static constexpr bool sends_done = true;

  explicit AsioTask(AsioExecutor* asio_executor_ptr) noexcept
      : asio_executor_ptr_(asio_executor_ptr) {}

  template <typename Receiver>
  AsioOperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver&& receiver) {
    return AsioOperationState<unifex::remove_cvref_t<Receiver>>(asio_executor_ptr_, (Receiver &&) receiver);
  }

  AsioExecutor* asio_executor_ptr_;
};

template <typename Receiver>
requires unifex::receiver<Receiver>
struct AsioSchedulerAfterOperationState {
  template <typename Receiver2>
  requires std::constructible_from<Receiver, Receiver2>
  explicit AsioSchedulerAfterOperationState(AsioExecutor* asio_executor_ptr, const std::chrono::steady_clock::duration& dt, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : asio_executor_ptr_(asio_executor_ptr), dt_(dt), receiver_((Receiver2 &&) r) {}

  void start() noexcept {
    try {
      std::shared_ptr<boost::asio::steady_timer> timer_ptr = std::make_shared<boost::asio::steady_timer>(*(asio_executor_ptr_->IO()), dt_);
      timer_ptr->async_wait([timer_ptr, receiver_ = std::move(receiver_)](const boost::system::error_code& ec) mutable {
        if (ec) {
          std::ostringstream buffer;
          buffer << "scheduler timer get error, " << ec;
          unifex::set_error(std::move(receiver_), std::make_exception_ptr(std::runtime_error(buffer.str())));
        }

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
    } catch (...) {
      unifex::set_error(std::move(receiver_), std::current_exception());
    }
  }

  Receiver receiver_;
  AsioExecutor* asio_executor_ptr_;
  std::chrono::steady_clock::duration dt_;
};

struct AsioSchedulerAfterTask {
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<>>;

  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  static constexpr bool sends_done = true;

  explicit AsioSchedulerAfterTask(AsioExecutor* asio_executor_ptr, const std::chrono::steady_clock::duration& dt) noexcept
      : asio_executor_ptr_(asio_executor_ptr), dt_(dt) {}

  template <typename Receiver>
  AsioSchedulerAfterOperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver&& receiver) {
    return AsioSchedulerAfterOperationState<unifex::remove_cvref_t<Receiver>>(asio_executor_ptr_, dt_, (Receiver &&) receiver);
  }

  AsioExecutor* asio_executor_ptr_;
  std::chrono::steady_clock::duration dt_;
};

template <typename Receiver>
requires unifex::receiver<Receiver>
struct AsioSchedulerAtOperationState {
  template <typename Receiver2>
  requires std::constructible_from<Receiver, Receiver2>
  explicit AsioSchedulerAtOperationState(AsioExecutor* asio_executor_ptr, const std::chrono::steady_clock::time_point& tp, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : asio_executor_ptr_(asio_executor_ptr), tp_(tp), receiver_((Receiver2 &&) r) {}

  void start() noexcept {
    try {
      std::shared_ptr<boost::asio::steady_timer> timer_ptr = std::make_shared<boost::asio::steady_timer>(*(asio_executor_ptr_->IO()), tp_);
      timer_ptr->async_wait([timer_ptr, receiver_ = std::move(receiver_)](const boost::system::error_code& ec) mutable {
        if (ec) {
          std::ostringstream buffer;
          buffer << "scheduler timer get error, " << ec;
          unifex::set_error(std::move(receiver_), std::make_exception_ptr(std::runtime_error(buffer.str())));
        }

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
    } catch (...) {
      unifex::set_error(std::move(receiver_), std::current_exception());
    }
  }

  Receiver receiver_;
  AsioExecutor* asio_executor_ptr_;
  std::chrono::steady_clock::time_point tp_;
};

struct AsioSchedulerAtTask {
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<>>;

  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  static constexpr bool sends_done = true;

  explicit AsioSchedulerAtTask(AsioExecutor* asio_executor_ptr, const std::chrono::steady_clock::time_point& tp) noexcept
      : asio_executor_ptr_(asio_executor_ptr), tp_(tp) {}

  template <typename Receiver>
  AsioSchedulerAtOperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver&& receiver) {
    return AsioSchedulerAtOperationState<unifex::remove_cvref_t<Receiver>>(asio_executor_ptr_, tp_, (Receiver &&) receiver);
  }

  AsioExecutor* asio_executor_ptr_;
  std::chrono::steady_clock::time_point tp_;
};

class AsioScheduler {
 public:
  explicit AsioScheduler(const std::shared_ptr<AsioExecutor>& asio_executor_ptr) noexcept
      : asio_executor_ptr_(asio_executor_ptr) {}

  AsioTask schedule() const noexcept {
    return AsioTask(asio_executor_ptr_.get());
  }

  AsioSchedulerAfterTask schedule_after(const std::chrono::steady_clock::duration& dt) const noexcept {
    return AsioSchedulerAfterTask(asio_executor_ptr_.get(), dt);
  }

  AsioSchedulerAtTask schedule_at(const std::chrono::steady_clock::time_point& tp) const noexcept {
    return AsioSchedulerAtTask(asio_executor_ptr_.get(), tp);
  }

  friend bool operator==(AsioScheduler a, AsioScheduler b) noexcept {
    return a.asio_executor_ptr_ == b.asio_executor_ptr_;
  }

  friend bool operator!=(AsioScheduler a, AsioScheduler b) noexcept {
    return a.asio_executor_ptr_ != b.asio_executor_ptr_;
  }

 private:
  std::shared_ptr<AsioExecutor> asio_executor_ptr_;
};

class AsioContext {
 public:
  explicit AsioContext(const std::shared_ptr<AsioExecutor>& asio_executor_ptr) noexcept
      : asio_executor_ptr_(asio_executor_ptr) {}

  ~AsioContext() noexcept {}

  AsioScheduler get_scheduler() const noexcept {
    return AsioScheduler(asio_executor_ptr_);
  }

  std::shared_ptr<AsioExecutor> Executor() const noexcept {
    return asio_executor_ptr_;
  }

 private:
  std::shared_ptr<AsioExecutor> asio_executor_ptr_;
};

}  // namespace ytlib
