/**
 * @file uuid.hpp
 * @brief UUID
 * @details 基于boost的UUID
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace ytlib {

///生成UUID
inline std::string GenUUID(void) {
  boost::uuids::random_generator rgen;
  return boost::uuids::to_string(rgen());
}

}  // namespace ytlib
