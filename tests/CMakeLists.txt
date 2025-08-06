find_package(Catch2 3 REQUIRED)
set(TEST_BINARY ${PROJECT_NAME}_test)
add_executable(${TEST_BINARY} tests.cxx)
target_link_libraries(${TEST_BINARY} PRIVATE ${PROJECT_NAME}_lib Catch2::Catch2WithMain)
target_include_directories(${TEST_BINARY} PRIVATE ${CMAKE_SOURCE_DIR}/includes)
include(CTest)
include(Catch)
catch_discover_tests(${TEST_BINARY})


