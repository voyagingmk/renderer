// header file for building precompiled headers under windows
// a no-op on other architectures

#if defined(_MSC_VER)

#include "CImg.h"
#include "json.hpp"
#include "MemoryPool.h"
#include "error.hpp"

#endif // _MSC_VER
