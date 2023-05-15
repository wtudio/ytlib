/**
 * @file channel.hpp
 * @brief Channel
 * @note 基于线程安全队列的channel
 * @author WT
 * @date 2021-05-06
 */
#pragma once

#include <list>
#include <thread>

#include "ytlib/thread/block_queue.hpp"

namespace ytlib {

/**
 * @brief 基于线程安全队列的channel
 * @note 保证处理完所有item才会退出。
 * 单队列，异步添加，可以多线程处理数据的通道。
 * 从进入通道到取出通道数据会经过复制操作。因此建议始终传递share_ptr一类的指针。
 * 使用阻塞取出，无法暂停，一旦开启将一直取出数据进行处理，直到无数据可取时阻塞。
 */
template <class T>
class Channel : public BlockQueue<T> {
 public:
  /**
   * @brief 通道构造函数
   * @param n 阻塞队列容量
   */
  Channel(size_t n = ULONG_MAX) : BlockQueue<T>(n) {}
  virtual ~Channel() { StopProcess(); }

  /**
   * @brief 初始化
   * @param f 处理内容的函数。参数推荐使用（智能）指针
   * @param th_size 消费者线程数
   */
  void Init(const std::function<void(T &&)> &f, uint32_t th_size = 1) {
    f_ = f;
    th_size_ = th_size;
  }

  /// 开启线程
  void StartProcess() {
    if (!f_) throw std::logic_error("Invalid HandleFun.");

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

  /// 清空队列并停止线程
  void StopProcess() {
    BlockQueue<T>::Stop();
    for (auto itr = threads_.begin(); itr != threads_.end();) {
      if (itr->joinable())
        itr->join();
      threads_.erase(itr++);
    }
  }

 protected:
  std::function<void(T &&)> f_;     ///< 处理函数
  uint32_t th_size_ = 1;            ///< 线程数
  std::list<std::thread> threads_;  ///< 运行线程
};

}  // namespace ytlib
