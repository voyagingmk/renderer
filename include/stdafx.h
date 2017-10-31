
#if defined(_MSC_VER)
#include "CImg.h"
#endif
#include "json.hpp"
#include "utils/MemoryPool.hpp"
#include "utils/error.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
