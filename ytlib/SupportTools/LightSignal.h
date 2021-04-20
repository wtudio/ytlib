/**
 * @file LightSignal.h
 * @brief 轻量级信号量
 * @details 轻量级信号量和相关操作
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <condition_variable>
#include <mutex>

namespace ytlib {
/**
 * @brief 轻量级信号量
 * 线程安全的轻量级信号量
 */
class LightSignal {
 public:
  LightSignal() : flag(false) {}
  ~LightSignal() {}

  ///唤醒所有wait
  void notify() {
    std::lock_guard<std::mutex> lck(m_mutex);
    flag = true;
    m_cond.notify_all();
  }
  ///无限等待唤醒
  void wait() {
    std::unique_lock<std::mutex> lck(m_mutex);
    if (flag) return;
    m_cond.wait(lck);
  }
  ///带超时的等待唤醒
  bool wait_for(uint32_t timeout_ms) {
    std::unique_lock<std::mutex> lck(m_mutex);
    if (flag) return true;
    if (m_cond.wait_for(lck, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout) return false;
    return true;
  }
  ///重置信号量
  void reset() {
    std::lock_guard<std::mutex> lck(m_mutex);
    flag = false;
  }

 private:
  std::mutex m_mutex;
  std::condition_variable m_cond;
  bool flag;
};
}  // namespace ytlib
