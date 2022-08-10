#pragma once

#include <list>

#include <unifex/async_mutex.hpp>
#include <unifex/execute.hpp>
#include <unifex/task.hpp>

namespace ytlib {

class DetachHolder {
 private:
  struct InnerCounter {
    InnerCounter() : n_(1) {}
    ~InnerCounter() {}

    InnerCounter(const InnerCounter &) = delete;             ///< no copy
    InnerCounter &operator=(const InnerCounter &) = delete;  ///< no copy

    std::atomic_uint32_t n_;
    std::function<void()> f_;
  };

  InnerCounter *counter_ptr_;

 public:
  DetachHolder() : counter_ptr_(new InnerCounter()) {}

  DetachHolder(const DetachHolder &rhs) : counter_ptr_(rhs.counter_ptr_) {
    ++(counter_ptr_->n_);
  }

  DetachHolder(DetachHolder &&rhs) : counter_ptr_(rhs.counter_ptr_) {
    rhs.counter_ptr_ = nullptr;
  }

  DetachHolder &operator=(const DetachHolder &rhs) = delete;
  DetachHolder &operator=(const DetachHolder &&rhs) = delete;

  ~DetachHolder() {
    if (counter_ptr_ != nullptr && --(counter_ptr_->n_) == 0) {
      counter_ptr_->f_();
      delete counter_ptr_;
    }
  }

  void SetDeferFun(std::function<void()> &&defer_fun) {
    counter_ptr_->f_ = std::move(defer_fun);
  }
};

struct DetachReceiver {
  explicit DetachReceiver(const DetachHolder &holder)
      : holder_ptr(new DetachHolder(holder)) {}

  template <typename... Values>
  void set_value(Values &&...values) noexcept {
    delete holder_ptr;
  }

  [[noreturn]] void set_error(std::exception_ptr) noexcept {
    delete holder_ptr;
    std::terminate();
  }

  void set_done() noexcept {
    delete holder_ptr;
  }

  DetachHolder *holder_ptr;
};

/**
 * @brief 分离式启动一个Sender
 * @note 会丢弃执行结果
 * @tparam Sender
 * @param sender
 */
template <typename Sender>
requires unifex::sender<Sender>
void StartDetached(Sender &&sender) {
  DetachHolder holder;
  DetachReceiver r(holder);

  using OpType = decltype(unifex::connect((Sender &&) sender, std::move(r)));
  OpType *op = new OpType(unifex::connect((Sender &&) sender, std::move(r)));
  unifex::start(*op);

  holder.SetDeferFun([op] {
    delete op;
  });
}

template <typename CallBack>
struct CallBackReceiver {
  CallBackReceiver(const DetachHolder &holder, CallBack &&cb)
      : holder_ptr(new DetachHolder(holder)),
        callback((CallBack &&) cb) {}

  template <typename... Values>
  void set_value(Values &&...values) noexcept {
    callback((Values &&) values...);
    delete holder_ptr;
  }

  [[noreturn]] void set_error(std::exception_ptr) noexcept {
    delete holder_ptr;
    std::terminate();
  }

  void set_done() noexcept {
    delete holder_ptr;
  }

  CallBack callback;
  DetachHolder *holder_ptr;
};

template <typename Sender, typename CallBack>
requires unifex::sender<Sender>
void StartDetached(Sender &&sender, CallBack &&cb) {
  DetachHolder holder;
  CallBackReceiver r(holder, (CallBack &&) cb);

  using OpType = decltype(unifex::connect((Sender &&) sender, std::move(r)));
  OpType *op = new OpType(unifex::connect((Sender &&) sender, std::move(r)));
  unifex::start(*op);

  holder.SetDeferFun([op] {
    delete op;
  });
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

  explicit AsyncWrapper(AsyncFunc &&async_func)
      : async_func_(std::move(async_func)) {}

  template <typename Receiver>
  AsyncWrapperOperationState<unifex::remove_cvref_t<Receiver>, Results...> connect(Receiver &&receiver) {
    return AsyncWrapperOperationState<unifex::remove_cvref_t<Receiver>, Results...>((AsyncFunc &&) async_func_, (Receiver &&) receiver);
  }

 private:
  AsyncFunc async_func_;
};

/**
 * @brief 异步信号量
 * @note 仅能使用一次，Notify之后所有的wait都将立即返回
 */
class AsyncSignal {
 public:
  template <typename Receiver>
  requires unifex::receiver<Receiver>
  struct OperationState {
    template <typename Receiver2>
    requires std::constructible_from<Receiver, Receiver2>
    OperationState(AsyncSignal *sig_ptr, Receiver2 &&r)
    noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
        : sig_ptr_(sig_ptr), receiver_(new Receiver((Receiver2 &&) r)) {}

    void start() noexcept {
      StartDetached(unifex::co_invoke([sig_ptr = sig_ptr_, receiver = receiver_]() -> unifex::task<void> {
        co_await sig_ptr->receivers_list_mutex_.async_lock();
        if (sig_ptr->flag_) {
          sig_ptr->receivers_list_.emplace_back([receiver]() {
            try {
              unifex::set_value(std::move(*receiver));
            } catch (...) {
              unifex::set_error(std::move(*receiver), std::current_exception());
            }
          });
        } else {
          try {
            unifex::set_value(std::move(*receiver));
          } catch (...) {
            unifex::set_error(std::move(*receiver), std::current_exception());
          }
        }
        sig_ptr->receivers_list_mutex_.unlock();
      }));
    }

    AsyncSignal *sig_ptr_;

    std::shared_ptr<Receiver> receiver_;
  };

  class Sender {
   public:
    template <template <typename...> class Variant, template <typename...> class Tuple>
    using value_types = Variant<Tuple<>>;

    template <template <typename...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done = false;

    Sender(AsyncSignal *sig_ptr)
        : sig_ptr_(sig_ptr) {}

    template <typename Receiver>
    OperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver &&receiver) {
      return OperationState<unifex::remove_cvref_t<Receiver>>(sig_ptr_, (Receiver &&) receiver);
    }

   private:
    AsyncSignal *sig_ptr_;
  };

  void Notify() {
    StartDetached(unifex::co_invoke([this]() -> unifex::task<void> {
      co_await receivers_list_mutex_.async_lock();
      flag_ = false;
      receivers_list_mutex_.unlock();

      for (auto &f : receivers_list_) f();
      receivers_list_.clear();
    }));
  }

  AsyncSignal::Sender Wait() {
    return AsyncSignal::Sender(this);
  }

 private:
  unifex::async_mutex receivers_list_mutex_;
  bool flag_ = true;
  std::list<std::function<void()>> receivers_list_;
};

}  // namespace ytlib
