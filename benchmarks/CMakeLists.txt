
cmake_minimum_required(VERSION 3.10)

# Set project name and version
project(FrancosBench VERSION 1.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Create test executable
add_executable(TestLatency latency.cpp)
target_link_libraries(TestLatency PRIVATE francos)
target_include_directories(TestLatency PRIVATE ../include)
target_compile_options(TestLatency PRIVATE -O3 -Wall -Wextra)
# target_link_options(TestLatency PRIVATE )


add_executable(TestThroughput throughput.cpp)
target_link_libraries(TestThroughput PRIVATE francos)
target_include_directories(TestThroughput PRIVATE ../include)
target_compile_options(TestThroughput PRIVATE -g -O0 -fsanitize=address -Wall -Wextra)
target_link_options(TestThroughput PRIVATE -fsanitize=address)