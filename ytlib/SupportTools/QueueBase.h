/**
 * @file QueueBase.h
 * @brief 线程安全队列
 * @details 简单的多线程环境下线程安全的队列
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace ytlib {

/**
 * @brief 线程安的队列
 * 简单的多线程环境下线程安全的队列
 */
template <class T>
class QueueBase {
 public:
  explicit QueueBase(size_t n_) : m_maxcount(n_), stopflag(false) {}
  virtual ~QueueBase() { Stop(); }

  size_t GetMaxCount() { return m_maxcount; }
  size_t Count() {
    std::lock_guard<std::mutex> lck(m_mutex);
    return m_queue.size();
  }

  void Clear() {
    std::lock_guard<std::mutex> lck(m_mutex);
    while (!m_queue.empty())
      m_queue.pop();
  }

  void Stop() {
    stopflag = true;
    Clear();
    m_cond.notify_all();
  }

  ///添加元素。一般情况下只添加指针
  bool Enqueue(const T &item) {
    std::lock_guard<std::mutex> lck(m_mutex);
    if (m_queue.size() < m_maxcount) {
      m_queue.emplace(item);
      m_cond.notify_one();
      return true;
    }
    return false;
  }

  ///取出元素
  bool Dequeue(T &item) {
    std::lock_guard<std::mutex> lck(m_mutex);
    if (m_queue.empty()) return false;
    item = std::move(m_queue.front());
    m_queue.pop();
    return true;
  }

  ///阻塞式取出。如果空了就一直等待到可以取出。（没有做阻塞式添加，因为一般不可能用到）
  bool BlockDequeue(T &item) {
    std::unique_lock<std::mutex> lck(m_mutex);
    while (m_queue.empty()) {
      m_cond.wait(lck);
      if (stopflag) return false;
    }
    item = std::move(m_queue.front());
    m_queue.pop();
    return true;
  }

 protected:
  std::mutex m_mutex;              ///<同步锁。此处不能用读写锁，因为condition_variable还不支持
  std::condition_variable m_cond;  ///<条件锁
  std::queue<T> m_queue;           ///<队列

  const size_t m_maxcount;    ///<队列可支持最大个数
  std::atomic_bool stopflag;  ///<停止标志
};
}  // namespace ytlib