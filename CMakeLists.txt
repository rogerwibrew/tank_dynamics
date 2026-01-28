cmake_minimum_required(VERSION 3.20)
project(TankDynamics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch dependencies using FetchContent
include(FetchContent)

FetchContent_Declare(
    Eigen3
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG        3.4.0
)

FetchContent_Declare(
    GoogleTest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

# Try to find GSL using find_package, otherwise provide installation instructions
find_package(GSL REQUIRED)
if(NOT GSL_FOUND)
    message(FATAL_ERROR "GSL not found. Please install it via your system package manager:
    - On Ubuntu: sudo apt-get install libgsl-dev
    - On Arch: sudo pacman -S gsl")
endif()

FetchContent_MakeAvailable(Eigen3 GoogleTest)

# Define the main library target
add_library(tank_sim_core INTERFACE)

# Link dependencies to the library
target_link_libraries(tank_sim_core INTERFACE
    Eigen3::Eigen
    GSL::gsl
    GSL::gslcblas
)

# Set include directories for the library
target_include_directories(tank_sim_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Add subdirectories for src, tests, and bindings
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(bindings)
