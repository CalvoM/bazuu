cmake_minimum_required(VERSION 3.20)

project(
  bazuu
  VERSION 1.0
  DESCRIPTION "Bazuu, a chess engine."
  LANGUAGES CXX
)

include(FetchContent)

# C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Sources
file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cc)

# Library target
add_library(bazuu_lib ${SOURCES})

target_include_directories(bazuu_lib
  PUBLIC ${CMAKE_SOURCE_DIR}/includes
)

# Warnings (debug-oriented, but cheap)
target_compile_options(bazuu_lib
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

option(WITH_ASCII_ONLY "No Unicode Support" OFF)
target_compile_definitions(bazuu_lib
  PRIVATE
  ASCII_ONLY=$<BOOL:${WITH_ASCII_ONLY}>
)
# Speed-focused optimizations
target_compile_options(bazuu_lib
  PRIVATE
    $<$<CONFIG:Release>:
      -O3
      -march=native
      -mtune=native
      -ffast-math
      -fstrict-aliasing
      -funroll-loops
    >
)

# Link-time optimization
set_property(TARGET bazuu_lib PROPERTY
  INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
)

# Executable
add_executable(bazuu bazuu.cxx)

target_link_libraries(bazuu
  PRIVATE bazuu_lib
)

target_compile_options(bazuu
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Tests
enable_testing()
add_subdirectory(tests)

