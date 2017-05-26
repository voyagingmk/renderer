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
source_group("My Headers" FILES ${MY_HEADERS})

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/include/realtime")
include_directories("${PROJECT_SOURCE_DIR}/include/shapes")
include_directories("${PROJECT_SOURCE_DIR}/SDL")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/realtime")
include_directories("${PROJECT_SOURCE_DIR}/src/shapes")

FILE(GLOB_RECURSE BASE_SOURCERS 
  ${PROJECT_SOURCE_DIR}/src/*.cpp 
  ${PROJECT_SOURCE_DIR}/src/realtime/*.cpp 
  ${PROJECT_SOURCE_DIR}/src/shapes/*.cpp 
  )

FILE(GLOB_RECURSE MAIN_SOURCERS ${PROJECT_SOURCE_DIR}/main/*.cpp)

set (EXTRA_LIBS ${EXTRA_LIBS} SDL2)

add_executable(renderer ${BASE_SOURCERS} ${PROJECT_SOURCE_DIR})


find_package( Threads )
target_link_libraries (renderer ${EXTRA_LIBS})

install (TARGETS renderer DESTINATION bin)
