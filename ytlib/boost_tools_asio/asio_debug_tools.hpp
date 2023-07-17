/**
 * @file asio_debug_tools.hpp
 * @brief asio调试工具
 * @note asio调试工具
 * @author WT
 * @date 2022-03-19
 */
#pragma once

#include <atomic>
#include <chrono>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace ytlib {

enum class AsioDebugState : uint32_t {
  UnKnown,
  Start,
  End,
};

/**
 * @brief 单例模式，全局统计工具
 *
 */
class AsioDebugTool {
 public:
  static AsioDebugTool& Ins() {
    static AsioDebugTool instance;
    return instance;
  }

  ~AsioDebugTool() = default;

  void AddLog(uint64_t co_id, AsioDebugState state, const std::string& co_name = "") {
    thread_local uint64_t thread_id = thread_count_++;

    if (thread_id >= max_thread_num_)
      throw std::logic_error("Thread num is greater than the maximum.");

    uint64_t t = (std::chrono::steady_clock::now() - start_time_point_).count();

    thread_logs_vec[thread_id].emplace_back(co_id, state, co_name, thread_id, t);
  }

  std::string GetStatisticalResult(bool simple_mode = true) {
    // 统计信息：有多少协程被创建、有多少协程已经结束，未结束的协程的信息列表
    std::map<uint64_t, std::set<AsioDebugLog> > result_map;

    for (uint32_t thread_id = 0; thread_id < thread_logs_vec.size(); ++thread_id) {
      for (const auto& log : thread_logs_vec[thread_id]) {
        auto finditr = result_map.find(log.co_id);
        if (finditr == result_map.end()) {
          result_map.emplace(log.co_id, std::set<AsioDebugLog>{log});
        } else {
          finditr->second.emplace(log);
        }
      }
    }

    std::list<uint64_t> unfinished_co;

    for (const auto& co_itr : result_map) {
      AsioDebugState state = AsioDebugState::UnKnown;
      for (const auto& log : co_itr.second) {
        if (state == AsioDebugState::UnKnown && log.state == AsioDebugState::Start) {
          state = AsioDebugState::Start;
        } else if (state == AsioDebugState::Start && log.state == AsioDebugState::End) {
          state = AsioDebugState::End;
        }
      }
      if (state != AsioDebugState::End) {
        unfinished_co.emplace_back(co_itr.first);
      }
    }

    std::stringstream ss;
    ss << "asio debug report:\n"
       << "used thread num: " << thread_count_ << "\n"
       << "total co num: " << result_map.size() << "\n";

    if (!simple_mode) {
      for (const auto& co_itr : result_map) {
        ss << "co id: " << co_itr.first << "\n";
        for (const auto& log : co_itr.second) {
          ss << "    "
             << log.co_id
             << "|" << log.thread_id
             << "|" << static_cast<uint32_t>(log.state)
             << "|" << log.t
             << "|" << log.co_name
             << "\n";
        }
      }
    }

    ss << "unfinished co num: " << unfinished_co.size() << "\n";
    for (auto co_id : unfinished_co) {
      ss << "  unfinished co id:" << co_id << "\n";
      auto finditr = result_map.find(co_id);
      for (const auto& log : finditr->second) {
        ss << "    "
           << log.co_id
           << "|" << log.thread_id
           << "|" << static_cast<uint32_t>(log.state)
           << "|" << log.t
           << "|" << log.co_name
           << "\n";
      }
    }

    ss << std::endl;
    return ss.str();
  }

  void Reset() {
    thread_count_ = 0;
    start_time_point_ = std::chrono::steady_clock::now();
    thread_logs_vec.clear();
    thread_logs_vec.resize(max_thread_num_);
  }

 private:
  struct AsioDebugLog {
    uint64_t co_id;
    AsioDebugState state;
    std::string co_name;
    uint64_t thread_id;
    uint64_t t;

    bool operator<(const AsioDebugLog& val) const { return t < val.t; }
    bool operator>(const AsioDebugLog& val) const { return t > val.t; }
    bool operator<=(const AsioDebugLog& val) const { return t <= val.t; }
    bool operator>=(const AsioDebugLog& val) const { return t >= val.t; }
    bool operator==(const AsioDebugLog& val) const { return t == val.t; }
    bool operator!=(const AsioDebugLog& val) const { return t != val.t; }
  };

  AsioDebugTool() : start_time_point_(std::chrono::steady_clock::now()),
                    thread_logs_vec(max_thread_num_) {}

  const uint32_t max_thread_num_ = 64;

  std::atomic<uint64_t> thread_count_ = 0;
  std::chrono::steady_clock::time_point start_time_point_;
  std::vector<std::list<AsioDebugLog> > thread_logs_vec;
};

/**
 * @brief 协程统计句柄
 *
 */
class AsioDebugHandle {
 public:
  AsioDebugHandle(const std::string& co_name) {
    static std::atomic<uint64_t> co_count = 0;
    co_id_ = co_count++;
    AsioDebugTool::Ins().AddLog(co_id_, AsioDebugState::Start, co_name);
  }
  ~AsioDebugHandle() {
    AsioDebugTool::Ins().AddLog(co_id_, AsioDebugState::End);
  }

 private:
  uint64_t co_id_;
};

}  // namespace ytlib

// ASIO_DEBUG_HANDLE，一般用于detach型协程开始处
#ifdef ASIO_ENABLE_DEBUG
  #define _YT_ASIO_DEBUG_STRING(x) #x
  #define YT_ASIO_DEBUG_STRING(x) _YT_ASIO_DEBUG_STRING(x)

  #define ASIO_DEBUG_HANDLE(co_name) \
    ytlib::AsioDebugHandle co_name##_debug_handle(#co_name "[" __FILE__ ":" YT_ASIO_DEBUG_STRING(__LINE__) "]");
#else
  #define ASIO_DEBUG_HANDLE(co_name)
#endif
