#include <gtest/gtest.h>

#include <string>
#include <thread>
#include <utility>

#include "local_cache.hpp"

#include "ytlib/misc/misc_macro.h"

namespace ytlib {

TEST(CACHE_TEST, BASE_test) {
  using TestLocalCache = LocalCache<int, std::string>;
  TestLocalCache::Cfg cfg{
      .capacity = 10,
      .clean_size = 5,
      .ttl = std::chrono::milliseconds(10)};

  TestLocalCache cache(cfg);

  {
    auto ret = cache.Get(1);
    EXPECT_FALSE(ret);
    EXPECT_EQ(cache.Size(), 0);
  }

  {
    cache.Update(1, "test1");
    auto ret = cache.Get(1);
    ASSERT_TRUE(ret);
    EXPECT_STREQ(ret->c_str(), "test1");
    EXPECT_EQ(cache.Size(), 1);
  }

  // test ttl
  {
    cache.Update(2, "test2");
    auto ret = cache.Get(2);
    ASSERT_TRUE(ret);
    EXPECT_STREQ(ret->c_str(), "test2");
    EXPECT_EQ(cache.Size(), 2);

    ret = cache.Get(2);
    ASSERT_TRUE(ret);
    EXPECT_STREQ(ret->c_str(), "test2");
    EXPECT_EQ(cache.Size(), 2);

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ret = cache.Get(2);
    EXPECT_FALSE(ret);
    EXPECT_EQ(cache.Size(), 0);
  }

  // test lru: capacity=10, clean_size=5; inserting the 10th item triggers Clean down to 5
  {
    for (int i = 1; i <= 9; ++i) {
      cache.Update(i, "test" + std::to_string(i));
    }
    EXPECT_EQ(cache.Size(), 9);

    cache.Update(10, "test10");
    EXPECT_EQ(cache.Size(), 5);

    for (int i = 1; i <= 5; ++i) {
      EXPECT_FALSE(cache.Get(i));
    }
    for (int i = 6; i <= 10; ++i) {
      ASSERT_TRUE(cache.Get(i));
    }
  }

  // test del
  {
    ASSERT_TRUE(cache.Get(6));
    cache.Del(6);
    EXPECT_FALSE(cache.Get(6));
  }

  // test clear
  {
    ASSERT_EQ(cache.Size(), 4);
    cache.Clear();
    EXPECT_EQ(cache.Size(), 0);
  }
}

// 验证 try_emplace 修复：rvalue 参数在新增/更新两个路径下都应正确转发，不出现 use-after-move
TEST(CACHE_TEST, UPDATE_rvalue_test) {
  using TestLocalCache = LocalCache<int, std::string>;
  TestLocalCache::Cfg cfg{.capacity = 10, .clean_size = 5, .ttl = std::chrono::seconds(60)};
  TestLocalCache cache(cfg);

  // 新增路径：传 rvalue
  std::string s1 = "hello world";
  cache.Update(1, std::move(s1));
  auto ret = cache.Get(1);
  ASSERT_TRUE(ret);
  EXPECT_EQ(*ret, "hello world");

  // 更新路径：传 rvalue 替换已存在 key
  std::string s2 = "second value";
  cache.Update(1, std::move(s2));
  ret = cache.Get(1);
  ASSERT_TRUE(ret);
  EXPECT_EQ(*ret, "second value");
  EXPECT_EQ(cache.Size(), 1);
}

// 验证多参构造：std::string(count, char)
TEST(CACHE_TEST, UPDATE_multi_arg_ctor_test) {
  using TestLocalCache = LocalCache<int, std::string>;
  TestLocalCache::Cfg cfg{.capacity = 10, .clean_size = 5, .ttl = std::chrono::seconds(60)};
  TestLocalCache cache(cfg);

  // 用 (size_t, char) 构造 std::string
  cache.Update(1, static_cast<size_t>(5), 'x');
  auto ret = cache.Get(1);
  ASSERT_TRUE(ret);
  EXPECT_EQ(*ret, "xxxxx");

  // 已存在 key，再用多参构造更新
  cache.Update(1, static_cast<size_t>(3), 'y');
  ret = cache.Get(1);
  ASSERT_TRUE(ret);
  EXPECT_EQ(*ret, "yyy");
}

// 验证 Cfg::Verify 强制最小 capacity
TEST(CACHE_TEST, CFG_min_capacity_test) {
  using TestLocalCache = LocalCache<int, std::string>;

  // capacity < 10 应被提升到 10
  TestLocalCache::Cfg cfg1{.capacity = 3, .clean_size = 1, .ttl = std::chrono::seconds(60)};
  TestLocalCache cache1(cfg1);
  EXPECT_EQ(cache1.GetCfg().capacity, 10u);
  EXPECT_EQ(cache1.GetCfg().clean_size, 1u);  // clean_size < capacity，保持原值

  // capacity == 0 也应被提升到 10
  TestLocalCache::Cfg cfg2{.capacity = 0, .clean_size = 0, .ttl = std::chrono::seconds(60)};
  TestLocalCache cache2(cfg2);
  EXPECT_EQ(cache2.GetCfg().capacity, 10u);

  // clean_size >= capacity 时被修正为 capacity * 0.9
  TestLocalCache::Cfg cfg3{.capacity = 100, .clean_size = 100, .ttl = std::chrono::seconds(60)};
  TestLocalCache cache3(cfg3);
  EXPECT_EQ(cache3.GetCfg().capacity, 100u);
  EXPECT_EQ(cache3.GetCfg().clean_size, 90u);
}

}  // namespace ytlib
