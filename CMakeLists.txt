cmake_minimum_required(VERSION 3.14)
project(ImageProcessor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Collect source files (excluding main.cpp for library)
file(GLOB_RECURSE LIB_SOURCES
    "src/model/*.cpp"
    "src/operations/*.cpp"
    "src/command/*.cpp"
    "src/factory/*.cpp"
    "src/builder/*.cpp"
    "src/controller/*.cpp"
    "src/view/*.cpp"
)

# Core library (used by both app and tests)
add_library(imageprocessor_lib STATIC ${LIB_SOURCES})
target_include_directories(imageprocessor_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

# Main executable
add_executable(image_processor src/main.cpp)
target_link_libraries(image_processor PRIVATE imageprocessor_lib)

# Tests
enable_testing()
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
add_executable(run_tests ${TEST_SOURCES})
target_link_libraries(run_tests PRIVATE imageprocessor_lib GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(run_tests)
