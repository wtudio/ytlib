#pragma once

#include <atomic>
#include <chrono>
#include <list>
#include <memory>
#include <vector>

#include <boost/asio.hpp>

#include "employee.hpp"
#include "group.hpp"
#include "ship.hpp"
#include "station.hpp"

namespace ytlib {

struct WorldCfg {
  WorldCfg() : min_frame_duration(std::chrono::milliseconds(10)) {}

  std::chrono::steady_clock::duration min_frame_duration;
};

class World : public std::enable_shared_from_this<World> {
 public:
  World(std::shared_ptr<boost::asio::io_context> io_ptr,
        const WorldCfg& cfg) : cfg_(cfg),
                               io_ptr_(io_ptr),
                               loop_strand_(boost::asio::make_strand(*io_ptr_)) {}
  virtual ~World() {}

  void Start() {
    auto self = shared_from_this();
    boost::asio::co_spawn(
        loop_strand_,
        [this, self]() -> boost::asio::awaitable<void> {
          boost::asio::steady_timer timer(loop_strand_);

          std::chrono::steady_clock::time_point last_frame_end_time_point;
          std::chrono::steady_clock::duration last_frame_duration;

          // first frame start
          game_start_time_point_ = std::chrono::steady_clock::now();

          ++frame_count_;

          timer.expires_after(cfg_.min_frame_duration);
          co_await timer.async_wait(boost::asio::use_awaitable);

          last_frame_end_time_point = std::chrono::steady_clock::now();
          last_frame_duration = last_frame_end_time_point - game_start_time_point_;
          // first frame end

          while (run_flag_) {
            // frame start
            LoopConce(static_cast<float>(last_frame_duration.count()) / 1e9);

            ++frame_count_;

            auto cur_time_point = std::chrono::steady_clock::now();
            last_frame_duration = cur_time_point - last_frame_end_time_point;

            if (last_frame_duration < cfg_.min_frame_duration) {
              timer.expires_after(cfg_.min_frame_duration - last_frame_duration);
              co_await timer.async_wait(boost::asio::use_awaitable);

              cur_time_point = std::chrono::steady_clock::now();
              last_frame_duration = cur_time_point - last_frame_end_time_point;
            }

            last_frame_end_time_point = cur_time_point;
            // frame end
          }

          LoopConce(static_cast<float>(last_frame_duration.count()) / 1e9);

          game_end_time_point_ = last_frame_end_time_point;

          co_return;
        },
        boost::asio::detached);
  }

  void Stop() {
    run_flag_ = false;
  }

  void LoopConce(float dt) {
    for (auto& ship_itr : ship_map_) {
      ship_itr.second->OnWorldLoopOnce(dt);
    }
  }

 public:
  std::atomic_bool run_flag_ = true;
  WorldCfg cfg_;  // 配置
  std::shared_ptr<boost::asio::io_context> io_ptr_;
  boost::asio::strand<boost::asio::io_context::executor_type> loop_strand_;

  std::chrono::steady_clock::time_point game_start_time_point_;
  std::chrono::steady_clock::time_point game_end_time_point_;
  uint64_t frame_count_ = 0;

  std::map<uint32_t, std::shared_ptr<Group> > group_map_;
  std::map<uint32_t, std::shared_ptr<Ship> > ship_map_;
  std::map<uint32_t, std::shared_ptr<Station> > station_map_;
  std::map<uint32_t, std::shared_ptr<Employee> > employee_map_;
};

}  // namespace ytlib
