# CMakeLists.txt for tests/ directory
# This file creates test executables and registers them with CTest

# Enable testing for this project
# This command enables the add_test() function and creates the "test" build target
# After calling enable_testing(), you can run tests with: ctest --test-dir build
enable_testing()

# Include GoogleTest CMake utilities
# This provides the gtest_discover_tests() function for automatic test discovery
# gtest_discover_tests() is better than add_test() because it automatically finds
# and registers all TEST() and TEST_F() macros in the executable
include(GoogleTest)

# Define test executable name derived from core library name
# Convention: test_<core_library_name>
set(TEST_EXECUTABLE test_${CORE_LIB})

# Create test executable
# The executable name is derived from CORE_LIB for consistency
add_executable(${TEST_EXECUTABLE}
    test_tank_model.cpp
    test_pid_controller.cpp
)

# Link test executable against required libraries
# PRIVATE keyword means these are only for building this executable, not for consumers
target_link_libraries(${TEST_EXECUTABLE} PRIVATE
    ${CORE_LIB}            # Core library being tested
    gtest                  # GoogleTest framework library
    gtest_main             # GoogleTest main function (contains default main())
                           # If you want custom main, remove gtest_main and write your own
)

# Specify include directories for the test executable
# PRIVATE keyword means includes are only used when compiling this target
target_include_directories(${TEST_EXECUTABLE} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Automatically discover and register all tests in the executable
# gtest_discover_tests() parses the executable for TEST() macros and creates
# CTest entries for each one. This is better than manually listing tests.
gtest_discover_tests(${TEST_EXECUTABLE})
