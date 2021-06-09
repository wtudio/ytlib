#pragma once

#include "ytlib/misc/shared_lib_def.h"

#if defined(math_tools_EXPORTS)
  #define MATH_TOOLS_API YT_DECLSPEC_EXPORT
#else
  #define MATH_TOOLS_API YT_DECLSPEC_IMPORT
#endif
