# CMakeLists.txt for src/ directory
# This file adds source files to the core library target
# that was already defined in the root CMakeLists.txt

# Add source files to the core library target
# PRIVATE keyword means these files are implementation details, not part of the public interface
# target_sources() is preferred over add_library(lib STATIC file.cpp) because:
#   - The target is already defined in the parent scope (root CMakeLists.txt)
#   - We only need to add source files to the existing target
#   - It's cleaner for multi-level CMake projects with subdirectories
target_sources(${CORE_LIB} PRIVATE
    tank_model.cpp
    pid_controller.cpp
    stepper.cpp
    simulator.cpp
)

# Note: Header files (tank_model.h) are not listed here because:
#   - CMake doesn't require headers to be listed (they're found via #include)
#   - Headers are found via target_include_directories() set in root CMakeLists.txt
#   - Some developers list headers for IDE support, but it's optional
