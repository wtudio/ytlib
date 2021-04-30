#pragma once

#include "log.hpp"

namespace ytlib {

class ConsoleWriter {
 public:
  void Write(const LogData& data);
  void WriteDirectly(const LogData& data);

  bool print_color_ = true;
};

LogWriter GetConsoleWriter(const std::map<std::string, std::string>& cfg);

}
