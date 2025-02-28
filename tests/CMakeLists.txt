
cmake_minimum_required(VERSION 3.10)

# Set project name and version
project(FrancosTests VERSION 1.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find GoogleTest
find_package(GTest REQUIRED)

# Add test source files
# file(GLOB TEST_SOURCES "tests/*.cpp")  # Change if your test files are in a subfolder

# Create test executable
add_executable(FrancosTest 
    test_queue.cpp
    test_ping_pong.cpp
)

# Link GoogleTest and pthread
target_link_libraries(FrancosTest PRIVATE GTest::GTest GTest::Main pthread francos)

# Include directories if needed
target_include_directories(FrancosTest PRIVATE ../include)
target_compile_options(FrancosTest PRIVATE -g -Wall -Wextra)


# Discover tests
include(GoogleTest)
gtest_discover_tests(FrancosTest)
