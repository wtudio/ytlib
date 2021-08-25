#pragma once

#ifdef _DEBUG
  #define _DBG_PRINT_STRING(x) #x
  #define DBG_PRINT_STRING(x) _DBG_PRINT_STRING(x)
  #define DBG_PRINT(fmt, ...) printf("[" __FILE__ ":" DBG_PRINT_STRING(__LINE__) "@%s]" fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#else
  #define DBG_PRINT(fmt, ...)
#endif
