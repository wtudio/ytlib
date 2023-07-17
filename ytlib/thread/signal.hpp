/**
 * @file signal.hpp
 * @brief 轻量级信号量
 * @note 轻量级信号量和相关操作
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace ytlib {
/**
 * @brief 轻量级信号量
 * @note 使用c++20中atomic的信号量功能
 */
class LightSignalAtomic {
 public:
  LightSignalAtomic() : flag_(false) {}
  ~LightSignalAtomic() = default;

  /// 唤醒所有wait
  void notify() {
    flag_ = true;
    flag_.notify_all();
  }

  /// 无限等待唤醒
  void wait() {
    flag_.wait(false);
  }

  /// 重置信号量
  void reset() {
    flag_ = false;
  }

 private:
  std::atomic_bool flag_;
};

/**
 * @brief 轻量级信号量
 * @note 线程安全的轻量级信号量
 */
class LightSignal {
 public:
  LightSignal() = default;
  ~LightSignal() = default;

  /// 唤醒所有wait
  void notify() {
    std::lock_guard<std::mutex> lck(mutex_);
    flag_ = true;
    cond_.notify_all();
  }

  /// 无限等待唤醒
  void wait() {
    std::unique_lock<std::mutex> lck(mutex_);
    if (flag_) return;
    cond_.wait(lck);
  }

  /// 带超时的等待唤醒
  bool wait_for(uint32_t timeout_ms) {
    std::unique_lock<std::mutex> lck(mutex_);
    if (flag_) return true;
    if (cond_.wait_for(lck, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout) return false;
    return true;
  }

  /// 重置信号量
  void reset() {
    std::lock_guard<std::mutex> lck(mutex_);
    flag_ = false;
  }

 private:
  std::mutex mutex_;
  std::condition_variable cond_;
  bool flag_ = false;
};
}  // namespace ytlib
