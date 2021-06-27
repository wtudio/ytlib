#pragma once

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/asio.hpp>

#include <atomic>
#include <functional>
#include <list>
#include <thread>
#include <vector>

#include "ytlib/misc/error.hpp"

namespace ytlib {

class AsioExecutor {
 public:
  explicit AsioExecutor(uint32_t threads_num) : threads_num_(threads_num),
                                                io_(threads_num),
                                                signals_(io_, SIGINT, SIGTERM),
                                                stop_flag_(false) {}

  ~AsioExecutor() {
    Stop();
    stop_flag_.wait(false);
  }

  // no copy
  AsioExecutor(const AsioExecutor&) = delete;
  AsioExecutor& operator=(const AsioExecutor&) = delete;

  // 注册svr的start、stop function。越早注册的start func越早执行，越早注册的stop func越晚执行
  void RegisterSvrFunc(std::function<void()>&& start_func, std::function<void()>&& stop_func) {
    if (start_func) start_func_vec_.emplace_back(std::move(start_func));
    if (stop_func) stop_func_vec_.emplace_back(std::move(stop_func));
  }

  // 阻塞运行
  void Run() noexcept {
    try {
      RT_ASSERT(threads_num_ >= 1, "threads_num_ must >= 1");

      for (std::size_t ii = 0; ii < start_func_vec_.size(); ++ii) {
        start_func_vec_[ii]();
      }

      signals_.async_wait([&](auto, auto) { Stop(); });

      auto run_func = [auto& io = io_] {
        io.run();
        std::cerr << "AsioExecutor thread " << std::this_thread::get_id() << " exit.\n";
      };

      for (uint32_t ii = 0; ii < threads_num_ - 1; ++ii) {
        threads_.emplace(threads_.end(), run_func);
      }

      run_func();

      for (auto itr = threads_.begin(); itr != threads_.end();) {
        itr->join();
        threads_.erase(itr++);
      }

    } catch (const std::exception& e) {
      std::cerr << "AsioExecutor Run get exception:" << e.what() << '\n';
    }

    // 只有当run结束了，才能析构
    stop_flag_ = true;
    stop_flag_.notify_all();
  }

  // 停止，在其他的线程异步调用
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
  std::atomic_bool stop_flag_;
};

}  // namespace ytlib
