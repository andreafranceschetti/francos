cmake_minimum_required(VERSION 3.10)

# Set project name and version
project(MyTests VERSION 1.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add include directory
include_directories(../include)

# # Enable testing
# enable_testing()

# # Fetch GoogleTest
# include(FetchContent)
# FetchContent_Declare(
#     googletest
#     URL https://github.com/google/googletest/archive/refs/tags/v1.12.1.zip
# )
# FetchContent_MakeAvailable(googletest)

# # Add test source files
# file(GLOB TEST_SOURCES "*.cpp")

# # Create test executable
# add_executable(MyTests ${TEST_SOURCES})

# # Link GoogleTest
# target_link_libraries(MyTests PRIVATE gtest gtest_main)

# # Add tests
# include(GoogleTest)
# gtest_discover_tests(MyTests)
