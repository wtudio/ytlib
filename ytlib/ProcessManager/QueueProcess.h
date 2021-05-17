/**
 * @file QueueProcess.h
 * @brief 队列过程
 * @details 定义了基本的多线程场景下的队列过程
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <ytlib/ProcessManager/ProcessBase.h>
#include <ytlib/SupportTools/QueueBase.h>

#include <boost/thread.hpp>

namespace ytlib {
/**
 * @brief 流水线类Process
 * 暂停和停止只是暂停添加数据，处理数据的线程不受影响。等到队列中的数据处理完了自然就暂停下来了
 * 表层非线程安全，不要并发操作
 * 与通道类很像。区别就是此处的处理函数是子类重载的，此处的添加操作有暂停功能。
 * 但是还是感觉通道比较好用。这个类很有可能没有什么用
 */
template <class T,
          class _Queue = QueueBase<T> >
class QueueProcess : public ProcessBase {
 public:
  QueueProcess(std::size_t thCount_ = 1, std::size_t queueSize_ = 1000) : ProcessBase(),
                                                                          m_bStopFlag(false),
                                                                          m_threadCount(thCount_),
                                                                          m_queue(queueSize_) {
  }
  virtual ~QueueProcess() {
    stop();
  }
  virtual bool init() {
    if (m_Threads.size() > 0) return false;
    m_bStopFlag = false;
    for (std::size_t ii = 0; ii < m_threadCount; ii++) {
      m_Threads.create_thread(std::bind(&QueueProcess::Run, this));
    }
    return ProcessBase::init();
  }

  ///子类析构时一定要调用它
  virtual bool stop() {
    if (m_bStopFlag || !ProcessBase::stop()) return false;
    //stop之后需要重新初始化
    m_bInit = false;
    m_bStopFlag = true;
    m_Threads.join_all();  //等待所有线程结束
    return true;
  }

  ///start之后才能add
  virtual bool Add(const T& item_) {
    return m_bRunning && m_queue.Enqueue(item_);
  }

 private:
  ///线程运行函数
  void Run() {
    while (!m_bStopFlag) {
      T item_;
      if (m_queue.BlockDequeue(item_))
        ProcFun(item_);  //处理数据，此处的处理不支持异步
    }
  }

 protected:
  ///处理函数，子类重载
  virtual void ProcFun(const T&) = 0;

  std::atomic_bool m_bStopFlag;
  boost::thread_group m_Threads;
  _Queue m_queue;
  const std::size_t m_threadCount;
};
}  // namespace ytlib