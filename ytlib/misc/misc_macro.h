#pragma once

#define _STRING(x) #x
#define STRING(x) _STRING(x)

#ifdef _DEBUG
  #define DBG_PRINT(fmt, ...) printf("[" __FILE__ ":" STRING(__LINE__) "@%s]" fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#else
  #define DBG_PRINT(fmt, ...)
#endif
