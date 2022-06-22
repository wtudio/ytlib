#pragma once

#include <cstdio>

#include "ytlib/dll_tools/shared_lib_def.h"

#if defined(testlib2_EXPORTS)
  #define TESTLIB2_API YT_DECLSPEC_EXPORT
#else
  #define TESTLIB2_API YT_DECLSPEC_IMPORT
#endif

extern "C" {
TESTLIB2_API int add(int a, int b);
TESTLIB2_API int sub(int a, int b);
}
