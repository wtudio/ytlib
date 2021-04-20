/**
 * @file ChannelBase.h
 * @brief 基本通道
 * @details 基于生产者消费者模型的线程安全的通道基类
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <ytlib/SupportTools/QueueBase.h>
#include <boost/thread.hpp>
#include <vector>

namespace ytlib {

/**
 * @brief 基本通道类
 * 单队列，异步添加，可以多线程处理数据的通道。
 * 从进入通道到取出通道数据会经过复制操作。因此建议始终传递share_ptr一类的指针。
 * 使用阻塞取出，无法暂停，一旦开启将一直取出数据进行处理，直到无数据可取时阻塞
 */
template <class T,
          class _Queue = QueueBase<T> >
class ChannelBase {
 public:
  /**
   * @brief 通道构造函数
   * @details 构造之后即初始化，创建处理线程。单处理线程的通道适用于顺序数据处理，多处理线程的通道适用于并行处理
   * @param f_ 处理内容的函数。参数推荐使用（智能）指针
   * @param thCount_ 消费者线程数
   * @param queueSize_ 阻塞队列容量
   */
  ChannelBase(std::function<void(T &)> f_, uint32_t thCount_ = 1, uint32_t queueSize_ = 1000) : m_processFun(f_),
                                                                                                m_bRunning(true),
                                                                                                m_queue(queueSize_) {
    for (uint32_t ii = 0; ii < thCount_; ++ii) {
      m_Threads.create_thread(std::bind(&ChannelBase::Run, this));
    }
  }
  virtual ~ChannelBase(void) {
    //结束线程，禁止添加
    Stop();
  }
  inline bool Add(const T &item_) {
    return m_queue.Enqueue(item_);
  }
  void Stop() {
    if (m_bRunning) {
      m_bRunning = false;
      m_queue.Stop();
      m_Threads.join_all();
    }
  }
  inline void Clear() { m_queue.Clear(); }
  inline double GetUsagePercentage() { return (double)(m_queue.Count()) / (double)(m_queue.GetMaxCount()); }

 private:
  ///线程运行函数
  void Run() {
    while (m_bRunning) {
      T item_;
      if (m_queue.BlockDequeue(item_))
        m_processFun(item_);  //处理数据，此处的处理不支持异步
    }
  }

 protected:
  std::atomic_bool m_bRunning;
  boost::thread_group m_Threads;
  _Queue m_queue;
  std::function<void(T &)> m_processFun;  ///<处理内容函数。参数推荐使用（智能）指针
};

}  // namespace ytlib