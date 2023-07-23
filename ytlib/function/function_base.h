#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef YTLIB_FUNCTION_LOCAL_BUF_SIZE
  #define YTLIB_FUNCTION_LOCAL_BUF_SIZE (3 * sizeof(void*))
#endif

typedef struct {
  unsigned char object_buf[YTLIB_FUNCTION_LOCAL_BUF_SIZE];
  const void* ops;
} ytlib_function_base_t;

#ifdef __cplusplus
}
#endif
