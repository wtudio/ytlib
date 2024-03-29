#pragma once

#include <iostream>
#include <list>
#include <thread>

#include <unifex/execute.hpp>
#include <unifex/inline_scheduler.hpp>
#include <unifex/new_thread_context.hpp>
#include <unifex/scheduler_concepts.hpp>

#include "ytlib/misc/misc_macro.h"
#include "ytlib/thread/thread_id.hpp"

/*
一个异步scheduler，每次会新建一个线程去执行任务
*/
namespace test2 {

struct MyContext;

// OperationState
template <typename Receiver>
struct MyOperationState final {
  // 必选实现：从另一个Receiver的构造函数
  template <typename Receiver2>
    requires std::constructible_from<Receiver, Receiver2>
  explicit MyOperationState(MyContext* ctx, Receiver2&& r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : ctx_(ctx), receiver_((Receiver2 &&) r) {}

  // 必选实现：void start() noexcept
  void start() noexcept;

  Receiver receiver_;
  MyContext* ctx_;
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

  explicit MySchedulerTask(MyContext* ctx) noexcept
      : ctx_(ctx) {}

  // 可选实现：unifex::blocking
  friend constexpr unifex::blocking_kind tag_invoke(unifex::tag_t<unifex::blocking>, const MySchedulerTask&) noexcept {
    return unifex::blocking_kind::never;
  }

  // 必选实现：OperationState connect(Receiver&&)
  template <typename Receiver>
  MyOperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver&& receiver) {
    return MyOperationState<unifex::remove_cvref_t<Receiver>>{ctx_, (Receiver &&) receiver};
  }

  MyContext* ctx_;
};

// Scheduler
struct MyScheduler {
  explicit MyScheduler(MyContext* ctx) noexcept : ctx_(ctx) {}

  // 必选实现：Sender schedule()
  MySchedulerTask schedule() const noexcept {
    return MySchedulerTask{ctx_};
  }

  // 必选实现：==
  friend bool operator==(MyScheduler a, MyScheduler b) noexcept {
    return a.ctx_ == b.ctx_;
  }

  // 必选实现：!=
  friend bool operator!=(MyScheduler a, MyScheduler b) noexcept {
    return a.ctx_ != b.ctx_;
  }

  MyContext* ctx_;
};

// Context
struct MyContext {
  MyContext() {}

  ~MyContext() {
    for (auto& t : thread_list_) {
      if (t.joinable()) t.join();
    }
  }

  MyScheduler GetScheduler() noexcept {
    return MyScheduler{this};
  }

  std::list<std::thread> thread_list_;
};

template <typename Receiver>
inline void MyOperationState<Receiver>::start() noexcept {
  try {
    ctx_->thread_list_.emplace(ctx_->thread_list_.end(), [this] {
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
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
}

inline void Test2() {
  DBG_PRINT("[run in thread %llu]Test2 main.", ytlib::GetThreadId());

  // unifex::new_thread_context ctx;
  MyContext ctx;

  for (int i = 0; i < 5; ++i) {
    unifex::execute(ctx.GetScheduler(), [i]() {
      DBG_PRINT("[run in thread %llu]hello new thread execute().", ytlib::GetThreadId());
    });
  }
}

}  // namespace test2