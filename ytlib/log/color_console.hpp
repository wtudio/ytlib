#pragma once

namespace ytlib {

#if defined(_WIN32)
  #include <windows.h>

  #define CC_DEFAULT (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
  #define CC_DBG (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
  #define CC_INF (FOREGROUND_GREEN)
  #define CC_WRN (FOREGROUND_GREEN | FOREGROUND_RED)
  #define CC_ERR (FOREGROUND_BLUE | FOREGROUND_RED)
  #define CC_FATAL (BACKGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)

#else
  #define CC_NONE "\033[0m"
  #define CC_BLACK "\033[0;30m"
  #define CC_L_BLACK "\033[1;30m"
  #define CC_RED "\033[0;31m"
  #define CC_L_RED "\033[1;31m"
  #define CC_GREEN "\033[0;32m"
  #define CC_L_GREEN "\033[1;32m"
  #define CC_BROWN "\033[0;33m"
  #define CC_YELLOW "\033[1;33m"
  #define CC_BLUE "\033[0;34m"
  #define CC_L_BLUE "\033[1;34m"
  #define CC_PURPLE "\033[0;35m"
  #define CC_L_PURPLE "\033[1;35m"
  #define CC_CYAN "\033[0;36m"
  #define CC_L_CYAN "\033[1;36m"
  #define CC_GRAY "\033[0;37m"
  #define CC_WHITE "\033[1;37m"

  #define CC_BOLD "\033[1m"
  #define CC_UNDERLINE "\033[4m"
  #define CC_BLINK "\033[5m"
  #define CC_REVERSE "\033[7m"
  #define CC_HIDE "\033[8m"
  #define CC_CLEAR "\033[2J"

  #define CC_DBG CC_WHITE
  #define CC_INF CC_GREEN
  #define CC_WRN CC_YELLOW
  #define CC_ERR CC_PURPLE
  #define CC_FATAL CC_RED
#endif

}  // namespace ytlib
