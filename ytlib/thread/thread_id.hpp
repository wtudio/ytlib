#pragma once

#include <atomic>

namespace ytlib {
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
