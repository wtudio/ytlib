#pragma once

// 在cmake中调用【define_filename_macro】来定义__FILENAME__，以在日志中打印相对路径
#if !defined(__FILENAME__)
  #define __FILENAME__ __FILE__
#endif

#define _STRING(x) #x
#define STRING(x) _STRING(x)

#ifdef _DEBUG
  #define DBG_PRINT(fmt, ...) printf("[" __FILENAME__ ":" STRING(__LINE__) "@%s]" fmt "\n", __FUNCTION__, ##__VA_ARGS__)
#else
  #define DBG_PRINT(fmt, ...)
#endif
