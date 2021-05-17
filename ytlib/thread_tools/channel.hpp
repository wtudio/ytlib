/**
 * @file channel.hpp
 * @brief Channel
 * @details 基于线程安全队列的channel
 * @author WT
 * @email 905976782@qq.com
 * @date 2021-05-06
 */
#pragma once

#include <cassert>
#include <list>
#include <thread>

#include "block_queue.hpp"

namespace ytlib {

/**
 * @brief 基于线程安全队列的channel
 * 保证处理完所有item才会退出
 */
template <class T>
class Channel : public BlockQueue<T> {
 public:
  Channel(std::size_t n = ULONG_MAX) : BlockQueue<T>(n) {}
  virtual ~Channel() { StopProcess(); }

  ///初始化
  void Init(std::function<void(T &&)> f, uint32_t th_size = 1) {
    f_ = f;
    th_size_ = th_size;
  }

  ///开启线程
  void StartProcess() {
    assert(f_);
    for (uint32_t ii = 0; ii < th_size_; ++ii) {
      threads_.emplace(threads_.end(), [&] {
        std::unique_lock<std::mutex> lck(BlockQueue<T>::mutex_);
        while (true) {
          while (!BlockQueue<T>::queue_.empty()) {
            T data(std::move(BlockQueue<T>::queue_.front()));
            BlockQueue<T>::queue_.pop();
            lck.unlock();
            f_(std::move(data));
            lck.lock();
          }
          if (!BlockQueue<T>::running_flag_) return;
          BlockQueue<T>::cond_.wait(lck);
        }
      });
    }
  }

  ///清空队列并停止线程
  void StopProcess() {
    BlockQueue<T>::Stop();
    for (auto itr = threads_.begin(); itr != threads_.end();) {
      itr->join();
      threads_.erase(itr++);
    }
  }

 protected:
  std::function<void(T &&)> f_;     ///<处理函数
  uint32_t th_size_ = 1;            ///<线程数
  std::list<std::thread> threads_;  ///<运行线程
};

}  // namespace ytlib
