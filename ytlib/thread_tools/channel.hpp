/**
 * @file channel.hpp
 * @brief Channel
 * @details 基于线程安全队列的channel
 * @author WT
 * @email 905976782@qq.com
 * @date 2021-05-06
 */
#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <queue>
#include <thread>

namespace ytlib {

/**
 * @brief 基于线程安全队列的channel
 * 保证处理完所有item才会退出
 */
template <class T>
class Channel {
 public:
  Channel() : running_flag_(false) {}
  virtual ~Channel() { Stop(); }

  ///初始化
  void Init(std::function<void(T &&)> f, uint32_t th_size = 1) {
    f_ = f;
    th_size_ = th_size;
  }

  ///开启线程
  void Start() {
    running_flag_ = true;
    for (uint32_t ii = 0; ii < th_size_; ++ii) {
      threads_.emplace(threads_.end(), [&] {
        std::unique_lock<std::mutex> lck(mutex_);
        while (running_flag_) {
          if (cond_.wait_for(lck, std::chrono::milliseconds(1000), [&] { return !queue_.empty(); })) {
            while (!queue_.empty()) {
              T data = std::move(queue_.front());
              queue_.pop();
              lck.unlock();
              f_(std::move(data));
              lck.lock();
            }
          }
        }
      });
    }
  }

  ///清空队列并停止线程
  void Stop() {
    running_flag_ = false;
    for (auto itr = threads_.begin(); itr != threads_.end();) {
      itr->join();
      threads_.erase(itr++);
    }
  }

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

 protected:
  std::mutex mutex_;                ///<队列同步锁
  std::condition_variable cond_;    ///<条件锁
  std::queue<T> queue_;             ///<队列
  std::function<void(T &&)> f_;     ///<处理函数
  uint32_t th_size_ = 1;            ///<线程数
  std::atomic_bool running_flag_;   ///<运行标志
  std::list<std::thread> threads_;  ///<运行线程
};

}  // namespace ytlib
