#pragma once

#include <unifex/execute.hpp>

#include "ytlib/boost_asio/asio_tools.hpp"

namespace ytlib {

template <typename Receiver>
requires unifex::receiver<Receiver>
struct AsioOperationState {
  // 必选实现：从另一个Receiver的构造函数
  template <typename Receiver2>
  requires std::constructible_from<Receiver, Receiver2>
  explicit AsioOperationState(AsioExecutor* asio_executor_ptr, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : asio_executor_ptr_(asio_executor_ptr), receiver_((Receiver2 &&) r) {}

  // 必选实现：void start() noexcept
  void start() noexcept {
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
  }

  Receiver receiver_;
  AsioExecutor* asio_executor_ptr_;
};

struct AsioTask {
  // 必选实现：value_types
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<>>;

  // 必选实现：error_types
  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  // 必选实现：bool sends_done
  static constexpr bool sends_done = true;

  explicit AsioTask(AsioExecutor* asio_executor_ptr) noexcept
      : asio_executor_ptr_(asio_executor_ptr) {}

  // 必选实现：OperationState connect(Receiver&&)
  template <typename Receiver>
  AsioOperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver&& receiver) {
    return AsioOperationState<unifex::remove_cvref_t<Receiver>>(asio_executor_ptr_, (Receiver &&) receiver);
  }

  AsioExecutor* asio_executor_ptr_;
};

// Scheduler
class AsioScheduler {
 public:
  explicit AsioScheduler(const std::shared_ptr<AsioExecutor>& asio_executor_ptr) noexcept
      : asio_executor_ptr_(asio_executor_ptr) {}

  // 必选实现：Sender schedule()
  AsioTask schedule() const noexcept {
    return AsioTask(asio_executor_ptr_.get());
  }

  // 必选实现：==
  friend bool operator==(AsioScheduler a, AsioScheduler b) noexcept {
    return a.asio_executor_ptr_ == b.asio_executor_ptr_;
  }

  // 必选实现：!=
  friend bool operator!=(AsioScheduler a, AsioScheduler b) noexcept {
    return a.asio_executor_ptr_ != b.asio_executor_ptr_;
  }

 private:
  std::shared_ptr<AsioExecutor> asio_executor_ptr_;
};

// Context
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
