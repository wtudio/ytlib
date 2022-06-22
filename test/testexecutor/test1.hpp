#pragma once

#include <iostream>
#include <thread>

#include <unifex/execute.hpp>
#include <unifex/inline_scheduler.hpp>
#include <unifex/scheduler_concepts.hpp>
#include <unifex/single_thread_context.hpp>

#include "ytlib/misc/misc_macro.h"
#include "ytlib/thread/thread_id.hpp"

/*
一个inline scheduler，简单的在当前线程同步执行
*/
namespace test1 {

// OperationState
template <typename Receiver>
struct MyOperationState final {
  // 必选实现：从另一个Receiver的构造函数
  template <typename Receiver2>
  requires std::constructible_from<Receiver, Receiver2>
  explicit MyOperationState(Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : receiver_((Receiver2 &&) r) {}

  // 必选实现：void start() noexcept
  void start() noexcept {
    try {
      if constexpr (unifex::is_stop_never_possible_v<unifex::stop_token_type_t<Receiver&>>) {
        unifex::set_value((Receiver &&) receiver_);
      } else {
        if (unifex::get_stop_token(receiver_).stop_requested()) {
          unifex::set_done((Receiver &&) receiver_);
        } else {
          unifex::set_value((Receiver &&) receiver_);
        }
      }
    } catch (...) {
      unifex::set_error((Receiver &&) receiver_, std::current_exception());
    }
  }

  Receiver receiver_;
};

// Sender
struct MySchedulerTask {
  // 必选实现：value_types
  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<>>;

  // 必选实现：error_types
  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  // 必选实现：bool sends_done
  static constexpr bool sends_done = true;

  // 可选实现：unifex::blocking
  friend constexpr unifex::blocking_kind tag_invoke(unifex::tag_t<unifex::blocking>, const MySchedulerTask&) noexcept {
    return unifex::blocking_kind::always_inline;
  }

  // 必选实现：OperationState connect(Receiver&&)
  template <typename Receiver>
  MyOperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver&& receiver) {
    return MyOperationState<unifex::remove_cvref_t<Receiver>>{(Receiver &&) receiver};
  }
};

// Scheduler
struct MyScheduler {
  // 必选实现：Sender schedule()
  constexpr MySchedulerTask schedule() const noexcept {
    return MySchedulerTask{};
  }

  // 必选实现：==
  friend bool operator==(MyScheduler a, MyScheduler b) noexcept {
    return true;
  }

  // 必选实现：!=
  friend bool operator!=(MyScheduler a, MyScheduler b) noexcept {
    return false;
  }
};

void Test1() {
  DBG_PRINT("[run in thread %llu]Test1 main.", ytlib::GetThreadId());

  // unifex::inline_scheduler sche;
  MyScheduler sche;

  for (int i = 0; i < 5; ++i) {
    unifex::execute(sche, [i]() {
      DBG_PRINT("[run in thread %llu]hello inline execute().", ytlib::GetThreadId());
    });
  }
}
}  // namespace test1
