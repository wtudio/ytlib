/**
 * @file random.hpp
 * @brief 全局可复现随机数源
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <random>

namespace ytlib::testgame3 {

class WorldRng {
 public:
  explicit WorldRng(uint64_t seed) : engine_(seed) {}

  bool Chance(float p) {
    return std::uniform_real_distribution<float>(0.0f, 1.0f)(engine_) < p;
  }

  int IntRange(int lo, int hi) {
    return std::uniform_int_distribution<int>(lo, hi)(engine_);
  }

  float FloatRange(float lo, float hi) {
    return std::uniform_real_distribution<float>(lo, hi)(engine_);
  }

 private:
  std::mt19937_64 engine_;
};

}  // namespace ytlib::testgame3
