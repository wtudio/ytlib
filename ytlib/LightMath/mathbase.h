#pragma once
#include <ytlib/Common/Util.h>
#include <cmath>

#define USE_DOUBLE_PRECISION

#if defined(USE_DOUBLE_PRECISION)
typedef double tfloat;      // double precision
#else 
typedef float  tfloat;    // single precision
#endif 

#define PI 3.1415926535897932384626433832795028841971
