#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "ytlib/math/vector3.hpp"

namespace ytlib {

class Ship : public std::enable_shared_from_this<Ship> {
 public:
  Ship() {}
  virtual ~Ship() {}

  void OnWorldLoopOnce(float dt) {
    direction_ = target_direction_;

    float acceleration = thrust_ / mass_;
    speed_ += (direction_ * (dt * acceleration));
    float cur_speed = speed_.Len();
    if (cur_speed > max_speed_) {
      speed_ = speed_ * (max_speed_ / cur_speed);
    }

    location_ += (speed_ * dt);
  }

 public:
  std::string name_;
  uint64_t id_;

  // 二阶状态，可以由AI决策时直接改变
  float mass_ = 1.0;    // kg
  float thrust_ = 0.0;  // N

  Vector3<float> target_direction_ = {1.0, 0.0, 0.0};  // 单位向量，目标指向，可瞬时转向

  float max_speed_ = 1000.0;  // m/s

  // 一阶状态，由二阶状态在每帧计算得到
  Vector3<float> location_ = {0.0, 0.0, 0.0};   // (m,m,m)
  Vector3<float> speed_ = {0.0, 0.0, 0.0};      // 速度向量，m/s
  Vector3<float> direction_ = {1.0, 0.0, 0.0};  // 单位向量，实际船头指向
};

}  // namespace ytlib
