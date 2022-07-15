#include <gtest/gtest.h>

#include <string>
#include <thread>

#include "local_cache.hpp"

#include "ytlib/misc/misc_macro.h"

namespace ytlib {

TEST(CACHE_TEST, BASE_test) {
  using TestLocalCache = LocalCache<int, std::string>;
  TestLocalCache::Cfg cfg{
      .capacity = 6,
      .clean_size = 3,
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

  // test lru
  {
    cache.Update(1, "test1");
    cache.Update(2, "test2");
    cache.Update(3, "test3");
    cache.Update(4, "test4");
    cache.Update(5, "test5");
    EXPECT_EQ(cache.Size(), 5);

    cache.Update(6, "test6");
    EXPECT_EQ(cache.Size(), 3);

    EXPECT_FALSE(cache.Get(1));
    EXPECT_FALSE(cache.Get(2));
    EXPECT_FALSE(cache.Get(3));

    ASSERT_TRUE(cache.Get(4));
    ASSERT_TRUE(cache.Get(5));
    ASSERT_TRUE(cache.Get(6));
  }
}

}  // namespace ytlib
