cmake_minimum_required(VERSION 3.14)
project(CompGraf)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto")
  add_compile_options(-Wno-changes-meaning)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF)

include_directories(src)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_library(lib STATIC ${SOURCES})

target_link_libraries(lib
    PRIVATE
    OpenGL::GL
    SDL2::SDL2
)

foreach(source_file ${SOURCES})
  file(READ ${source_file} CONTENTS)
  if(CONTENTS MATCHES "main\\s*\\(")
    get_filename_component(name ${source_file} NAME_WE)
    add_executable(${name} ${source_file})
    target_link_libraries(${name} PRIVATE lib)
  endif()
endforeach()
