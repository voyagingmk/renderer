
#if defined(_MSC_VER)
#include "CImg.h"
#endif
#include "json.hpp"
#include "utils/MemoryPool.hpp"
#include "utils/error.hpp"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags



#define USE_GL

#include <algorithm>
#include <memory>



#include "SDL.h"
// #include "SDL_opengl.h"



#ifdef USE_GL

#ifdef _MSC_VER
#include "GL/gl3w.h"
#include <GL/GL.h>
#include <GL/GLU.h>
#else
#ifdef __APPLE__
#include "GL/gl3w.h"
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#define GL3_PROTOTYPES 1
#include <OpenGL/gl3.h>
#endif
#endif

#endif

#include "SOIL.h"
