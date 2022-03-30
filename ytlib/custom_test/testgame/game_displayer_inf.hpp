#pragma once

#include <list>
#include <memory>
#include "ytlib/math/vector3.hpp"

namespace ytlib {

struct GameBufObj {
  Vector3<float> location = {0.0, 0.0, 0.0};   // (m,m,m)
  Vector3<float> direction = {1.0, 0.0, 0.0};  // 单位向量，实际船头指向
};

struct GameBuf {
  std::list<GameBufObj> obj_list;
};

class GameDisplayerInf {
 public:
  GameDisplayerInf() {}
  virtual ~GameDisplayerInf() {}

  virtual void Start() = 0;
  virtual void Stop() = 0;
  virtual void SyncBuf(std::shared_ptr<GameBuf> buf) = 0;
};

}  // namespace ytlib
