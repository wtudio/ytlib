/**
 * @file block_queue.hpp
 * @brief BlockQueue
 * @details 阻塞队列
 * @author WT
 * @email 905976782@qq.com
 * @date 2021-05-06
 */
#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace ytlib {
/**
 * @brief 线程安全的阻塞队列
 */
template <class T>
class BlockQueue {
 public:
  BlockQueue() {}
  virtual ~BlockQueue() {}

  ///添加元素
  void Enqueue(const T &item) {
    std::lock_guard<std::mutex> lck(mutex_);
    queue_.emplace(item);
    cond_.notify_one();
  }

  ///添加元素
  void Enqueue(T &&item) {
    std::lock_guard<std::mutex> lck(mutex_);
    queue_.emplace(std::move(item));
    cond_.notify_one();
  }

  ///取出元素，如果空了就一直阻塞到可以取出
  T Dequeue() {
    std::unique_lock<std::mutex> lck(mutex_);
    cond_.wait(lck, [&] { return !queue_.empty(); });
    T item(std::move(queue_.front()));
    queue_.pop();
    return item;
  }

 protected:
  std::mutex mutex_;              ///<同步锁
  std::condition_variable cond_;  ///<条件锁
  std::queue<T> queue_;           ///<队列
};
}  // namespace ytlib
