#pragma once

#include <list>
#include <memory>

#include <unifex/async_manual_reset_event.hpp>
#include <unifex/async_scope.hpp>
#include <unifex/execute.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>

namespace ytlib {

/**
 * @brief 分离式启动一个Sender
 * @note 会丢弃执行结果
 * @tparam Sender
 * @param sender
 */
template <typename Sender>
requires unifex::sender<Sender>
void StartDetached(Sender &&sender) {
  struct AsyncScopeDeleter {
    void operator()(unifex::async_scope *p) {
      unifex::sync_wait(p->cleanup());
      delete p;
    }
  };
  static std::unique_ptr<unifex::async_scope, AsyncScopeDeleter> scope_ptr{new unifex::async_scope()};
  scope_ptr->spawn((Sender &&) sender);
}

/**
 * @brief 分离式启动一个Sender
 * @note 会将结果通过callback函数返回
 * @tparam Sender
 * @tparam CallBack
 * @param sender
 * @param cb
 */
template <typename Sender, typename CallBack>
requires unifex::sender<Sender>
void StartDetached(Sender &&sender, CallBack &&cb) {
  StartDetached(((Sender &&) sender) | unifex::then((CallBack &&) cb));
}

template <typename Receiver, typename... Results>
requires unifex::receiver<Receiver>
struct AsyncWrapperOperationState {
  using CallBack = std::function<void(Results &&...)>;
  using AsyncFunc = std::function<void(CallBack)>;

  template <typename Receiver2>
  requires std::constructible_from<Receiver, Receiver2>
  explicit AsyncWrapperOperationState(AsyncFunc &&async_func, Receiver2 &&r) noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
      : async_func_(std::move(async_func)), receiver_(new Receiver((Receiver2 &&) r)) {}

  void start() noexcept {
    try {
      async_func_([receiver = receiver_](Results &&...values) {
        try {
          if (unifex::get_stop_token(*receiver).stop_requested()) {
            unifex::set_done(std::move(*receiver));
          } else {
            unifex::set_value(std::move(*receiver), (Results &&) values...);
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
  AsyncFunc async_func_;
};

/**
 * @brief 将异步回调型函数封装成一个Sender
 * @note todo：解决引用类型等问题
 * @tparam Results 结果的类型，也就是回调函数的参数类型
 */
template <typename... Results>
class AsyncWrapper {
 public:
  using CallBack = std::function<void(Results &&...)>;
  using AsyncFunc = std::function<void(CallBack)>;

  template <template <typename...> class Variant, template <typename...> class Tuple>
  using value_types = Variant<Tuple<Results...>>;

  template <template <typename...> class Variant>
  using error_types = Variant<std::exception_ptr>;

  static constexpr bool sends_done = true;

  template <typename F>
  explicit AsyncWrapper(F &&f) : async_func_((F &&) f) {}

  template <typename Receiver>
  AsyncWrapperOperationState<unifex::remove_cvref_t<Receiver>, Results...> connect(Receiver &&receiver) {
    return AsyncWrapperOperationState<unifex::remove_cvref_t<Receiver>, Results...>((AsyncFunc &&) async_func_, (Receiver &&) receiver);
  }

 private:
  AsyncFunc async_func_;
};

/**
 * @brief 异步计数器
 * @note Count一定次数后触发Wait返回。仅能使用一次，触发之后所有的AsyncWait都将立即返回
 */
class AsyncLatch {
 public:
  explicit AsyncLatch(uint32_t num)
      : n_(num) {}

  void CountDown() {
    if (--n_ == 0) ev_.set();
  }

  void Set() {
    ev_.set();
  }

  bool Ready() const {
    return ev_.ready();
  }

  auto AsyncWait() {
    return ev_.async_wait();
  }

 private:
  std::atomic_uint32_t n_;
  unifex::async_manual_reset_event ev_;
};

}  // namespace ytlib
