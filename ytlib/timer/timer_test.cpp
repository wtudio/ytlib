#include <gtest/gtest.h>

#include "timer.hpp"
#include "ytlib/misc/misc_macro.h"

using namespace std::chrono_literals;

namespace ytlib {

TEST(Timer, base) {
  Timer timer;

  // 时间暂停
  Timer::Options op{
      .dt = 1ms,
      .init_ratio = 0.0,
      .wheel_size_array = {1000, 60, 60, 24}};

  timer.Initialize(op);
  timer.Start();

  std::this_thread::sleep_for(100ms);

  auto start_tp = timer.StartTimePoint();
  DBG_PRINT("start time: %lums", start_tp.time_since_epoch().count() / 1000 / 1000);

  for (uint32_t ii = 0; ii < 100; ++ii) {
    timer.ExecuteAt(
        start_tp + ii * 20ms,
        [&timer, ii, start_tp]() {
          auto real_du = std::chrono::steady_clock::now() - start_tp;
          auto virtual_du = timer.Now() - start_tp;

          DBG_PRINT("count %lu, real time: %lums, virtual time: %lums",
                    ii,
                    real_du.count() / 1000 / 1000,
                    virtual_du.count() / 1000 / 1000);

          if (ii == 10) {
            timer.SetTimeRatio(0.0);
          } else if (ii == 20) {
            timer.SetTimeRatio(0.1);
          } else if (ii == 30) {
            timer.SetTimeRatio(10.0);
          } else if (ii == 40) {
            timer.SetTimeRatio(1.0);
          } else if (ii == 99) {
            timer.SetTimeRatio(0.0);
          }
        });
  }

  auto virtual_du = timer.Now() - start_tp;
  EXPECT_EQ(virtual_du.count(), 0);

  std::this_thread::sleep_for(100ms);

  virtual_du = timer.Now() - start_tp;
  EXPECT_EQ(virtual_du.count(), 0);

  timer.SetTimeRatio(1.0);

  std::this_thread::sleep_for(3s);

  virtual_du = timer.Now() - start_tp;
  EXPECT_EQ(virtual_du.count(), (10 * 20ms + op.dt).count());

  timer.SetTimeRatio(1.0);

  std::this_thread::sleep_for(5s);

  virtual_du = timer.Now() - start_tp;
  EXPECT_EQ(virtual_du.count(), (99 * 20ms + op.dt).count());

  timer.Shutdown();
}

}  // namespace ytlib
