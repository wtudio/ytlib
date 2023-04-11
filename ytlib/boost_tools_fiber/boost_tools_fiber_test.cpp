#include <gtest/gtest.h>

#include "fiber_tools.hpp"

namespace ytlib {

using namespace std;

TEST(BOOST_TOOLS_FIBER_TEST, FiberExecutor) {
  auto test_sys_ptr = std::make_shared<FiberExecutor>(2);

  std::atomic_uint32_t ct = 0;

  test_sys_ptr->Post([&ct]() {
    for (int i = 0; i < 3; ++i) {
      DBG_PRINT("[run in thread %llu]helloworld task %d.", ytlib::GetThreadId(), i);

      boost::fibers::fiber([&ct, i]() {
        for (int j = 0; j < 3; ++j) {
          DBG_PRINT("[run in thread %llu]sleep task %d(%d) begin.", ytlib::GetThreadId(), j, i);

          boost::this_fiber::sleep_for(std::chrono::milliseconds(20));
          ++ct;

          DBG_PRINT("[run in thread %llu]sleep task %d(%d) end.", ytlib::GetThreadId(), j, i);
        }
      }).detach();
    }
  });

  DBG_PRINT("test_sys_ptr Start");
  test_sys_ptr->Start();

  std::thread t([test_sys_ptr] {
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    DBG_PRINT("test_sys_ptr Stop");
    test_sys_ptr->Stop();
  });

  DBG_PRINT("test_sys_ptr Join");
  test_sys_ptr->Join();

  t.join();

  EXPECT_EQ(static_cast<uint32_t>(ct), 9);
}

}  // namespace ytlib
