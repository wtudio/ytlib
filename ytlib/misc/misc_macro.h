#pragma once

#define _STRING(x) #x
#define STRING(x) _STRING(x)
#define COMMON_FMT(fmt) "[" __FILENAME__ ":" STRING(__LINE__) "@%s]" fmt "\n", __FUNCTION__

#ifdef _DEBUG
  #define DBG_PRINT(fmt, ...) printf(COMMON_FMT(fmt), ##__VA_ARGS__)
#else
  #define DBG_PRINT(fmt, ...)
#endif
