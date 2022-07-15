#include <gtest/gtest.h>

#include "ytlib/execution/execution_tools.hpp"
#include "ytlib/execution/remote_cache.hpp"

#include <unifex/sync_wait.hpp>
#include <unifex/timed_single_thread_context.hpp>

namespace ytlib {

// 模拟异步请求
void AsyncUpdateDataFunc(uint32_t key, const std::function<void(std::optional<std::string>&&)>& callback) {
  std::thread t([key, callback]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (key == 42) {
      callback("the answer");
      return;
    }
    if (key > 50) {
      callback(std::nullopt);
      return;
    }
    callback(std::to_string(key));
    return;
  });
  t.detach();
}

TEST(EXECUTION_TEST, RemoteCache) {
  using TestRemoteCache = ytlib::RemoteCache<int, std::string>;

  TestRemoteCache::Cfg cfg;
  TestRemoteCache remote_cache(cfg);
  remote_cache.SetUpdateDataFunc([](int key) -> unifex::task<std::optional<std::string>> {
    co_return co_await ytlib::AsyncWrapper<std::optional<std::string>>(std::bind(AsyncUpdateDataFunc, key, std::placeholders::_1));
  });

  unifex::timed_single_thread_context ctx;

  auto work = [&]() -> unifex::task<bool> {
    for (int ii = 0; ii < 60; ++ii) {
      auto ret = co_await remote_cache.Get(ii);
      DBG_PRINT("key: %d, val: %s", ii, ret ? ret->c_str() : "std::nullopt");
    }
    for (int ii = 60; ii > 0; --ii) {
      auto ret = co_await remote_cache.Get(ii);
      DBG_PRINT("key: %d, val: %s", ii, ret ? ret->c_str() : "std::nullopt");
    }
    co_return true;
  };

  auto work_ret = unifex::sync_wait(work());
  EXPECT_TRUE(*work_ret);
}

}  // namespace ytlib