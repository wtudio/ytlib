#pragma once

#include "ytlib/misc/shared_lib_def.h"

#if defined(string_tools_EXPORTS)
  #define STRING_TOOLS_API YT_DECLSPEC_EXPORT
#else
  #define STRING_TOOLS_API YT_DECLSPEC_IMPORT
#endif
