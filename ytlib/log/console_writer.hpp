#pragma once

#include "log.hpp"
#include "log_exports.h"

namespace ytlib {

class ConsoleWriter {
 public:
  void Write(const LogData& data);
  void WriteDirectly(const LogData& data);

  bool print_color_ = true;
};

LOG_API LogWriter GetConsoleWriter(const std::map<std::string, std::string>& cfg);

}  // namespace ytlib
