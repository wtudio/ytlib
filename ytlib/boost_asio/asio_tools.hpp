#pragma once

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/asio.hpp>

#include <functional>
#include <list>
#include <thread>
#include <vector>

#include "ytlib/misc/error.hpp"

namespace ytlib {

/**
 * @brief asio执行工具
 * 使用时先调用RegisterSvrFunc注册子服务的启动、停止方法
 * 然后调用Start方法异步启动
 * 之后可以调用join方法，等待kill信号或其他异步程序里调用Stop方法结束整个服务
 */
class AsioExecutor {
 public:
  explicit AsioExecutor(uint32_t threads_num) : threads_num_(threads_num),
                                                io_(threads_num),
                                                signals_(io_, SIGINT, SIGTERM) {}

  ~AsioExecutor() noexcept {
    try {
      Join();
    } catch (const std::exception& e) {
      std::cerr << "AsioExecutor destruct get exception:" << e.what() << '\n';
    }
  }

  // no copy
  AsioExecutor(const AsioExecutor&) = delete;
  AsioExecutor& operator=(const AsioExecutor&) = delete;

  /**
   * @brief 注册svr的start、stop function
   * @details 越早注册的start func越早执行，越早注册的stop func越晚执行
   * @param start_func 子服务启动方法，一般在其中起一个启动协程
   * @param stop_func 子服务结束方法，需要保证可以重复调用
   */
  void RegisterSvrFunc(std::function<void()>&& start_func, std::function<void()>&& stop_func) {
    if (start_func) start_func_vec_.emplace_back(std::move(start_func));
    if (stop_func) stop_func_vec_.emplace_back(std::move(stop_func));
  }

  // 开始，异步
  void Start() {
    RT_ASSERT(threads_num_ >= 1, "threads_num_ must >= 1");

    for (std::size_t ii = 0; ii < start_func_vec_.size(); ++ii) {
      start_func_vec_[ii]();
    }

    signals_.async_wait([&](auto, auto) { Stop(); });

    auto run_func = [this] {
      std::cerr << "AsioExecutor thread " << std::this_thread::get_id() << " start.\n";
      io_.run();
      std::cerr << "AsioExecutor thread " << std::this_thread::get_id() << " exit.\n";
    };

    for (uint32_t ii = 0; ii < threads_num_; ++ii) {
      threads_.emplace(threads_.end(), run_func);
    }
  }

  // join，阻塞直到所有线程退出
  void Join() {
    for (auto itr = threads_.begin(); itr != threads_.end();) {
      itr->join();
      threads_.erase(itr++);
    }
  }

  // 停止，异步
  void Stop() {
    // 并不需要调用io_.stop()。当io_上所有任务都运行完毕后，会自动停止
    for (std::size_t ii = stop_func_vec_.size() - 1; ii < stop_func_vec_.size(); --ii) {
      stop_func_vec_[ii]();
    }
  }

  // 获取io
  boost::asio::io_context& IO() { return io_; }

 private:
  const uint32_t threads_num_;
  boost::asio::io_context io_;
  boost::asio::signal_set signals_;
  std::list<std::thread> threads_;
  std::vector<std::function<void()> > start_func_vec_;
  std::vector<std::function<void()> > stop_func_vec_;
};

}  // namespace ytlib
