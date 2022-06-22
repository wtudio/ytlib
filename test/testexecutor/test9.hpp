#pragma once

#include <unifex/execute.hpp>

#include "ytlib/misc/misc_macro.h"
#include "ytlib/thread/thread_id.hpp"

namespace test9 {

struct MyReceiver {
  void set_value(int ret) noexcept {
    DBG_PRINT("[run in thread %llu]MyReceiver set_value, ret = %d.", ytlib::GetThreadId(), ret);
  }

  [[noreturn]] void set_error(std::exception_ptr) noexcept {
    DBG_PRINT("[run in thread %llu]MyReceiver set_error.", ytlib::GetThreadId());
    std::terminate();
  }

  void set_done() noexcept {
    DBG_PRINT("[run in thread %llu]MyReceiver set_done.", ytlib::GetThreadId());
  }
};

template <typename Receiver>
struct MyOperationState {
  // 必选实现：void start() noexcept
  void start() noexcept {
    try {
      // receiver_.set_value(42);
      unifex::set_value((Receiver &&) receiver_, 42);
    } catch (...) {
      unifex::set_error((Receiver &&) receiver_, std::current_exception());
    }
  }

  Receiver receiver_;
};

struct MySender {
  // 必选实现：value_types
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<int>>;

  // 必选实现：error_types
  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  // 必选实现：bool sends_done
  static constexpr bool sends_done = false;

  // 必选实现：OperationState connect(Receiver&&)
  template <typename Receiver>
  MyOperationState<Receiver> connect(Receiver &&receiver) {
    return MyOperationState<Receiver>{static_cast<Receiver &&>(receiver)};
  }
};

struct DetachReceiver {
  template <typename... Values>
  void set_value(Values &&...values) noexcept {}

  [[noreturn]] void set_error(std::exception_ptr) noexcept {
    std::terminate();
  }

  void set_done() noexcept {}
};

template <typename Sender>
requires unifex::sender<Sender>
void StartDetached(Sender &&sender) {
  auto op = unifex::connect((Sender &&) sender, DetachReceiver());
  unifex::start(op);
}

void Test9() {
  DBG_PRINT("[run in thread %llu]Test9 main.", ytlib::GetThreadId());

  {
    MySender my_sender;
    MyReceiver my_receiver;

    auto op = unifex::connect(my_sender, my_receiver);
    unifex::start(op);
  }

  {
    MyReceiver my_receiver;
    auto op = unifex::connect(unifex::just(42), my_receiver);
    unifex::start(op);
  }

  {
    auto work = [&]() -> unifex::task<int> {
      int ret = co_await unifex::just(42);
      co_return ret;
    };

    MyReceiver my_receiver;
    auto op = unifex::connect(work(), my_receiver);
    unifex::start(op);
  }

  {
    auto work = [&]() -> unifex::task<int> {
      int ret = co_await MySender();
      co_return ret;
    };

    MyReceiver my_receiver;
    auto op = unifex::connect(work(), my_receiver);
    unifex::start(op);
  }

  {
    auto work = [&]() -> unifex::task<int> {
      int ret = co_await MySender();
      co_return ret;
    };

    StartDetached(work());
  }
}

}  // namespace test9
