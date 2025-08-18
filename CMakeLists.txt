include(FetchContent)
cmake_minimum_required(VERSION 3.16)
project(
  bazuu
  VERSION 1.0
  DESCRIPTION "Bazuu, a chess engine."
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INSTALL_PREFIX "/usr/local/include")
set(CMAKE_CONFIGURATION_TYPES "Debug")
set(CMAKE_BUILD_TYPE "Debug")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes/)

# configure the lib from src/*.cc
file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cc)
add_library(${PROJECT_NAME}_lib ${SOURCES})
target_include_directories(${PROJECT_NAME}_lib
                           PUBLIC ${CMAKE_SOURCE_DIR}/includes)

# configure the main target
add_executable(${PROJECT_NAME} bazuu.cxx)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
# Catch2 Testing
enable_testing()
add_subdirectory(tests)
