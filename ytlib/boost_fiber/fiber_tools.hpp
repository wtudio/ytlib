#pragma once

#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <boost/fiber/all.hpp>

#include "ytlib/misc/misc_macro.h"
#include "ytlib/thread/thread_id.hpp"

namespace ytlib {

/**
 * @brief
 * @note todo：分开线程池，参考https://stackoverflow.com/questions/51051481/multiple-shared-work-pools-with-boostfiber
 */
class FiberExecutor {
 public:
  using Task = std::function<void()>;

  explicit FiberExecutor(uint32_t threads_num = std::max<uint32_t>(std::thread::hardware_concurrency(), 1))
      : threads_num_(threads_num) {
  }

  ~FiberExecutor() {
    try {
      Stop();
      Join();
    } catch (const std::exception& e) {
      DBG_PRINT("FiberExecutor destruct get exception, %s", e.what());
    }
  }

  FiberExecutor(const FiberExecutor&) = delete;             ///< no copy
  FiberExecutor& operator=(const FiberExecutor&) = delete;  ///< no copy

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

  void Post(Task&& task) {
    task_ch_.push(std::move(task));
  }

  void Post(const Task& task) {
    task_ch_.push(task);
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
      DBG_PRINT("FiberExecutor thread %llu start.", ytlib::GetThreadId());

      try {
        boost::fibers::use_scheduling_algorithm<boost::fibers::algo::work_stealing>(threads_num_);

        Task tsk;
        while (boost::fibers::channel_op_status::closed != task_ch_.pop(tsk)) {
          tsk();
        }

      } catch (const std::exception& e) {
        DBG_PRINT("FiberExecutor thread %llu get exception %s.", ytlib::GetThreadId(), e.what());
      }

      DBG_PRINT("FiberExecutor thread %llu exit.", ytlib::GetThreadId());
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

    for (size_t ii = stop_func_vec_.size() - 1; ii < stop_func_vec_.size(); --ii) {
      stop_func_vec_[ii]();
    }
    stop_func_vec_.clear();

    task_ch_.close();
  }

 private:
  const std::chrono::steady_clock::duration thread_check_duration = std::chrono::seconds(10);

  uint32_t threads_num_;
  std::atomic_bool start_flag_ = false;
  std::atomic_bool stop_flag_ = false;
  boost::fibers::buffered_channel<Task> task_ch_{1024};
  std::list<std::thread> threads_;
  std::vector<std::function<void()> > start_func_vec_;
  std::vector<std::function<void()> > stop_func_vec_;
};

}  // namespace ytlib
