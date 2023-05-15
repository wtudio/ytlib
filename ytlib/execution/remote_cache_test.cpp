#include <gtest/gtest.h>

#include "ytlib/execution/execution_tools.hpp"
#include "ytlib/execution/remote_cache.hpp"

#include <unifex/sync_wait.hpp>

namespace ytlib {

TEST(EXECUTION_TEST, RemoteCache) {
  using TestRemoteCache = RemoteCache<int, std::string>;

  TestRemoteCache::Cfg cfg;
  cfg.local_cache_cfg.ttl = std::chrono::milliseconds(1000);
  TestRemoteCache remote_cache(cfg);

  // 模拟异步请求
  uint32_t prefix = 0;
  auto AsyncUpdateDataFunc = [&prefix](uint32_t key, const std::function<void(std::optional<std::string>&&)>& callback) {
    std::thread t([&prefix, key, callback]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      callback(std::to_string(prefix + key));
      return;
    });
    t.detach();
  };

  remote_cache.SetUpdateDataFunc([&](int key) -> unifex::task<std::optional<std::string>> {
    co_return co_await AsyncWrapper<std::optional<std::string>>(std::bind(AsyncUpdateDataFunc, key, std::placeholders::_1));
  });

  // 第一轮，都去查远程接口
  for (int ii = 0; ii < 10; ++ii) {
    auto ret = unifex::sync_wait(remote_cache.Get(ii));

    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret);
    EXPECT_STREQ((*ret)->c_str(), std::to_string(ii).c_str());
  }

  // 第二轮，都去查缓存
  prefix = 100;
  for (int ii = 0; ii < 10; ++ii) {
    auto ret = unifex::sync_wait(remote_cache.Get(ii));

    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret);
    EXPECT_STREQ((*ret)->c_str(), std::to_string(ii).c_str());
  }

  // 第三轮，缓存失效，都去查远程接口
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  for (int ii = 0; ii < 10; ++ii) {
    auto ret = unifex::sync_wait(remote_cache.Get(ii));

    EXPECT_TRUE(ret);
    EXPECT_TRUE(*ret);
    EXPECT_STREQ((*ret)->c_str(), std::to_string(100 + ii).c_str());
  }
}

}  // namespace ytlib