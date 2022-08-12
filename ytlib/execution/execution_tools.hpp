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
 * todo：尝试使用atomic改成无锁的
 */
class AsyncSignal {
 public:
  struct Content {
    unifex::async_mutex mutex;
    bool flag = true;
    std::list<std::function<void()>> receivers_list;
  };

  template <typename Receiver>
  requires unifex::receiver<Receiver>
  struct OperationState {
    template <typename Receiver2>
    requires std::constructible_from<Receiver, Receiver2>
    OperationState(const std::shared_ptr<Content> &content_ptr, Receiver2 &&r)
    noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
        : content_ptr_(content_ptr), receiver_(new Receiver((Receiver2 &&) r)) {}

    void start() noexcept {
      StartDetached(unifex::co_invoke([content_ptr = content_ptr_, receiver = receiver_]() -> unifex::task<void> {
        co_await content_ptr->mutex.async_lock();
        if (content_ptr->flag) {
          content_ptr->receivers_list.emplace_back([receiver]() {
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
        content_ptr->mutex.unlock();
      }));
    }

    std::shared_ptr<Content> content_ptr_;
    std::shared_ptr<Receiver> receiver_;
  };

  class Sender {
   public:
    template <template <typename...> class Variant, template <typename...> class Tuple>
    using value_types = Variant<Tuple<>>;

    template <template <typename...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done = false;

    Sender(const std::shared_ptr<Content> &content_ptr)
        : content_ptr_(content_ptr) {}

    template <typename Receiver>
    OperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver &&receiver) {
      return OperationState<unifex::remove_cvref_t<Receiver>>(content_ptr_, (Receiver &&) receiver);
    }

   private:
    std::shared_ptr<Content> content_ptr_;
  };

  explicit AsyncSignal()
      : content_ptr_(std::make_shared<Content>()) {}

  void Notify() {
    StartDetached(unifex::co_invoke([content_ptr = content_ptr_]() -> unifex::task<void> {
      co_await content_ptr->mutex.async_lock();
      content_ptr->flag = false;
      content_ptr->mutex.unlock();

      for (auto &f : content_ptr->receivers_list) f();
      content_ptr->receivers_list.clear();
    }));
  }

  AsyncSignal::Sender Wait() {
    return AsyncSignal::Sender(content_ptr_);
  }

 private:
  std::shared_ptr<Content> content_ptr_;
};

/**
 * @brief 异步计数器
 * @note Count一定次数后触发Wait返回。仅能使用一次，触发之后所有的wait都将立即返回
 * todo：尝试使用atomic改成无锁的
 */
class AsyncCounter {
 public:
  struct Content {
    explicit Content(uint32_t num)
        : count_num(num) {}

    unifex::async_mutex mutex;
    uint32_t count_num;
    std::list<std::function<void()>> receivers_list;
  };

  template <typename Receiver>
  requires unifex::receiver<Receiver>
  struct OperationState {
    template <typename Receiver2>
    requires std::constructible_from<Receiver, Receiver2>
    OperationState(const std::shared_ptr<Content> &content_ptr, Receiver2 &&r)
    noexcept(std::is_nothrow_constructible_v<Receiver, Receiver2>)
        : content_ptr_(content_ptr), receiver_(new Receiver((Receiver2 &&) r)) {}

    void start() noexcept {
      StartDetached(unifex::co_invoke([content_ptr = content_ptr_, receiver = receiver_]() -> unifex::task<void> {
        co_await content_ptr->mutex.async_lock();
        if (content_ptr->count_num) {
          content_ptr->receivers_list.emplace_back([receiver]() {
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
        content_ptr->mutex.unlock();
      }));
    }

    std::shared_ptr<Content> content_ptr_;
    std::shared_ptr<Receiver> receiver_;
  };

  class Sender {
   public:
    template <template <typename...> class Variant, template <typename...> class Tuple>
    using value_types = Variant<Tuple<>>;

    template <template <typename...> class Variant>
    using error_types = Variant<std::exception_ptr>;

    static constexpr bool sends_done = false;

    Sender(const std::shared_ptr<Content> &content_ptr)
        : content_ptr_(content_ptr) {}

    template <typename Receiver>
    OperationState<unifex::remove_cvref_t<Receiver>> connect(Receiver &&receiver) {
      return OperationState<unifex::remove_cvref_t<Receiver>>(content_ptr_, (Receiver &&) receiver);
    }

   private:
    std::shared_ptr<Content> content_ptr_;
  };

  explicit AsyncCounter(uint32_t num)
      : content_ptr_(std::make_shared<Content>(num)) {
  }

  void Count() {
    StartDetached(unifex::co_invoke([content_ptr = content_ptr_]() -> unifex::task<void> {
      co_await content_ptr->mutex.async_lock();
      if (content_ptr->count_num > 0) --(content_ptr->count_num);
      bool notify_flag = (content_ptr->count_num == 0);
      content_ptr->mutex.unlock();

      if (notify_flag) {
        for (auto &f : content_ptr->receivers_list) f();
        content_ptr->receivers_list.clear();
      }
    }));
  }

  AsyncCounter::Sender Wait() {
    return AsyncCounter::Sender(content_ptr_);
  }

 private:
  std::shared_ptr<Content> content_ptr_;
};

}  // namespace ytlib
