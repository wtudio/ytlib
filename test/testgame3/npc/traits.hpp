/**
 * @file traits.hpp
 * @brief NPC 个性特质
 * @author wtudio
 * @date 2026-05-28
 */
#pragma once

namespace ytlib::testgame3 {

struct NpcTraits {
  float risk = 0.5f;   // 越高越敢冒险（低耐久仍出航）
  float greed = 0.5f;  // 越高越追求高利润，倾向远距离套利
};

}  // namespace ytlib::testgame3
