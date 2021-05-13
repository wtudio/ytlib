#pragma once

#include "ytlib/Common/shared_lib_def.h"

#if defined(log_EXPORTS)
  #define LOG_API YT_DECLSPEC_EXPORT
#else
  #define LOG_API YT_DECLSPEC_IMPORT
#endif
