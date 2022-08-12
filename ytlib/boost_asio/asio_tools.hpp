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
#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "ytlib/misc/misc_macro.h"
#include "ytlib/thread/thread_id.hpp"

namespace ytlib {

/**
 * @brief asio执行工具
 * @note 使用时先调用RegisterSvrFunc注册子服务的启动、停止方法，
 * 然后调用Start方法异步启动，之后可以调用join方法，等待kill信号或其他异步程序里调用Stop方法结束整个服务。
 * 并不会调用asio的stop方法，只会调用注册的stop方法，等各个子服务自己停止。
 * signals_同时承担了work_guard的功能，保证没有显式Stop之前io不会退出。
 */
class AsioExecutor {
 public:
  explicit AsioExecutor(uint32_t threads_num = std::max<uint32_t>(std::thread::hardware_concurrency(), 1))
      : threads_num_(threads_num),
        io_ptr_(std::make_shared<boost::asio::io_context>(threads_num)),
        work_guard_(io_ptr_->get_executor()) {
  }

  ~AsioExecutor() {
    try {
      Stop();
      Join();
    } catch (const std::exception& e) {
      DBG_PRINT("AsioExecutor destruct get exception, %s", e.what());
    }
  }

  AsioExecutor(const AsioExecutor&) = delete;             ///< no copy
  AsioExecutor& operator=(const AsioExecutor&) = delete;  ///< no copy

  /**
   * @brief 注册svr的start、stop方法
   * @note 越早注册的start func越早执行，越早注册的stop func越晚执行
   * @param[in] start_func 子服务启动方法，一般在其中起一个启动协程
   * @param[in] stop_func 子服务结束方法，需要保证可以重复调用
   */
  void RegisterSvrFunc(std::function<void()>&& start_func, std::function<void()>&& stop_func) {
    if (start_flag_)
      throw std::runtime_error("RegisterSvrFunc should not be called after start.");

    if (start_func) start_func_vec_.emplace_back(std::move(start_func));
    if (stop_func) stop_func_vec_.emplace_back(std::move(stop_func));
  }

  /**
   * @brief 开始运行
   * @note 异步，会调用注册的start方法并启动指定数量的线程
   */
  void Start() {
    if (std::atomic_exchange(&start_flag_, true)) return;

    for (size_t ii = 0; ii < start_func_vec_.size(); ++ii) {
      start_func_vec_[ii]();
    }
    start_func_vec_.clear();

    auto run_func = [this] {
      DBG_PRINT("AsioExecutor thread %llu start.", ytlib::GetThreadId());

      try {
        io_ptr_->run();
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
    if (!start_flag_)
      throw std::runtime_error("Join should be called after start.");

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
    stop_func_vec_.clear();

    work_guard_.reset();
  }

  /**
   * @brief 接收停止信号
   *
   */
  void EnableStopSignal() {
    if (start_flag_)
      throw std::runtime_error("EnableStopSignal should not be called after start.");

    std::shared_ptr<boost::asio::signal_set> sig_ptr = std::make_shared<boost::asio::signal_set>(*io_ptr_, SIGINT, SIGTERM);

    start_func_vec_.emplace_back([this, sig_ptr] {
      sig_ptr->async_wait([this, sig_ptr](auto, auto) {
        DBG_PRINT("AsioExecutor get stop signal.");
        Stop();
      });
    });

    stop_func_vec_.emplace_back([sig_ptr] {
      sig_ptr->cancel();
      sig_ptr->clear();
    });
  }

  /**
   * @brief 获取io
   * @return io_context
   */
  const std::shared_ptr<boost::asio::io_context>& IO() { return io_ptr_; }

  /**
   * @brief 获取线程数
   *
   * @return const uint32_t
   */
  const uint32_t ThreadsNum() const { return threads_num_; }

 private:
  const uint32_t threads_num_;
  std::atomic_bool start_flag_ = false;
  std::atomic_bool stop_flag_ = false;
  std::shared_ptr<boost::asio::io_context> io_ptr_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard_;
  std::list<std::thread> threads_;
  std::vector<std::function<void()> > start_func_vec_;
  std::vector<std::function<void()> > stop_func_vec_;
};

}  // namespace ytlib
