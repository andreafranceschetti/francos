
cmake_minimum_required(VERSION 3.10)

# Set project name and version
project(FrancosBench VERSION 1.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


function(add_benchmark BENCHMARK_NAME SOURCE_FILE SANITIZE)
    add_executable(${BENCHMARK_NAME} ${SOURCE_FILE})
    target_link_libraries(${BENCHMARK_NAME} PRIVATE francos)
    target_include_directories(${BENCHMARK_NAME} PRIVATE ../include)
    
    if(SANITIZE)
        target_compile_options(${BENCHMARK_NAME} PRIVATE -g -O0 -fsanitize=address -Wall -Wextra)
        target_link_options(${BENCHMARK_NAME} PRIVATE -fsanitize=address)
    else()
        target_compile_options(${BENCHMARK_NAME} PRIVATE -O3 -Wall -Wextra)
    endif()
endfunction()

# Instantiate benchmarks
add_benchmark(TestLatency latency.cpp OFF)
add_benchmark(TestThroughput throughput.cpp ON)
add_benchmark(TestScalability scalability.cpp OFF)
