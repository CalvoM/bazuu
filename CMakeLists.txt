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

# find_package(quill REQUIRED)
FetchContent_Declare(
  quill
  GIT_REPOSITORY "https://github.com/odygrd/quill"
  GIT_TAG a29940ebe1122556fa6d99be1929264469d6ec99 # v10.0.1
)
FetchContent_MakeAvailable(quill)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes/)

# configure the lib from src/*.cc
file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cc)
add_library(${PROJECT_NAME}_lib ${SOURCES})
target_link_libraries(${PROJECT_NAME}_lib PUBLIC quill::quill)
target_include_directories(${PROJECT_NAME}_lib
                           PUBLIC ${CMAKE_SOURCE_DIR}/includes)

# configure the main target
add_executable(${PROJECT_NAME} bazuu.cxx)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
# Catch2 Testing
enable_testing()
add_subdirectory(tests)
