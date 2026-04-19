cmake_minimum_required(VERSION 3.14)
project(CompGraf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glm REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/**.cpp"
)

set(LIB_SOURCES "")
set(MAIN_SOURCES "")

foreach(source_file IN LISTS SOURCES)
  file(READ "${source_file}" CONTENTS)
  if(CONTENTS MATCHES "(^|[^A-Za-z0-9_])main[\t\r\n]*\\(")
    list(APPEND MAIN_SOURCES "${source_file}")
  else()
    list(APPEND LIB_SOURCES "${source_file}")
  endif()
endforeach()

add_library(deps INTERFACE)
target_link_libraries(deps INTERFACE
  OpenGL::GL
  OpenGL::GLU
  SDL2::SDL2
  glm::glm
)

function(apply_common_options target)
  target_compile_options(${target} PRIVATE
    -Wall -Wextra -Wpedantic -Wno-changes-meaning
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG -march=native -flto>
  )
  target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}/src")
endfunction()

if(LIB_SOURCES)
  add_library(core STATIC ${LIB_SOURCES})
  target_link_libraries(core PUBLIC deps)
  target_include_directories(core PUBLIC "${PROJECT_SOURCE_DIR}/src")

  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    apply_common_options(core)
  endif()
endif()

foreach(source_file IN LISTS MAIN_SOURCES)
  get_filename_component(name "${source_file}" NAME_WE)

  add_executable(${name} "${source_file}")
  target_link_libraries(${name} PRIVATE deps)
  target_include_directories(${name} PRIVATE "${PROJECT_SOURCE_DIR}/src")

  if(TARGET core)
    target_link_libraries(${name} PRIVATE core)
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    apply_common_options(${name})
  endif()
endforeach()
