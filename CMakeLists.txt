cmake_minimum_required(VERSION 3.10)

# Set project name and version
project(francos VERSION 1.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add source files
file(GLOB SOURCES "src/*.cpp")


add_library(francos ${SOURCES})
target_include_directories(francos PUBLIC include)
target_compile_definitions(francos PUBLIC )

# Enable testing
enable_testing()

add_subdirectory(tests)
add_subdirectory(benchmarks)
# Create executable
# add_executable(MyExecutable ${SOURCES})

# Link libraries (if any)
# target_link_libraries(MyExecutable PRIVATE some_library)