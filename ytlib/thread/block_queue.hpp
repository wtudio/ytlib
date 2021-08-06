/**
 * @file block_queue.hpp
 * @brief 阻塞队列
 * @note 线程安全的阻塞队列
 * @author WT
 * @date 2021-05-06
 */
#pragma once

#include <climits>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>

namespace ytlib {
/**
 * @brief 线程安全的阻塞队列
 */
template <class T>
class BlockQueue {
 public:
  BlockQueue(std::size_t n = ULONG_MAX) : maxcount_(n) {}
  virtual ~BlockQueue() { Stop(); }

  ///获取最大容量
  std::size_t GetMaxCount() const { return maxcount_; }

  ///获取当前容量
  std::size_t Count() const {
    std::lock_guard<std::mutex> lck(mutex_);
    return queue_.size();
  }

  ///清除所有元素
  void Clear() {
    std::lock_guard<std::mutex> lck(mutex_);
    while (!queue_.empty())
      queue_.pop();
  }
  ///停止队列
  void Stop() {
    std::unique_lock<std::mutex> lck(mutex_);
    running_flag_ = false;
    cond_.notify_all();
  }

  ///添加元素
  bool Enqueue(const T &item) {
    std::lock_guard<std::mutex> lck(mutex_);
    if (queue_.size() < maxcount_) {
      queue_.emplace(item);
      cond_.notify_one();
      return true;
    }
    return false;
  }

  ///添加元素
  bool Enqueue(T &&item) {
    std::lock_guard<std::mutex> lck(mutex_);
    if (queue_.size() < maxcount_) {
      queue_.emplace(std::move(item));
      cond_.notify_one();
      return true;
    }
    return false;
  }

  ///非阻塞式取出元素
  bool Dequeue(T &item) {
    std::lock_guard<std::mutex> lck(mutex_);
    if (!queue_.empty()) {
      item = std::move(queue_.front());
      queue_.pop();
      return true;
    }
    return false;
  }

  ///阻塞式取出元素
  bool BlockDequeue(T &item) {
    std::unique_lock<std::mutex> lck(mutex_);
    if (queue_.empty()) {
      if (!running_flag_) return false;
      cond_.wait(lck);
      if (queue_.empty()) return false;
    }

    item = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  ///阻塞式取出元素
  bool BlockDequeue(std::function<void(T &&)> f) {
    std::unique_lock<std::mutex> lck(mutex_);
    if (queue_.empty()) {
      if (!running_flag_) return false;
      cond_.wait(lck);
      if (queue_.empty()) return false;
    }

    T item(std::move(queue_.front()));
    queue_.pop();
    lck.unlock();
    f(std::move(item));
    return true;
  }

 protected:
  const std::size_t maxcount_;  ///<队列可支持最大个数

  mutable std::mutex mutex_;      ///<同步锁
  std::condition_variable cond_;  ///<条件锁
  std::queue<T> queue_;           ///<队列
  bool running_flag_ = true;      ///<运行标志，为false时，当队列为空阻塞式取出将失败
};
}  // namespace ytlib
