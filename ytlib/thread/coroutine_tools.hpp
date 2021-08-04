/**
 * @file coroutine_tools.hpp
 * @brief coroutine_tools
 * @details 协程工具，实验性质
 * @author WT
 * @date 2021-05-06
 */
#pragma once

#include <atomic>
#include <coroutine>
#include <functional>

namespace ytlib {

// 上层是线程的协程调度器
template <class T>
class CoroSched {
 public:
  class promise_type {
    friend class CoroSched;

   public:
    CoroSched get_return_object() { return CoroSched(this); }
    std::suspend_never initial_suspend() { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void unhandled_exception() {}

    void return_value(T&& re) {
      p_coro_sched->ret_ = std::move(re);
      p_coro_sched->flag_ = static_cast<uint8_t>(CoroState::END);
      p_coro_sched->flag_.notify_all();
    }

    std::suspend_always yield_value(const T& re) {
      p_coro_sched->ret_ = re;
      p_coro_sched->flag_ = static_cast<uint8_t>(CoroState::YIELD);
      p_coro_sched->flag_.notify_all();
      return {};
    }

    std::suspend_always yield_value(T&& re) {
      p_coro_sched->ret_ = std::move(re);
      p_coro_sched->flag_ = static_cast<uint8_t>(CoroState::YIELD);
      p_coro_sched->flag_.notify_all();
      return {};
    }

   private:
    CoroSched* p_coro_sched;
  };

  // 等待协程下一次调用co_yield或co_return
  void Wait() {
    flag_.wait(static_cast<uint8_t>(CoroState::RUN));
  }

  // 等待协程下一次调用co_yield或co_return，并获取其返回的值
  T& Get() {
    flag_.wait(static_cast<uint8_t>(CoroState::RUN));
    return ret_;
  }

  // 继续运行co_yield挂起的协程
  void Resume() {
    if (flag_ == static_cast<uint8_t>(CoroState::YIELD)) {
      flag_ = static_cast<uint8_t>(CoroState::RUN);
      handle_.resume();
    }
  }

 private:
  enum class CoroState : uint8_t {
    RUN = 0,
    YIELD,
    END,
  };

  CoroSched(promise_type* p) : flag_(static_cast<uint8_t>(CoroState::RUN)),
                               handle_(std::coroutine_handle<promise_type>::from_promise(*p)) {
    p->p_coro_sched = this;
  }

  T ret_;
  std::atomic_uint8_t flag_;
  std::coroutine_handle<promise_type> handle_;
};

template <class T>
class Awaitable {
 public:
  using RetCallback = std::function<void(T&&)>;

  Awaitable(std::function<void(RetCallback)> anyscfun) : anyscfun_(anyscfun) {}

  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    anyscfun_([=](T&& re) {
      this->re_ = std::move(re);
      h.resume();
    });
  }
  T await_resume() noexcept { return std::move(re_); }

 private:
  std::function<void(RetCallback)> anyscfun_;
  T re_;
};

template <>
class Awaitable<void> {
 public:
  using RetCallback = std::function<void()>;

  Awaitable(std::function<void(RetCallback)> anyscfun) : anyscfun_(anyscfun) {}

  bool await_ready() const noexcept { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    anyscfun_([=]() {
      h.resume();
    });
  }
  void await_resume() noexcept {}

 private:
  std::function<void(RetCallback)> anyscfun_;
};

}  // namespace ytlib
