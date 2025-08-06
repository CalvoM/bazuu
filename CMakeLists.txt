cmake_minimum_required(VERSION 3.16)
project(bazuu VERSION 1.0 DESCRIPTION "Bazuu, a chess engine." LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
set(CMAKE_INSTALL_PREFIX "/usr/local/include")

file(GLOB SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cc)
add_library(${PROJECT_NAME}_lib ${SOURCES})
add_executable(${PROJECT_NAME} bazuu.cxx)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
# Catch2 Testing
enable_testing()
add_subdirectory(tests)
