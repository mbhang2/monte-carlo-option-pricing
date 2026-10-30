# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and language
project(IE522 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Dependencies (header-only): brew install boost eigen  |  apt install libboost-dev libeigen3-dev
find_package(Boost REQUIRED)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(Threads REQUIRED)

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif()

# Include directories
include_directories(include)

# Add the executable target
add_executable(main
    src/main.cpp
    include/AmericanOption.cpp
    include/AsianOption.cpp
    include/EuropeanOption.cpp
    include/MonteCarlo.h
)

# Link Boost libraries to the 'main' target
target_link_libraries(main ${Boost_LIBRARIES} Eigen3::Eigen Threads::Threads)

# Set the output directory for the executable
set_target_properties(main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/exec"
)

# Optionally, add compiler warnings
target_compile_options(main PRIVATE -Wall -Wextra -pedantic)
add_compile_options(-Wno-deprecated-copy)
