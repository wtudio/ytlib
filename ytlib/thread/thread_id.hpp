/**
 * @file thread_id.hpp
 * @brief thread_id
 * @note 获取线程id
 * @author WT
 * @date 2021-05-06
 */
#pragma once

#include <atomic>

namespace ytlib {

///获取线程id工具
class ThreadIdTool {
 public:
  static ThreadIdTool& Ins() {
    static ThreadIdTool instance;
    return instance;
  }

  const uint64_t& GetThisThreadId() {
    thread_local uint64_t cur_thread_id = ++thread_count_;
    return cur_thread_id;
  }

 private:
  ThreadIdTool() {}

  std::atomic<uint64_t> thread_count_ = 0;
};

inline const uint64_t& GetThreadId() {
  return ThreadIdTool::Ins().GetThisThreadId();
}

}  // namespace ytlib
