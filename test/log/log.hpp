/**
 * @file log.hpp
 * @brief 高性能日志工具
 * @note 高性能异步日志工具，支持ctx、自定义writer
 * @author WT
 * @date 2021-05-08
 */
#pragma once

#include <cinttypes>
#include <cstdarg>
#include <functional>
#include <map>
#include <memory>
#include <shared_mutex>
#include <stack>
#include <string>
#include <vector>

#include "loglevel_def.hpp"
#include "ytlib/thread/block_queue.hpp"
#include "ytlib/thread/channel.hpp"

namespace ytlib {

enum {
  MAX_BUF_SIZE = 1024,  // fmt的buf大小，不支持过大的日志
  MAX_LOG_SIZE = 1024,  // 日志缓冲条数，超过缓冲大小将被丢弃
};

// context
using Ctx = std::shared_ptr<std::map<std::string, std::string> >;

/**
 * @brief 日志数据
 * 实现自定义writer时需要
 */
struct LogData {
  LOG_LEVEL lvl = LOG_LEVEL::L_TRACE;  ///< 日志级别
  uint64_t thread_id = 0;              ///< 创建日志的线程的id
  uint64_t time = 0;                   ///< 微秒时间戳
  Ctx ctx;                             ///< context
  char msg[MAX_BUF_SIZE];              ///< fmt后的日志msg字符串
};

// 自定义writer接口
using LogWriter = std::function<void(const LogData&)>;

/**
 * @brief 高性能日志工具
 * 高性能异步日志工具，支持ctx、自定义writer
 * 使用时先AddWriter再Init
 */
class Log {
 public:
  static Log& Ins() {
    static Log instance;
    return instance;
  }

  virtual ~Log() {}

  void Init(LOG_LEVEL lvl = LOG_LEVEL::L_INFO) {
    lvl_ = lvl;

    log_buf_.resize(MAX_LOG_SIZE);
    for (uint32_t ii = 0; ii < MAX_LOG_SIZE; ++ii) {
      log_buf_queue_.Enqueue(&(log_buf_[ii]));
    }

    ctx_stack_.emplace();

    log_channel_.Init([&](LogData*&& p) {
      uint32_t writer_num = static_cast<uint32_t>(writers_.size());
      for (uint32_t ii = 0; ii < writer_num; ++ii)
        writers_[ii](*p);

      log_buf_queue_.Enqueue(p);
    });

    log_channel_.StartProcess();
  }

  void SetLevel(LOG_LEVEL lvl) { lvl_ = lvl; }
  LOG_LEVEL Level() const { return lvl_; }

  void Trace(LOG_LEVEL lvl, const char* fmt, ...) {
    if (lvl < lvl_) return;

    ctx_mutex_.lock_shared();
    Ctx ctx = ctx_stack_.top();
    ctx_mutex_.unlock_shared();

    va_list argp;
    va_start(argp, fmt);
    TraceHandle(lvl, ctx, fmt, argp);
    va_end(argp);
  }
  void Trace(LOG_LEVEL lvl, const Ctx& ctx, const char* fmt, ...) {
    if (lvl < lvl_) return;

    va_list argp;
    va_start(argp, fmt);
    TraceHandle(lvl, ctx, fmt, argp);
    va_end(argp);
  }

  void AddWriter(LogWriter&& func) {
    writers_.emplace_back(std::move(func));
  }

  void PushCtx(const Ctx& ctx) {
    std::unique_lock<std::shared_mutex> lck(ctx_mutex_);
    ctx_stack_.emplace(ctx);
  }
  void PopCtx() {
    std::unique_lock<std::shared_mutex> lck(ctx_mutex_);
    if (ctx_stack_.size() > 1)
      ctx_stack_.pop();
  }

 private:
  Log() {}

  uint64_t GetThreadId() {
    thread_local uint64_t thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());  // 缓存的线程id
    return thread_id;
  }

  void TraceHandle(LOG_LEVEL lvl, const Ctx& ctx, const char* fmt, va_list argp) {
    LogData* log_data;
    log_buf_queue_.Dequeue(log_data);
    vsnprintf(log_data->msg, MAX_BUF_SIZE, fmt, argp);

    log_data->lvl = lvl;
    log_data->thread_id = GetThreadId();
    log_data->time = 0;
    log_data->ctx = ctx;

    log_channel_.Enqueue(log_data);
  }

  LOG_LEVEL lvl_ = LOG_LEVEL::L_INFO;   // 日志打印级别
  std::shared_mutex ctx_mutex_;         // context锁
  std::stack<Ctx> ctx_stack_;           // context
  std::vector<LogWriter> writers_;      // writer
  std::vector<LogData> log_buf_;        // log缓冲
  BlockQueue<LogData*> log_buf_queue_;  // log缓冲管理队列
  Channel<LogData*> log_channel_;       // log线程
};
}  // namespace ytlib
