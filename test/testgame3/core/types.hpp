/**
 * @file types.hpp
 * @brief 基础类型定义
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <cmath>
#include <cstdint>

namespace ytlib::testgame3 {

using Money = int64_t;
using GameTime = uint64_t;
using FacilityId = uint64_t;
using NpcId = uint64_t;
using ItemId = uint32_t;
using RecipeId = uint32_t;

constexpr FacilityId kNullFacility = 0;
constexpr NpcId kNullNpc = 0;

enum class FacilityType : uint8_t {
  Ship,
  Factory,
  Trade,
  Shipyard,
  Faction,
  MineArea,
};

enum class SkillType : uint8_t {
  ShipPilot,
  FactoryManager,
  Trader,
  FactionCommander,
};

struct Vec2 {
  float x = 0.0f;
  float y = 0.0f;

  Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
  Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
  Vec2 operator*(float s) const { return {x * s, y * s}; }

  float Length() const { return std::sqrt(x * x + y * y); }
  float Distance(const Vec2& o) const { return (*this - o).Length(); }
  Vec2 Normalized() const {
    float l = Length();
    return (l > 1e-6f) ? Vec2{x / l, y / l} : Vec2{0, 0};
  }
};

}  // namespace ytlib::testgame3
