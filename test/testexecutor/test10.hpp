#pragma once

#include <unifex/execute.hpp>

#include "ytlib/misc/misc_macro.h"
#include "ytlib/thread/thread_id.hpp"

namespace test10 {

template <typename Receiver, typename ResultType>
struct MyOperationState {
  using CallBack = std::function<void(ResultType)>;
  using AsyncFunc = std::function<void(CallBack)>;

  // 必选实现：void start() noexcept
  void start() noexcept {
    try {
      async_func_([&](ResultType result) {
        unifex::set_value((Receiver &&) receiver_, result);
      });

    } catch (...) {
      unifex::set_error((Receiver &&) receiver_, std::current_exception());
    }
  }

  Receiver receiver_;
  AsyncFunc async_func_;
};

template <typename ResultType>
struct MySender {
  using CallBack = std::function<void(ResultType)>;
  using AsyncFunc = std::function<void(CallBack)>;

  AsyncFunc async_func_;

  // 必选实现：value_types
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<ResultType>>;

  // 必选实现：error_types
  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  // 必选实现：bool sends_done
  static constexpr bool sends_done = false;

  explicit MySender(AsyncFunc &&async_func)
      : async_func_(std::move(async_func)) {}

  // 必选实现：OperationState connect(Receiver&&)
  template <typename Receiver>
  MyOperationState<Receiver, ResultType> connect(Receiver &&receiver) {
    return {static_cast<Receiver &&>(receiver), static_cast<AsyncFunc &&>(async_func_)};
  }
};

// 模拟异步请求
void AsyncSendRecv(uint32_t in, const std::function<void(uint32_t)> &callback) {
  std::thread t([in, callback]() {
    DBG_PRINT("[run in thread %llu]AsyncSendRecv step-1.", ytlib::GetThreadId());
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    DBG_PRINT("[run in thread %llu]AsyncSendRecv step-2.", ytlib::GetThreadId());
    callback(in + 1);
    DBG_PRINT("[run in thread %llu]AsyncSendRecv step-3.", ytlib::GetThreadId());
  });
  t.detach();
}

void Test10() {
  DBG_PRINT("[run in thread %llu]Test10 main.", ytlib::GetThreadId());

  AsyncSendRecv(42, [](uint32_t out) {
    DBG_PRINT("[run in thread %llu]out = %u.", ytlib::GetThreadId(), out);
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  auto work = [&]() -> unifex::task<int> {
    DBG_PRINT("[run in thread %llu]work step-1.", ytlib::GetThreadId());
    int ret = co_await MySender<int>(std::bind(AsyncSendRecv, 42, std::placeholders::_1));
    DBG_PRINT("[run in thread %llu]work step-2.", ytlib::GetThreadId());
    co_return ret;
  };

  auto work_ret = unifex::sync_wait(work());
  DBG_PRINT("[run in thread %llu]work_ret = %u.", ytlib::GetThreadId(), *work_ret);
}

}  // namespace test10