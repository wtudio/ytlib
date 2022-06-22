#pragma once

#include <cstdio>

#include "ytlib/dll_tools/shared_lib_def.h"

#if defined(testlib_EXPORTS)
  #define TESTLIB_API YT_DECLSPEC_EXPORT
#else
  #define TESTLIB_API YT_DECLSPEC_IMPORT
#endif

extern "C" {
TESTLIB_API int add(int a, int b);
TESTLIB_API int sub(int a, int b);
}
