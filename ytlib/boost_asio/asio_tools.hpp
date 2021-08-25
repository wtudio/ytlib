/**
 * @file asio_tools.hpp
 * @brief asio工具类
 * @note asio执行工具类，封装了asio的一般使用模式
 * @author WT
 * @date 2021-08-05
 */
#pragma once

#include <functional>
#include <list>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "ytlib/misc/error.hpp"
#include "ytlib/misc/misc_macro.h"
#include "ytlib/thread/thread_id.hpp"

namespace ytlib {

/**
 * @brief asio执行工具
 * @note 使用时先调用RegisterSvrFunc注册子服务的启动、停止方法，
 * 然后调用Start方法异步启动，之后可以调用join方法，等待kill信号或其他异步程序里调用Stop方法结束整个服务。
 * 并不会调用asio的stop方法，只会调用注册的stop方法，等各个子服务自己停止。
 */
class AsioExecutor {
 public:
  /**
   * @brief 构造函数
   * @param[in] threads_num 服务要启动的线程数
   */
  explicit AsioExecutor(uint32_t threads_num) : threads_num_(threads_num),
                                                io_(threads_num),
                                                signals_(io_, SIGINT, SIGTERM) {}

  ///析构函数
  ~AsioExecutor() noexcept {
    try {
      Join();
    } catch (const std::exception& e) {
      DBG_PRINT("AsioExecutor destruct get exception, %s", e.what());
    }
  }

  AsioExecutor(const AsioExecutor&) = delete;             ///<no copy
  AsioExecutor& operator=(const AsioExecutor&) = delete;  ///<no copy

  /**
   * @brief 注册svr的start、stop方法
   * @note 越早注册的start func越早执行，越早注册的stop func越晚执行
   * @param[in] start_func 子服务启动方法，一般在其中起一个启动协程
   * @param[in] stop_func 子服务结束方法，需要保证可以重复调用
   */
  void RegisterSvrFunc(std::function<void()>&& start_func, std::function<void()>&& stop_func) {
    if (start_func) start_func_vec_.emplace_back(std::move(start_func));
    if (stop_func) stop_func_vec_.emplace_back(std::move(stop_func));
  }

  /**
   * @brief 开始运行
   * @note 异步，会调用注册的start方法并启动指定数量的线程
   */
  void Start() {
    RT_ASSERT(threads_num_ >= 1, "Threads_num_ must >= 1.");

    for (size_t ii = 0; ii < start_func_vec_.size(); ++ii) {
      start_func_vec_[ii]();
    }

    signals_.async_wait([&](auto, auto) { Stop(); });

    auto run_func = [this] {
      DBG_PRINT("AsioExecutor thread %llu start.", ytlib::GetThreadId());

      try {
        io_.run();
      } catch (const std::exception& e) {
        DBG_PRINT("AsioExecutor thread %llu get exception %s.", ytlib::GetThreadId(), e.what());
      }

      DBG_PRINT("AsioExecutor thread %llu exit.", ytlib::GetThreadId());
    };

    for (uint32_t ii = 0; ii < threads_num_; ++ii) {
      threads_.emplace(threads_.end(), run_func);
    }
  }

  /**
   * @brief join
   * @note 阻塞直到所有线程退出
   */
  void Join() {
    for (auto itr = threads_.begin(); itr != threads_.end();) {
      if (itr->joinable())
        itr->join();
      threads_.erase(itr++);
    }
  }

  /**
   * @brief 停止
   * @note 异步，会调用注册的stop方法
   */
  void Stop() {
    if (std::atomic_exchange(&stop_flag_, true)) return;

    // 并不需要调用io_.stop()。当io_上所有任务都运行完毕后，会自动停止
    for (size_t ii = stop_func_vec_.size() - 1; ii < stop_func_vec_.size(); --ii) {
      stop_func_vec_[ii]();
    }

    signals_.cancel();
    signals_.clear();
  }

  /**
   * @brief 获取io
   * @return io_context
   */
  boost::asio::io_context& IO() { return io_; }

 private:
  const uint32_t threads_num_;
  boost::asio::io_context io_;
  boost::asio::signal_set signals_;
  std::list<std::thread> threads_;
  std::vector<std::function<void()> > start_func_vec_;
  std::vector<std::function<void()> > stop_func_vec_;
  std::atomic_bool stop_flag_ = false;
};

}  // namespace ytlib
