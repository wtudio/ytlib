#pragma once

#include <unifex/execute.hpp>

#include <boost/asio.hpp>

namespace ytlib {

template <typename Receiver, typename AsioExecutorType>
  requires unifex::receiver<Receiver>
struct AsioOperationState {
  template <typename Receiver2>
    requires std::constructible_from<Receiver, Receiver2>
  explicit AsioOperationState(AsioExecutorType* asio_executor_ptr, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : asio_executor_ptr_(asio_executor_ptr), receiver_((Receiver2 &&) r) {}

  void start() noexcept {
    try {
      boost::asio::dispatch(*asio_executor_ptr_, [receiver_ = std::move(receiver_)]() mutable {
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
  AsioExecutorType* asio_executor_ptr_;
};

template <typename AsioExecutorType>
struct AsioTask {
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<>>;

  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  static constexpr bool sends_done = true;

  explicit AsioTask(AsioExecutorType* asio_executor_ptr) noexcept
      : asio_executor_ptr_(asio_executor_ptr) {}

  template <typename Receiver>
  AsioOperationState<unifex::remove_cvref_t<Receiver>, AsioExecutorType> connect(Receiver&& receiver) {
    return AsioOperationState<unifex::remove_cvref_t<Receiver>, AsioExecutorType>(asio_executor_ptr_, (Receiver &&) receiver);
  }

  AsioExecutorType* asio_executor_ptr_;
};

template <typename Receiver, typename AsioExecutorType>
  requires unifex::receiver<Receiver>
struct AsioSchedulerAfterOperationState {
  template <typename Receiver2>
    requires std::constructible_from<Receiver, Receiver2>
  explicit AsioSchedulerAfterOperationState(AsioExecutorType* asio_executor_ptr, const std::chrono::steady_clock::duration& dt, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : asio_executor_ptr_(asio_executor_ptr), dt_(dt), receiver_((Receiver2 &&) r) {}

  void start() noexcept {
    try {
      std::shared_ptr<boost::asio::steady_timer> timer_ptr = std::make_shared<boost::asio::steady_timer>(*asio_executor_ptr_, dt_);
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
  AsioExecutorType* asio_executor_ptr_;
  std::chrono::steady_clock::duration dt_;
};

template <typename AsioExecutorType>
struct AsioSchedulerAfterTask {
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<>>;

  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  static constexpr bool sends_done = true;

  explicit AsioSchedulerAfterTask(AsioExecutorType* asio_executor_ptr, const std::chrono::steady_clock::duration& dt) noexcept
      : asio_executor_ptr_(asio_executor_ptr), dt_(dt) {}

  template <typename Receiver>
  AsioSchedulerAfterOperationState<unifex::remove_cvref_t<Receiver>, AsioExecutorType> connect(Receiver&& receiver) {
    return AsioSchedulerAfterOperationState<unifex::remove_cvref_t<Receiver>, AsioExecutorType>(asio_executor_ptr_, dt_, (Receiver &&) receiver);
  }

  AsioExecutorType* asio_executor_ptr_;
  std::chrono::steady_clock::duration dt_;
};

template <typename Receiver, typename AsioExecutorType>
  requires unifex::receiver<Receiver>
struct AsioSchedulerAtOperationState {
  template <typename Receiver2>
    requires std::constructible_from<Receiver, Receiver2>
  explicit AsioSchedulerAtOperationState(AsioExecutorType* asio_executor_ptr, const std::chrono::steady_clock::time_point& tp, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : asio_executor_ptr_(asio_executor_ptr), tp_(tp), receiver_((Receiver2 &&) r) {}

  void start() noexcept {
    try {
      std::shared_ptr<boost::asio::steady_timer> timer_ptr = std::make_shared<boost::asio::steady_timer>(*asio_executor_ptr_, tp_);
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
  AsioExecutorType* asio_executor_ptr_;
  std::chrono::steady_clock::time_point tp_;
};

template <typename AsioExecutorType>
struct AsioSchedulerAtTask {
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<>>;

  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  static constexpr bool sends_done = true;

  explicit AsioSchedulerAtTask(AsioExecutorType* asio_executor_ptr, const std::chrono::steady_clock::time_point& tp) noexcept
      : asio_executor_ptr_(asio_executor_ptr), tp_(tp) {}

  template <typename Receiver>
  AsioSchedulerAtOperationState<unifex::remove_cvref_t<Receiver>, AsioExecutorType> connect(Receiver&& receiver) {
    return AsioSchedulerAtOperationState<unifex::remove_cvref_t<Receiver>, AsioExecutorType>(asio_executor_ptr_, tp_, (Receiver &&) receiver);
  }

  AsioExecutorType* asio_executor_ptr_;
  std::chrono::steady_clock::time_point tp_;
};

template <typename AsioExecutorType>
class AsioScheduler {
 public:
  explicit AsioScheduler(const std::shared_ptr<AsioExecutorType>& asio_executor_ptr) noexcept
      : asio_executor_ptr_(asio_executor_ptr) {}

  AsioTask<AsioExecutorType> schedule() const noexcept {
    return AsioTask<AsioExecutorType>(asio_executor_ptr_.get());
  }

  AsioSchedulerAfterTask<AsioExecutorType> schedule_after(const std::chrono::steady_clock::duration& dt) const noexcept {
    return AsioSchedulerAfterTask<AsioExecutorType>(asio_executor_ptr_.get(), dt);
  }

  AsioSchedulerAtTask<AsioExecutorType> schedule_at(const std::chrono::steady_clock::time_point& tp) const noexcept {
    return AsioSchedulerAtTask<AsioExecutorType>(asio_executor_ptr_.get(), tp);
  }

  friend bool operator==(AsioScheduler a, AsioScheduler b) noexcept {
    return a.asio_executor_ptr_ == b.asio_executor_ptr_;
  }

  friend bool operator!=(AsioScheduler a, AsioScheduler b) noexcept {
    return a.asio_executor_ptr_ != b.asio_executor_ptr_;
  }

 private:
  std::shared_ptr<AsioExecutorType> asio_executor_ptr_;
};

class AsioContext {
 public:
  explicit AsioContext(const std::shared_ptr<boost::asio::io_context>& io_ptr) noexcept
      : io_ptr_(io_ptr) {}

  ~AsioContext() noexcept {}

  auto GetScheduler() const noexcept
      -> AsioScheduler<boost::asio::io_context> {
    return AsioScheduler<boost::asio::io_context>(io_ptr_);
  }

  auto GetStrandScheduler() const noexcept
      -> AsioScheduler<boost::asio::strand<boost::asio::io_context::executor_type>> {
    using AsioExecutorType = boost::asio::strand<boost::asio::io_context::executor_type>;
    return AsioScheduler<AsioExecutorType>(std::make_shared<AsioExecutorType>(boost::asio::make_strand(*io_ptr_)));
  }

  const std::shared_ptr<boost::asio::io_context>& IO() const noexcept {
    return io_ptr_;
  }

 private:
  std::shared_ptr<boost::asio::io_context> io_ptr_;
};

}  // namespace ytlib
