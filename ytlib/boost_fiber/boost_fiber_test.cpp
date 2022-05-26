#include <gtest/gtest.h>

#include "fiber_tools.hpp"

namespace ytlib {

using namespace std;

TEST(BOOST_FIBER_TEST, FiberExecutor) {
  auto test_sys_ptr = std::make_shared<FiberExecutor>(2);

  test_sys_ptr->Post([]() {
    for (int i = 0; i < 5; ++i) {
      DBG_PRINT("[run in thread %llu]helloworld task %d.", ytlib::GetThreadId(), i);

      boost::fibers::fiber([i]() {
        for (int j = 0; j < 3; ++j) {
          DBG_PRINT("[run in thread %llu]sleep task %d(%d) begin.", ytlib::GetThreadId(), j, i);

          boost::this_fiber::sleep_for(std::chrono::milliseconds(500));

          DBG_PRINT("[run in thread %llu]sleep task %d(%d) end.", ytlib::GetThreadId(), j, i);
        }
      }).detach();
    }
  });

  DBG_PRINT("test_sys_ptr Start");
  test_sys_ptr->Start();

  std::thread t([test_sys_ptr] {
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    DBG_PRINT("test_sys_ptr Stop");
    test_sys_ptr->Stop();
  });

  DBG_PRINT("test_sys_ptr Join");
  test_sys_ptr->Join();

  t.join();
}

}  // namespace ytlib
