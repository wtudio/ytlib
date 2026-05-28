/**
 * @file memory.hpp
 * @brief NPC 短期记忆（MVP 留接口不读取）
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

#include <string>

#include "core/types.hpp"

namespace ytlib::testgame3 {

struct MemoryEntry {
  GameTime t = 0;
  std::string text;
};

}  // namespace ytlib::testgame3
