#pragma once

#include "ytlib/misc/shared_lib_def.h"

#if defined(math_EXPORTS)
  #define MATH_API YT_DECLSPEC_EXPORT
#else
  #define MATH_API YT_DECLSPEC_IMPORT
#endif
