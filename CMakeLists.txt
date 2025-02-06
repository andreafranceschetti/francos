cmake_minimum_required(VERSION 3.10)

# Set project name and version
project(MyProject VERSION 1.0 LANGUAGES CXX)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add include directory
include_directories(include)

# Add source files
file(GLOB SOURCES "src/*.cpp")

add_subdirectory(tests)
# Create executable
# add_executable(MyExecutable ${SOURCES})

# Link libraries (if any)
# target_link_libraries(MyExecutable PRIVATE some_library)