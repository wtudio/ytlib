#include <gtest/gtest.h>

#include <string>

#include "local_cache.hpp"

#include "ytlib/misc/misc_macro.h"

namespace ytlib {

TEST(CACHE_TEST, BASE_test) {
  LocalCacheCfg cfg = LocalCacheCfg{
      .capacity = 10,
      .clean_size = 8,
      .ttl_ms = 500000};

  LocalCache<int, std::string> cache(cfg);
  auto ret = cache.Get(1);
  DBG_PRINT("%s", ret ? ret->c_str() : "std::nullopt");

  cache.Update(1, "test1");
  ret = cache.Get(1);
  DBG_PRINT("%s", ret ? ret->c_str() : "std::nullopt");

  cache.Update(2, "test2");
  ret = cache.Get(2);
  DBG_PRINT("%s", ret ? ret->c_str() : "std::nullopt");
}

}  // namespace ytlib
