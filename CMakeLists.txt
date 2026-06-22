cmake_minimum_required(VERSION 3.14)
project(CompGraf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/arkanoid/bin")

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

include(FetchContent)

find_package(PkgConfig REQUIRED)

find_package(OpenGL REQUIRED)

find_package(SDL2 QUIET)
if(NOT SDL2_FOUND)
  FetchContent_Declare(
    sdl2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG 3.4.8
  )

  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(sdl2)
endif()

pkg_check_modules(SDL2_image REQUIRED IMPORTED_TARGET SDL2_image)

find_package(glm QUIET)
if(NOT glm_FOUND)
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
  )

  FetchContent_MakeAvailable(glm)
endif()

find_package(Boost CONFIG QUIET)
if(NOT TARGET Boost::mp11)
  FetchContent_Declare(
    boost_mp11
    GIT_REPOSITORY https://github.com/boostorg/mp11.git
    GIT_TAG boost-1.91.0
  )

  set(BOOST_INCLUDE_LIBRARIES container CACHE STRING "" FORCE)
  set(BOOST_ENABLE_CMAKE ON CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(boost_mp11)
endif()

find_package(assimp QUIET)
if(NOT assimp_FOUND)
  FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.5
  )

  set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_3DS_IMPORTER ON CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
  set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
  set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
  set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(assimp)
endif()

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.5
)

FetchContent_MakeAvailable(imgui)

# ImGui ships no CMake for the core library, so build it explicitly with the
# SDL2 + fixed-function OpenGL2 backends (matches our GL 2.1 compat profile).
add_library(imgui STATIC
  "${imgui_SOURCE_DIR}/imgui.cpp"
  "${imgui_SOURCE_DIR}/imgui_draw.cpp"
  "${imgui_SOURCE_DIR}/imgui_tables.cpp"
  "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
  "${imgui_SOURCE_DIR}/imgui_demo.cpp"
  "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp"
  "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp"
)

target_include_directories(imgui PUBLIC
  "${imgui_SOURCE_DIR}"
  "${imgui_SOURCE_DIR}/backends"
)

target_link_libraries(imgui PUBLIC
  SDL2::SDL2
  OpenGL::GL
)


file(GLOB_RECURSE OME_SOURCES
  CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/oh-my-engine/*.cpp"
)

add_library(ome STATIC ${OME_SOURCES})

target_include_directories(ome PUBLIC
  "${PROJECT_SOURCE_DIR}/src"
)

target_link_libraries(ome PUBLIC
  SDL2::SDL2
  OpenGL::GL
  OpenGL::GLU
  PkgConfig::SDL2_image
  glm::glm
  Boost::mp11
  assimp::assimp
  imgui
)

file(GLOB_RECURSE GAME_SOURCES
  CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/arkanoid/*.cpp"
)

add_executable(game ${GAME_SOURCES})

target_link_libraries(game PRIVATE
  ome
)

function(apply_common_options target)
  target_compile_options(${target} PRIVATE
    -Wall -Wextra -Wpedantic -Wno-changes-meaning
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG -march=native -flto=auto>
  )
endfunction()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  apply_common_options(ome)
  apply_common_options(game)
endif()

add_custom_command(TARGET game POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${PROJECT_SOURCE_DIR}/assets"
          "${CMAKE_BINARY_DIR}/arkanoid/assets"
)
