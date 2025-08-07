# find_package(Catch2 3 REQUIRED)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY "https://github.com/catchorg/Catch2"
  GIT_TAG fee81626d2a4811095c3a39d20fb355eeb954101 # v3.9.0
)
FetchContent_MakeAvailable(Catch2)
set(TEST_BINARY ${PROJECT_NAME}_test)
add_executable(${TEST_BINARY} tests.cxx)
target_link_libraries(${TEST_BINARY} PRIVATE ${PROJECT_NAME}_lib
                                             Catch2::Catch2WithMain)
target_include_directories(${TEST_BINARY} PRIVATE ${CMAKE_SOURCE_DIR}/includes)
include(CTest)
include(Catch)
catch_discover_tests(${TEST_BINARY})
