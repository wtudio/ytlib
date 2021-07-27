#pragma once

#include "ytlib/misc/shared_lib_def.h"

#if defined(testlib_EXPORTS)
  #define TESTLIB_API YT_DECLSPEC_EXPORT
#else
  #define TESTLIB_API YT_DECLSPEC_IMPORT
#endif

namespace ytlib {

TESTLIB_API int add(int a, int b);

TESTLIB_API int sub(int a, int b);

}  // namespace ytlib
