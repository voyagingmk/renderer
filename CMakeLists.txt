cmake_minimum_required (VERSION 3.5)


SET(buildtarget "dll" CACHE STRING "buildtarget = exe/dll/staic/")

if(UNIX)
    MESSAGE(STATUS "It is UNIX-like OS")  
    SET (CMAKE_C_COMPILER       "/usr/bin/clang")
    SET (CMAKE_CXX_COMPILER     "/usr/bin/clang++")
endif()

project (renderer)

set (FORWARDER_VERSION_MAJOR 1)
set (FORWARDER_VERSION_MINOR 0)
set (CMAKE_CXX_STANDARD 11)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")

FILE(GLOB_RECURSE MY_HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")
source_group("include" FILES ${MY_HEADERS})

include_directories("${PROJECT_SOURCE_DIR}")

include_directories("${PROJECT_SOURCE_DIR}/external")
include_directories("${PROJECT_SOURCE_DIR}/external/SOIL")
include_directories("${PROJECT_SOURCE_DIR}/external/SDL")
include_directories("${PROJECT_SOURCE_DIR}/external/imgui")
include_directories("${PROJECT_SOURCE_DIR}/external/reactphysics3d")
#include_directories("${PROJECT_SOURCE_DIR}/external/GL")
#include_directories("${PROJECT_SOURCE_DIR}/external/assimp")
#include_directories("${PROJECT_SOURCE_DIR}/external/vulkan")
#include_directories("${PROJECT_SOURCE_DIR}/external/ozz")
#include_directories("${PROJECT_SOURCE_DIR}/external/GLFW")
#include_directories("${PROJECT_SOURCE_DIR}/external/freetype")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/include/ecs")
include_directories("${PROJECT_SOURCE_DIR}/src")

FILE(GLOB_RECURSE EXTERNAL_GL 
  ${PROJECT_SOURCE_DIR}/external/GL/*
)
FILE(GLOB_RECURSE EXTERNAL_OZZ 
  ${PROJECT_SOURCE_DIR}/external/ozz/*
)
FILE(GLOB_RECURSE EXTERNAL_PHYSICS 
  ${PROJECT_SOURCE_DIR}/external/reactphysics3d/*
)
FILE(GLOB_RECURSE EXTERNAL_IMGUI 
  ${PROJECT_SOURCE_DIR}/external/imgui/*
)
FILE(GLOB_RECURSE EXTERNAL_GLFW 
  ${PROJECT_SOURCE_DIR}/external/GLFW/*
)
FILE(GLOB_RECURSE EXTERNAL_FREETYPE
  ${PROJECT_SOURCE_DIR}/external/freetype/*
)
FILE(GLOB_RECURSE EXTERNAL_ASSIMP
  ${PROJECT_SOURCE_DIR}/external/assimp/*
)
FILE(GLOB_RECURSE EXTERNAL_SDL
  ${PROJECT_SOURCE_DIR}/external/SDL/*
)
FILE(GLOB_RECURSE EXTERNAL_VULKAN
  ${PROJECT_SOURCE_DIR}/external/vulkan/*
)

FILE(GLOB_RECURSE HEADER_EVENT 
  ${PROJECT_SOURCE_DIR}/include/event/*.hpp
)
FILE(GLOB_RECURSE HEADER_UTILS
  ${PROJECT_SOURCE_DIR}/include/utils/*.hpp
)
FILE(GLOB_RECURSE HEADER_ECS 
  ${PROJECT_SOURCE_DIR}/include/ecs/*.hpp
)
FILE(GLOB_RECURSE HEADER_SYSTEM
  ${PROJECT_SOURCE_DIR}/include/system/*.hpp
)
FILE(GLOB_RECURSE HEADER_COM
  ${PROJECT_SOURCE_DIR}/include/com/*.hpp
)

FILE(GLOB_RECURSE SRC_MAIN
  ${PROJECT_SOURCE_DIR}/src/main.cpp
  ${PROJECT_SOURCE_DIR}/src/main_ecs.cpp
)
FILE(GLOB_RECURSE SRC_ECS 
  ${PROJECT_SOURCE_DIR}/src/ecs/*.cpp
)
FILE(GLOB_RECURSE SRC_SYSTEM
  ${PROJECT_SOURCE_DIR}/src/system/*.cpp
)
FILE(GLOB_RECURSE SRC_COM
  ${PROJECT_SOURCE_DIR}/src/com/*.cpp
)
FILE(GLOB_RECURSE SRC_UTILS
  ${PROJECT_SOURCE_DIR}/src/utils/*.cpp
)
FILE(GLOB_RECURSE SRC_GL
  ${PROJECT_SOURCE_DIR}/external/GL/*.c
)
FILE(GLOB_RECURSE SRC_OZZ
  ${PROJECT_SOURCE_DIR}/external/ozz/*.cc
)
FILE(GLOB_RECURSE SRC_PHYSICS
  ${PROJECT_SOURCE_DIR}/external/reactphysics3d/*/*.cpp
)
FILE(GLOB_RECURSE SRC_IMGUI
  ${PROJECT_SOURCE_DIR}/external/imgui/*.cpp
)

source_group("include\\ecs"  FILES ${HEADER_ECS})
source_group("include\\event"  FILES ${HEADER_EVENT})
source_group("include\\utils"  FILES ${HEADER_UTILS})
source_group("include\\system"  FILES ${HEADER_SYSTEM})
source_group("include\\com"  FILES ${HEADER_COM})


source_group("src\\ecs"  FILES ${SRC_ECS})
source_group("src\\utils"  FILES ${SRC_UTILS})
source_group("src\\com"  FILES ${SRC_COM})
source_group("src\\system"  FILES ${SRC_SYSTEM})
source_group("src"  FILES ${SRC_MAIN})

source_group("external\\GL"  FILES ${EXTERNAL_GL})
source_group("external\\ozz"  FILES ${EXTERNAL_OZZ})
source_group("external\\reactphysics3d"  FILES ${EXTERNAL_PHYSICS})
source_group("external\\GLFW"  FILES ${EXTERNAL_GLFW})
source_group("external\\freetype"  FILES ${EXTERNAL_FREETYPE})
source_group("external\\assimp"  FILES ${EXTERNAL_ASSIMP})
source_group("external\\SDL"  FILES ${EXTERNAL_SDL})
source_group("external\\vulkan"  FILES ${EXTERNAL_VULKAN})
source_group("external\\imgui"  FILES ${EXTERNAL_IMGUI})


IF(APPLE)
   #SET(GUI_TYPE MACOSX_BUNDLE)
   FIND_LIBRARY(CARBON_LIBRARY Carbon)
   FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
   FIND_LIBRARY(CF_LIBRARY CoreFoundation)
   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
   FIND_LIBRARY(COREAUDIO_LIBRARY CoreAudio)
   FIND_LIBRARY(AUDIOTOOLBOX_LIBRARY AudioToolbox)
   FIND_LIBRARY(COREVIDEO_LIBRARY CoreVideo)
   FIND_LIBRARY(IOKIT_LIBRARY IOKit)
   FIND_LIBRARY(FORCEFEEDBACK_LIBRARY ForceFeedback)
   FIND_LIBRARY(ASSIMP_LIBRARY assimp)
   FIND_LIBRARY(FREETYPE_LIBRARY freetype)
   
   MARK_AS_ADVANCED (CARBON_LIBRARY
                     OPENGL_LIBRARY
                     CF_LIBRARY
                     COCOA_LIBRARY
                     COREAUDIO_LIBRARY
                     AUDIOTOOLBOX_LIBRARY
                     COREVIDEO_LIBRARY
                     IOKIT_LIBRARY
                     FORCEFEEDBACK_LIBRARY
                     ASSIMP_LIBRARY
                     FREETYPE_LIBRARY)
   SET(EXTRA_LIBS "-Wl,--allow-multiple-definition" ${CARBON_LIBRARY} ${OPENGL_LIBRARY} 
    ${CF_LIBRARY} ${COCOA_LIBRARY}
    ${COREAUDIO_LIBRARY} ${AUDIOTOOLBOX_LIBRARY} 
    ${COREVIDEO_LIBRARY} ${IOKIT_LIBRARY} 
    ${FORCEFEEDBACK_LIBRARY} ${ASSIMP_LIBRARY}
    ${FREETYPE_LIBRARY})
ENDIF (APPLE)

#如果出现link冲突SDL，则尝试项目-属性-链接器-命令行-附加参数加入   /FORCE:MULTIPLE

link_directories(
    "./lib/mac/"
    "/usr/local/lib/"
)


set (EXTRA_LIBS ${EXTRA_LIBS} SDL2)
set (EXTRA_LIBS ${EXTRA_LIBS} SOIL)

IF (WIN32)
set (EXTRA_LIBS ${EXTRA_LIBS} SDL2main)
set (EXTRA_LIBS ${EXTRA_LIBS} glu32)
set (EXTRA_LIBS ${EXTRA_LIBS} opengl32)
set (EXTRA_LIBS ${EXTRA_LIBS} SOIL)
set (EXTRA_LIBS ${EXTRA_LIBS} assimp-vc140-mt)
set (EXTRA_LIBS ${EXTRA_LIBS} freetype)
set (EXTRA_LIBS ${EXTRA_LIBS} Version)
set (EXTRA_LIBS ${EXTRA_LIBS} winmm)
link_directories(
    "lib/"
    "lib/32/"
    "lib/32/debug/"
    "lib/32/debug/static"
)
ENDIF (WIN32)

add_executable(renderer  ${SRC_MAIN} ${SRC_ECS} ${SRC_COM} ${SRC_SYSTEM} ${SRC_UTILS}
    ${EXTERNAL_SDL} ${EXTERNAL_IMGUI} ${EXTERNAL_ASSIMP} ${EXTERNAL_PHYSICS} ${EXTERNAL_OZZ} ${EXTERNAL_IMGUI}
    ${EXTERNAL_GL} ${EXTERNAL_GLFW}
    ${HEADER_ECS} ${HEADER_EVENT} ${HEADER_UTILS} ${HEADER_SYSTEM} ${HEADER_COM}
    ${PROJECT_SOURCE_DIR})


IF (WIN32)
set_property(TARGET renderer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")
ENDIF (WIN32)

find_package( Threads )
target_link_libraries (renderer ${EXTRA_LIBS})

install (TARGETS renderer DESTINATION bin)
