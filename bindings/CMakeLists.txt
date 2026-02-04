# CMakeLists.txt for bindings/ directory
# This directory contains language bindings and verification utilities for the C++ library

# ============================================================================
# VERIFICATION PROGRAMS
# ============================================================================

# Stepper verification program
# Tests RK4 implementation with exponential decay ODE
# Verifies: numerical accuracy and 4th-order convergence
add_executable(stepper_verify stepper_verify.cpp)
target_link_libraries(stepper_verify PRIVATE ${CORE_LIB})

# Simulator verification program
# Tests Simulator instantiation and basic operation
# Verifies: construction, step() execution, and state retrieval
add_executable(simulator_verify simulator_verify.cpp)
target_link_libraries(simulator_verify PRIVATE ${CORE_LIB})

# ============================================================================
# PYTHON BINDINGS (Phase 2)
# ============================================================================

# Python bindings will be added in Phase 2 using pybind11
# pybind11 is a lightweight header-only library that creates Python C++ extension modules
#
# When implementing Python bindings:
# 1. Declare pybind11 as a FetchContent dependency in root CMakeLists.txt
# 2. Create pybind11 module with pybind11_add_module()
# 3. Bind C++ classes and functions to Python
# 4. Link against tank_sim_core library
#
# Example structure (to be added later):
# pybind11_add_module(tank_sim_py tank_sim_py.cpp)
# target_link_libraries(tank_sim_py PRIVATE tank_sim_core)
