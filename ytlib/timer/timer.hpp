/**
 * @file time_wheel.hpp
 * @author WT
 * @brief 时间轮
 * @date 2023-12-02
 */
#pragma once

#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>

namespace ytlib {

// 先全部加锁，实现完成后再看哪些地方可以用atomic或无锁方式优化的

/**
 * @brief 定时器
 * @note 采用时间轮实现
 * TODO：解决空推进问题
 * TODO：采用无锁方式提高效率
 *
 */
class Timer {
 public:
  struct Options {
    /// 时间轮的最小分辨率
    std::chrono::steady_clock::duration dt = std::chrono::microseconds(1000);

    /// 时间速率
    double init_ratio = 1.0;

    /// 时间轮层级与尺寸
    std::vector<size_t> wheel_size_array = {1000, 60, 60};

    /// 校验配置
    static Options Verify(const Options& verify_options) {
      Options options(verify_options);

      if (options.wheel_size_array.empty())
        throw std::runtime_error("options.wheel_size_array is empty!");

      return options;
    }
  };

  enum class State : uint32_t {
    PreInit,
    Init,
    Start,
    Shutdown,
  };

  using Task = std::function<void()>;
  using Executor = std::function<void(Task&&)>;

 public:
  Timer() : task_executor_([](Task&& task) { task(); }) {}

  ~Timer() { Shutdown(); }

  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;

  template <typename... Args>
    requires std::constructible_from<Executor, Args...>
  void RegisterExecutor(Args&&... args) {
    task_executor_ = Executor(std::forward<Args>(args)...);
  }

  void Initialize(const Options& options) {
    if (std::atomic_exchange(&state_, State::Init) != State::PreInit)
      throw std::runtime_error("Timer can only be initialized once.");

    options_ = options;

    SetTimeRatio(options_.init_ratio);

    uint64_t cur_scale = 1;
    for (size_t ii = 0; ii < options_.wheel_size_array.size(); ++ii) {
      timing_wheel_vec_.emplace_back(TimingWheelTool{
          .current_pos = 0,
          .scale = (cur_scale *= options_.wheel_size_array[ii]),
          .wheel = std::vector<TaskList>(options_.wheel_size_array[ii]),
          .borrow_func = [ii, this]() {
            TaskList task_list;
            if (ii < options_.wheel_size_array.size() - 1) {
              ++(timing_wheel_vec_[ii + 1].current_pos);
              task_list = timing_wheel_vec_[ii + 1].Tick();
            } else {
              ++timing_task_map_pos_;
              auto itr = timing_task_map_.find(timing_task_map_pos_);
              if (itr != timing_task_map_.end()) {
                task_list = std::move(itr->second);
                timing_task_map_.erase(itr);
              }
            }

            while (!task_list.empty()) {
              auto itr = task_list.begin();
              auto& cur_list = timing_wheel_vec_[ii].wheel[itr->tick_count % timing_wheel_vec_[ii].scale];
              cur_list.splice(cur_list.begin(), task_list, itr);
            }
          }});
    }
  }

  void Start() {
    if (std::atomic_exchange(&state_, State::Start) != State::Init)
      throw std::runtime_error("Timer can only start when state is 'Init'.");

    timer_thread_ = std::make_unique<std::thread>(std::bind(&Timer::TimerLoop, this));
  }

  void Shutdown() {
    if (std::atomic_exchange(&state_, State::Shutdown) == State::Shutdown)
      return;

    if (timer_thread_->joinable()) timer_thread_->join();
  }

  std::chrono::steady_clock::time_point StartTimePoint() const {
    assert(state_.load() == State::Start);

    return std::chrono::steady_clock::time_point(std::chrono::nanoseconds(start_time_point_));
  }

  std::chrono::steady_clock::time_point Now() const {
    assert(state_.load() == State::Start);

    std::shared_lock<std::shared_mutex> lck(tick_mutex_);

    return std::chrono::steady_clock::time_point(
        std::chrono::nanoseconds(current_tick_count_ * options_.dt.count() + start_time_point_));
  }

  void ExecuteAt(std::chrono::steady_clock::time_point tp, Task&& task) {
    assert(state_.load() == State::Start);

    uint64_t virtual_tp =
        static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                tp.time_since_epoch())
                .count()) -
        start_time_point_;

    std::unique_lock<std::shared_mutex> lck(tick_mutex_);

    if (virtual_tp < current_tick_count_ * options_.dt.count()) {
      lck.unlock();
      task_executor_(std::move(task));
      return;
    }

    // 当前时间点 time_point_
    uint64_t temp_current_tick_count = current_tick_count_;
    uint64_t diff_tick_count = virtual_tp / options_.dt.count() - current_tick_count_;

    const size_t len = options_.wheel_size_array.size();
    for (size_t ii = 0; ii < len; ++ii) {
      if (diff_tick_count < options_.wheel_size_array[ii]) {
        auto pos = (diff_tick_count + temp_current_tick_count) % options_.wheel_size_array[ii];

        // TODO：基于时间将任务排序后插进去
        timing_wheel_vec_[ii].wheel[pos].emplace_back(
            TaskWithTimestamp{virtual_tp / options_.dt.count(), std::move(task)});
        return;
      } else {
        diff_tick_count /= options_.wheel_size_array[ii];
        temp_current_tick_count /= options_.wheel_size_array[ii];
      }
    }

    timing_task_map_[diff_tick_count + temp_current_tick_count].emplace_back(
        TaskWithTimestamp{virtual_tp / options_.dt.count(), std::move(task)});
  }

  void ExecuteAfter(std::chrono::steady_clock::duration dt, Task&& task) {
    assert(state_.load() == State::Start);

    ExecuteAt(Now() + dt, std::move(task));
  }

  void SetTimeRatio(double ratio) {
    assert(state_.load() == State::Init || state_.load() == State::Start);

    std::unique_lock<std::shared_mutex> lck(ratio_mutex_);

    // 大于1，快进
    if (ratio >= 1.0) {
      ratio_direction_ = true;
      real_ratio_ = static_cast<uint32_t>(ratio);
      return;
    }

    ratio_direction_ = false;

    // 大于0小于1，慢放
    if (ratio > 1e-15 && ratio < 1.0 &&
        (1.0 / ratio) < std::numeric_limits<uint32_t>::max()) {
      real_ratio_ = static_cast<uint32_t>(1.0 / ratio);
      return;
    }

    // 小于0等于0，暂停
    real_ratio_ = std::numeric_limits<uint32_t>::max();
  }

  double GetTimeRatio() const {
    assert(state_.load() == State::Init || state_.load() == State::Start);

    std::shared_lock<std::shared_mutex> lck(ratio_mutex_);

    if (ratio_direction_)
      return real_ratio_;

    if (real_ratio_ == std::numeric_limits<uint32_t>::max())
      return 0.0;

    return 1.0 / real_ratio_;
  }

 private:
  void TimerLoop() {
    auto last_loop_time_point = std::chrono::steady_clock::now();

    // 记录初始时间
    start_time_point_ = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            last_loop_time_point.time_since_epoch())
            .count());

    while (state_.load() != State::Shutdown) {
      // 获取时间比例。注意：调速只能在下一个tick生效
      ratio_mutex_.lock_shared();

      bool ratio_direction = ratio_direction_;
      uint32_t real_ratio = real_ratio_;

      ratio_mutex_.unlock_shared();

      // sleep一个tick
      if (real_ratio == std::numeric_limits<uint32_t>::max()) {
        // 暂停了
        std::this_thread::sleep_until(last_loop_time_point += std::chrono::seconds(1));
        continue;
      }

      auto real_dt = ratio_direction ? options_.dt : (options_.dt * real_ratio);
      do {
        // 最长sleep时间
        static constexpr auto max_sleep_dt = std::chrono::seconds(1);

        auto sleep_time = (real_dt > max_sleep_dt) ? max_sleep_dt : real_dt;
        real_dt -= sleep_time;

        // 一个小优化，防止real_dt太小
        if (options_.dt < max_sleep_dt && real_dt <= options_.dt) {
          sleep_time += real_dt;
          real_dt -= real_dt;
        }

        std::this_thread::sleep_until(last_loop_time_point += sleep_time);

      } while (state_.load() != State::Shutdown && real_dt.count());

      // 走时间轮

      // 要走的tick数
      uint64_t diff_tick_count = ratio_direction ? real_ratio : 1;

      tick_mutex_.lock();

      do {
        // 取出task
        TaskList task_list = timing_wheel_vec_[0].Tick();

        // 执行任务
        if (!task_list.empty()) {
          tick_mutex_.unlock();

          for (auto itr = task_list.begin(); itr != task_list.end(); ++itr) {
            task_executor_(std::move(itr->task));
          }

          tick_mutex_.lock();
        }

        // 更新time point
        ++current_tick_count_;

      } while (--diff_tick_count);

      tick_mutex_.unlock();
    }
  }

 private:
  struct TaskWithTimestamp {
    uint64_t tick_count;  // 距离start_time的时间tick
    Task task;
  };

  using TaskList = std::list<TaskWithTimestamp>;

  struct TimingWheelTool {
    uint64_t current_pos;
    uint64_t scale;
    std::vector<TaskList> wheel;
    std::function<void()> borrow_func;

    TaskList Tick() {
      TaskList task_list;
      task_list.swap(wheel[current_pos]);

      ++current_pos;
      if (current_pos == wheel.size()) [[unlikely]] {
        current_pos = 0;
        borrow_func();
      }

      return task_list;
    }
  };

 private:
  Options options_;
  std::atomic<State> state_ = State::PreInit;

  Executor task_executor_;

  mutable std::shared_mutex ratio_mutex_;
  bool ratio_direction_ = true;
  uint32_t real_ratio_ = 1;

  uint64_t start_time_point_ = 0;

  mutable std::shared_mutex tick_mutex_;
  uint64_t current_tick_count_ = 0;
  std::vector<TimingWheelTool> timing_wheel_vec_;
  uint64_t timing_task_map_pos_ = 0;
  std::map<uint64_t, TaskList> timing_task_map_;

  std::unique_ptr<std::thread> timer_thread_;
};

}  // namespace ytlib
