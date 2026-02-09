# Tank Dynamics Simulator - Developer Guide

This guide is for developers working on the Tank Dynamics project. It covers project structure, development workflow, building, testing, and contributing.

## Table of Contents

1. [Project Overview](#project-overview)
2. [Development Environment Setup](#development-environment-setup)
3. [Project Structure](#project-structure)
4. [Building the Project](#building-the-project)
5. [Testing](#testing)
6. [Code Organization](#code-organization)
7. [Development Workflow](#development-workflow)
8. [Debugging Tips](#debugging-tips)
9. [Common Tasks](#common-tasks)

## Project Overview

Tank Dynamics Simulator is a real-time process simulation and control system with three main components:

- **C++ Simulation Core** (`libsim`): High-performance physics engine with GSL RK4 integrator
- **Python Bindings** (`tank_sim`): pybind11 interface exposing simulation to Python
- **FastAPI Backend + Next.js Frontend**: Web-based SCADA interface

The project follows a hybrid AI workflow with distinct roles for architecture, engineering, code review, and documentation. See [CLAUDE.md](../CLAUDE.md) for workflow details.

## Development Environment Setup

### System Requirements

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install cmake libgsl-dev build-essential git nodejs npm python3.9 python3-pip
```

**Arch Linux:**
```bash
sudo pacman -S cmake gsl base-devel git nodejs npm python
```

**macOS:**
```bash
brew install cmake gsl git node python@3.9
```

**Windows:** Not currently supported. Use WSL2 with Ubuntu setup above.

### Repository Setup

```bash
# Clone the repository
git clone <repository-url>
cd tank_dynamics

# Create development branches for your work
git checkout -b feature/your-feature-name
```

### IDE Setup

For proper code completion and go-to-definition with clangd:

```bash
# From project root
ln -sf build/compile_commands.json compile_commands.json
```

Then install the clangd extension in your editor:
- **VSCode**: Install "clangd" extension by LLVM
- **Neovim**: Use `nvim-lspconfig` with clangd
- **Emacs**: Use `eglot` with clangd
- **Other editors**: See https://clangd.llvm.org/installation

## Project Structure

```
tank_dynamics/
├── CMakeLists.txt                 # Top-level CMake configuration
├── build.sh                       # Build script for quick compilation
├── README.md                      # Project overview and quick start
├── CLAUDE.md                      # AI workflow configuration
│
├── src/                           # C++ simulation library source
│   ├── CMakeLists.txt            # C++ library CMake config
│   ├── tank_model.h              # Tank physics model header
│   ├── tank_model.cpp            # Tank physics model implementation
│   ├── pid_controller.h          # PID controller header
│   ├── pid_controller.cpp        # PID controller implementation
│   ├── stepper.h                 # GSL RK4 stepper wrapper header
│   ├── stepper.cpp               # GSL RK4 stepper wrapper implementation
│   ├── simulator.h               # Master simulator orchestrator (planned)
│   └── simulator.cpp             # Master simulator orchestrator (planned)
│
├── bindings/                      # pybind11 Python bindings
│   ├── CMakeLists.txt            # Bindings CMake config
│   ├── bindings.cpp              # pybind11 binding code
│   └── stepper_verify.cpp        # Stepper verification utility
│
├── tests/                         # C++ unit tests (GoogleTest)
│   ├── CMakeLists.txt            # Test CMake config
│   ├── test_tank_model.cpp       # TankModel unit tests
│   ├── test_pid_controller.cpp   # PIDController unit tests
│   ├── test_stepper.cpp          # Stepper unit tests (planned)
│   └── test_simulator.cpp        # Simulator unit tests (planned)
│
├── api/                           # FastAPI backend (planned)
│   ├── __init__.py
│   ├── main.py
│   ├── simulation.py
│   ├── models.py
│   └── tests/
│       └── test_api.py
│
├── frontend/                      # Next.js frontend (planned)
│   ├── package.json
│   ├── tailwind.config.js
│   ├── app/
│   │   ├── layout.tsx
│   │   ├── page.tsx
│   │   └── components/
│   │       ├── ProcessView.tsx
│   │       ├── TrendsView.tsx
│   │       └── TankGraphic.tsx
│   └── tests/
│       └── e2e/
│
└── docs/                          # Project documentation
    ├── specs.md                   # Feature specifications
    ├── plan.md                    # Architecture & implementation plan
    ├── next.md                    # Upcoming implementation tasks
    ├── DEVELOPER_GUIDE.md         # This file
    ├── Model Class.md             # TankModel detailed specification
    ├── PID Controller Class.md    # PIDController detailed specification
    ├── Stepper Class.md           # Stepper detailed specification
    ├── Simulator Class.md         # Simulator detailed specification
    └── TankDynamics.md            # Process dynamics theory
```

## Building the Project

### Quick Build (Recommended for Development)

```bash
# From project root
./build.sh

# Or manually:
cmake -B build -S .
cmake --build build
```

### Clean Build

```bash
rm -rf build
cmake -B build -S .
cmake --build build
```

### Specific Build Targets

```bash
# Build only the main library
cmake --build build --target tank_sim_core

# Build only tests
cmake --build build --target tank_model_test
cmake --build build --target pid_controller_test

# Build Python bindings
cmake --build build --target tank_sim
```

### CMake Options

```bash
# Enable debug symbols
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug

# Disable tests
cmake -B build -S . -DBUILD_TESTING=OFF

# Specify GSL location
cmake -B build -S . -DGSL_ROOT_DIR=/usr/local
```

## Testing

### C++ Unit Tests

All C++ tests use GoogleTest framework.

```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test executable
./build/tests/test_tank_model --gtest_detail=all

# Run tests matching a pattern
ctest --test-dir build -R "PID" --output-on-failure

# Run with verbose output
ctest --test-dir build --output-on-failure -V
```

### Test Coverage

Each test file covers a specific C++ class:

| Test File | Covers | Status |
|-----------|--------|--------|
| `test_tank_model.cpp` | `TankModel` class | ✅ Complete (7 tests) |
| `test_pid_controller.cpp` | `PIDController` class | ✅ Complete (10 tests) |
| `test_stepper.cpp` | `Stepper` class | ✅ Complete (7 tests) |
| `test_simulator.cpp` | `Simulator` class | ✅ Complete (18 tests) |
| **C++ Total** | **All core classes** | **✅ Complete (42 tests)** |

### Python Unit Tests (Phase 2)

Python tests use pytest framework and verify the pybind11 bindings.

```bash
# Set up Python environment (one time)
uv venv
source .venv/bin/activate
uv pip install -e ".[dev]"

# Run all Python tests
pytest tests/python/ -v

# Run specific test class
pytest tests/python/test_simulator_bindings.py::TestSteadyStateStability -v

# Run with coverage report
pytest tests/python/ --cov=tank_sim --cov-report=html
```

### Python Test Coverage

Python tests verify that the C++ bindings work correctly:

| Test Category | File | Tests | Status |
|---------------|------|-------|--------|
| Configuration | `test_simulator_bindings.py` | 4 tests | ✅ Complete |
| Construction | `test_simulator_bindings.py` | 3 tests | ✅ Complete |
| Steady State | `test_simulator_bindings.py` | 1 test | ✅ Complete |
| Step Response | `test_simulator_bindings.py` | 2 tests | ✅ Complete |
| Disturbance | `test_simulator_bindings.py` | 2 tests | ✅ Complete |
| Reset | `test_simulator_bindings.py` | 2 tests | ✅ Complete |
| Exceptions | `test_simulator_bindings.py` | 2 tests | ✅ Complete |
| NumPy Arrays | `test_simulator_bindings.py` | 2 tests | ✅ Complete |
| Retuning | `test_simulator_bindings.py` | 1 test | ✅ Complete |
| Edge Cases | `test_simulator_bindings.py` | 7 tests | ✅ Complete |
| Integration | `test_simulator_bindings.py` | 2 tests | ✅ Complete |
| **Python Total** | **test_simulator_bindings.py** | **28 tests** | **✅ Complete** |

### Writing New Tests

Tests follow GoogleTest conventions:

```cpp
#include <gtest/gtest.h>
#include "../src/your_class.h"

using namespace tank_sim;

TEST(YourClassTest, DescribeWhatItTests) {
    // Arrange: Set up test data
    YourClass obj(initial_value);
    
    // Act: Execute the behavior
    double result = obj.someMethod(input);
    
    // Assert: Verify expected outcome
    EXPECT_NEAR(result, expected_value, tolerance);
}
```

Guidelines:
- Test one behavior per test function
- Use descriptive test names (TestClass_Behavior_ExpectedOutcome)
- Use `EXPECT_*` for non-fatal assertions, `ASSERT_*` for fatal
- Use `EXPECT_NEAR` for floating-point comparisons (always specify tolerance)
- Add comments explaining the expected values and why

## Python Bindings (Phase 2)

### Overview

The C++ simulation library is exposed to Python via pybind11 bindings. This allows Python code to use the high-performance C++ engine seamlessly with NumPy integration.

### Building Python Bindings

```bash
# One-time setup
uv venv
source .venv/bin/activate

# Install in development mode with dev dependencies
uv pip install -e ".[dev]"

# This builds C++ extension module and makes tank_sim importable
```

### Using the Python Bindings

```python
import tank_sim
import numpy as np

# Create simulator with default configuration
sim = tank_sim.Simulator(tank_sim.create_default_config())

# Run simulation loop
for step in range(100):
    sim.step()
    
    # Get current state as NumPy array
    state = sim.get_state()
    time = sim.get_time()
    level = state[0]
    
    print(f"t={time:.1f}s, level={level:.2f}m")

# Change control parameters
sim.set_setpoint(0, 3.0)  # New setpoint for controller 0

# Continue simulation
for step in range(200):
    sim.step()
```

### Core Classes and Structures

**SimulatorConfig** - Complete simulation configuration

```python
config = tank_sim.SimulatorConfig()
config.model_params = tank_sim.TankModelParameters(
    area=120.0,           # Cross-sectional area (m²)
    k_v=1.2649,          # Valve coefficient (m^2.5/s)
    max_height=5.0       # Maximum tank height (m)
)
config.controllers = [
    tank_sim.ControllerConfig(
        gains=tank_sim.PIDGains(Kc=1.0, tau_I=10.0, tau_D=2.0),
        bias=0.5,
        min_output=0.0,
        max_output=1.0,
        max_integral=10.0,
        measured_index=0,      # Tank level state
        output_index=1,        # Valve position input
        initial_setpoint=2.5
    )
]
config.initial_state = np.array([2.5])      # Starting level
config.initial_inputs = np.array([1.0, 0.5]) # Inlet flow, valve pos
config.dt = 1.0                              # 1 second timestep
```

**Simulator** - Main simulation interface

```python
sim = tank_sim.Simulator(config)

# Control methods
sim.step()                              # Advance simulation by dt
sim.reset()                             # Reset to initial conditions
sim.set_input(index, value)             # Set manual input
sim.set_setpoint(controller_idx, sp)    # Change controller setpoint
sim.set_controller_gains(idx, gains)    # Retune PID controller

# Query methods (all return NumPy arrays or floats)
state = sim.get_state()                 # Current state vector
inputs = sim.get_inputs()               # Current input vector
time = sim.get_time()                   # Current simulation time
setpoint = sim.get_setpoint(0)          # Controller setpoint
error = sim.get_error(0)                # Control error
output = sim.get_controller_output(0)   # Controller output
```

### Convenience Function

For quick prototyping, use the default configuration:

```python
# Equivalent to manually building the config above
sim = tank_sim.Simulator(tank_sim.create_default_config())

# This simulator starts at steady state:
# - Tank level at 2.5 m (50% full)
# - Inlet flow at 1.0 m³/s
# - Valve open to 50%
# - PID controller ready to regulate level
```

### Working with NumPy Arrays

The bindings automatically convert between C++ Eigen vectors and NumPy arrays:

```python
import numpy as np
import tank_sim

config = tank_sim.create_default_config()

# Set initial state using NumPy
config.initial_state = np.array([2.0])

# Get state as NumPy array (automatically converted from Eigen::VectorXd)
sim = tank_sim.Simulator(config)
state = sim.get_state()

# state is a numpy.ndarray with float64 dtype
print(type(state))        # <class 'numpy.ndarray'>
print(state.dtype)        # float64
print(state.shape)        # (1,)

# Can use all NumPy operations
state_rounded = np.round(state, 2)
state_abs = np.abs(state)
```

### Exception Handling

C++ exceptions are automatically converted to Python exceptions:

```python
import tank_sim

try:
    # Invalid configuration (empty state vector)
    config = tank_sim.SimulatorConfig()
    config.initial_state = np.array([])
    sim = tank_sim.Simulator(config)
except ValueError as e:
    print(f"Configuration error: {e}")

try:
    # Invalid controller index
    sim = tank_sim.Simulator(tank_sim.create_default_config())
    sim.get_setpoint(999)  # Only controller 0 exists
except IndexError as e:
    print(f"Index error: {e}")
```

### Advanced Usage

**Running multiple simulations in parallel:**

```python
import tank_sim
from concurrent.futures import ThreadPoolExecutor

def run_simulation(setpoint):
    config = tank_sim.create_default_config()
    sim = tank_sim.Simulator(config)
    
    sim.set_setpoint(0, setpoint)
    
    results = []
    for _ in range(100):
        sim.step()
        results.append(sim.get_state()[0])
    
    return results

# Test different setpoints
setpoints = [2.0, 2.5, 3.0, 3.5, 4.0]
with ThreadPoolExecutor(max_workers=5) as executor:
    all_results = executor.map(run_simulation, setpoints)
```

**Analyzing simulation results:**

```python
import tank_sim
import numpy as np

config = tank_sim.create_default_config()
sim = tank_sim.Simulator(config)

# Record data over time
times = []
levels = []
setpoints = []

# Run simulation with disturbance
for i in range(300):
    if i == 100:
        sim.set_setpoint(0, 3.0)  # Change setpoint
    if i == 200:
        sim.set_input(0, 1.2)     # Add disturbance
    
    sim.step()
    times.append(sim.get_time())
    levels.append(sim.get_state()[0])
    setpoints.append(sim.get_setpoint(0))

# Analyze results with NumPy
times = np.array(times)
levels = np.array(levels)
setpoints = np.array(setpoints)

error = levels - setpoints
settling_time = np.where(np.abs(error) < 0.05)[0]
print(f"Settling time: {settling_time[0] if len(settling_time) > 0 else 'N/A'} seconds")
```

## Code Organization

### Constants and Configuration

All numerical constants and configuration values are centralized in `src/constants.h`. This provides a single source of truth for system parameters and makes the codebase more maintainable.

**Constants are organized into logical groups:**

1. **System Architecture** - Fixed dimensions (state size, input size)
2. **Physical Parameters** - Tank properties, valve coefficients, limits
3. **Integration Parameters** - RK4 step size bounds, accuracy thresholds
4. **Control System** - PID gain defaults, output limits
5. **Numerical Tolerances** - Testing and validation tolerances
6. **Test-Specific Parameters** - Values used in unit tests
7. **Physics Constants** - Derived values like 2π

**Using constants in your code:**

```cpp
#include "constants.h"

using namespace tank_sim::constants;

// Use constants instead of magic numbers
if (dt < MIN_DT || dt > MAX_DT) {
    throw std::invalid_argument("Invalid time step");
}

// Access constants with full namespace qualification if needed
double area = tank_sim::constants::DEFAULT_TANK_AREA;
```

**Guidelines:**

- Never use magic numbers in implementation code
- Add new constants to `constants.h` (don't hardcode in source files)
- Update constant comments if the value or meaning changes
- Use `constexpr` for all constants (compile-time evaluation)
- Constants use `UPPER_SNAKE_CASE` with descriptive names
- Each constant includes detailed Doxygen documentation

**Example from tests:**

```cpp
// Before: Magic numbers scattered everywhere
TEST(TankModelTest, SteadyState) {
    TankModel model({120.0, 1.2649, 5.0});
    Eigen::VectorXd state(1);
    state << 2.5;
    Eigen::VectorXd inputs(2);
    inputs << 1.0, 0.5;
    EXPECT_NEAR(model.derivatives(state, inputs)(0), 0.0, 0.001);
}

// After: Clear intent with named constants
TEST(TankModelTest, SteadyState) {
    TankModel model({
        DEFAULT_TANK_AREA,
        DEFAULT_VALVE_COEFFICIENT,
        TANK_MAX_HEIGHT
    });
    Eigen::VectorXd state(1);
    state << TANK_NOMINAL_HEIGHT;
    Eigen::VectorXd inputs(2);
    inputs << TEST_INLET_FLOW, TEST_VALVE_POSITION;
    EXPECT_NEAR(
        model.derivatives(state, inputs)(0),
        0.0,
        TANK_STATE_TOLERANCE
    );
}
```

### Exception Handling Conventions

The project follows a consistent exception handling strategy to make error handling predictable for API users:

**Constructor Validation (Configuration Errors):**
- Use `std::invalid_argument` for invalid configuration parameters
- Thrown during object construction when setup is incorrect
- Examples: negative time step, invalid dimensions, out-of-bounds indices in config

```cpp
// Constructor validation example (Simulator)
if (dt <= 0.0 || dt < constants::MIN_DT || dt > constants::MAX_DT) {
    throw std::invalid_argument(
        "dt must be positive and between " + 
        std::to_string(constants::MIN_DT) + " and " + 
        std::to_string(constants::MAX_DT) + " seconds"
    );
}
```

**Runtime Validation (Index Errors):**
- Use `std::out_of_range` for index/bounds errors during normal operation
- Thrown by getter/setter methods when accessing invalid indices
- Examples: invalid controller index, setpoint index out of bounds

```cpp
// Runtime validation example (Simulator::getSetpoint)
if (index < 0 || static_cast<size_t>(index) >= setpoints.size()) {
    throw std::out_of_range(
        "Setpoint index " + std::to_string(index) +
        " out of bounds for " + std::to_string(setpoints.size()) +
        " controller(s)"
    );
}
```

**Numerical/GSL Errors:**
- Use `std::runtime_error` for computational failures
- Examples: GSL allocation failure, integration step failure

```cpp
// GSL error example (Stepper)
if (status != GSL_SUCCESS) {
    delete[] y;
    delete[] yerr;
    throw std::runtime_error("GSL RK4 step failed");
}
```

**Exception Safety Guidelines:**
- Always provide descriptive error messages with context
- Include relevant values in error messages for debugging
- Use RAII to ensure resources are cleaned up even if exceptions are thrown
- Document which exceptions each public method can throw

### Namespace and Naming

All simulation code lives in the `tank_sim` namespace:

```cpp
namespace tank_sim {
    class TankModel { /* ... */ };
    class PIDController { /* ... */ };
    class Stepper { /* ... */ };
}

// Constants live in tank_sim::constants
namespace tank_sim::constants {
    constexpr double DEFAULT_TANK_AREA = 120.0;
    // ...
}
```

### Class Design Principles

**Stateless Components (TankModel):**
- Pure computation - no internal state
- Same inputs → same outputs (every time)
- Safe for multi-threading
- Easy to test in isolation

**Stateful Components (PIDController):**
- Maintain necessary state (e.g., integral accumulator)
- Clear reset mechanism
- Documented state semantics
- Anti-windup for saturated controllers

**Wrappers (Stepper):**
- Thin abstractions around external libraries (GSL)
- Hide library-specific complexity
- Provide simple, clean interfaces
- Handle resource management (RAII)

### Documentation Standards

**Header Comments (Classes):**
```cpp
/**
 * @brief Brief description of the class.
 * 
 * More detailed explanation of what the class does, its purpose,
 * and design principles.
 * 
 * @note Important implementation details or caveats.
 */
```

**Method Comments:**
```cpp
/**
 * @brief What this method does.
 * 
 * @param param1 Description of parameter 1
 * @param param2 Description of parameter 2
 * @return Description of return value
 * 
 * @pre Preconditions (what must be true before calling)
 * @post Postconditions (what is guaranteed after calling)
 */
```

**Inline Comments:** Only where logic isn't self-evident.

## Development Workflow

### Getting Started with a Task

1. **Read the specification:** Check `docs/next.md` for the task description
2. **Read the design:** Review the corresponding class specification (e.g., `docs/Model Class.md`)
3. **Create a branch:** `git checkout -b feature/task-name`
4. **Implement:** Write code following the specification
5. **Test:** Ensure all tests pass
6. **Commit:** `git commit -m "Task N: Brief description"`

### Git Workflow

**Before starting work:**
```bash
git checkout main
git pull origin main
git checkout -b feature/your-feature
```

**During development:**
```bash
# Commit early and often
git add src/your_file.cpp
git commit -m "Task N: Description of what was done"

# Push to remote
git push origin feature/your-feature
```

**When done:**
```bash
# Create pull request on GitHub
# Link to any related issues
# Request review from code reviewer
```

**Commit Message Format:**
```
Task N: Brief one-line description

Longer explanation if needed. Include:
- What was changed
- Why it was changed
- Any important notes
```

### Code Review Process

1. **Self-review first:**
   - Run all tests locally: `ctest --test-dir build --output-on-failure`
   - Review your own changes: `git diff`
   - Check code style and comments

2. **Push and request review:**
   - Create a pull request on GitHub
   - Assign to code reviewer role
   - Add detailed PR description linking to specifications

3. **Address feedback:**
   - Make requested changes in new commits
   - Push and request re-review
   - Don't force-push if feedback is pending

### Phase Checkpoints

After completing major milestones, the Code Reviewer role reviews all work:

- ✅ **Phase 1 Complete**: All C++ classes implemented and tested
- ⏳ **Phase 2**: Python bindings working
- ⏳ **Phase 3**: FastAPI backend with WebSocket
- ⏳ **Phase 4+**: Frontend implementation and integration

See `docs/plan.md` for the complete phase breakdown.

## Debugging Tips

### Debugging C++ Code

**With GDB:**
```bash
# Build with debug symbols
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run a test with debugger
gdb ./build/tests/test_tank_model
(gdb) run
(gdb) bt  # backtrace
(gdb) print variable_name
(gdb) n   # next line
```

**With VSCode:**
1. Install "C/C++" extension by Microsoft
2. Create `.vscode/launch.json`:
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Run test",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/tests/test_tank_model",
            "args": ["--gtest_filter=TestName"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "preLaunchTask": "build",
            "MIMode": "gdb"
        }
    ]
}
```

### Common Issues

**CMake can't find GSL:**
```bash
# Check if GSL is installed
pkg-config --modversion gsl

# Specify location explicitly
cmake -B build -S . -DGSL_ROOT_DIR=/usr/local
```

**Compilation errors with C++17:**
```bash
# Ensure your compiler supports C++17
g++ --version  # Should be 9.0+
clang++ --version  # Should be 10.0+

# Or explicitly set C++ standard
cmake -B build -S . -DCMAKE_CXX_STANDARD=17
```

**Test failures with floating-point comparisons:**
- Use `EXPECT_NEAR(actual, expected, tolerance)` not `EXPECT_EQ`
- Choose tolerance based on relative error: `tolerance = expected * 1e-6` for 1 ppm

**Memory issues:**
```bash
# Run tests with valgrind
valgrind --leak-check=full ./build/tests/test_tank_model

# Or with AddressSanitizer
cmake -B build -S . -DCMAKE_CXX_FLAGS="-fsanitize=address"
```

## Common Tasks

### Adding a New Class

1. **Create header file** (`src/myclass.h`):
   - Document with Doxygen comments
   - Follow existing class structure (public methods, private members)

2. **Create implementation** (`src/myclass.cpp`):
   - Include guards already in header
   - Implement methods from specification

3. **Update CMakeLists.txt**:
   - Add `.cpp` file to source list in `src/CMakeLists.txt`

4. **Write tests** (`tests/test_myclass.cpp`):
   - Create GoogleTest file
   - Add to `tests/CMakeLists.txt`

### Running a Single Test

```bash
# Run all tests in a file
./build/tests/test_tank_model --gtest_filter="*"

# Run a specific test
./build/tests/test_tank_model --gtest_filter="TankModelTest.TestName"

# Run all tests matching a pattern
./build/tests/test_tank_model --gtest_filter="*Steady*"
```

### Checking Code Style

While there's no automated style checker configured yet, follow these guidelines:

- **Indentation:** 4 spaces (no tabs)
- **Line length:** Aim for ≤100 characters
- **Naming:** 
  - Classes: `PascalCase`
  - Methods: `camelCase`
  - Members: `snake_case_`
  - Constants: `UPPER_CASE`
- **Braces:** Allman style (opening brace on same line)

### Updating Documentation

**Class specifications:**
- Edit the corresponding `.md` file in `docs/`
- Include interface, design decisions, and equations

**Developer guide (this file):**
- Update when adding new procedures or requirements
- Keep examples working and tested

**API documentation:**
- Will be generated from Doxygen comments in code
- Keep comments accurate as code evolves

### Adding Dependencies

Before adding a new library:

1. Check if already available in package manager
2. Add to `CMakeLists.txt` using `FetchContent` (preferred) or `find_package`
3. Link to targets in `src/CMakeLists.txt`
4. Update `README.md` prerequisites if user must install manually
5. Document why the dependency was needed

Current dependencies:
- **Eigen3**: Linear algebra (header-only, FetchContent)
- **GSL**: ODE solver (external, must be installed)
- **GoogleTest**: Unit testing (FetchContent)

## Error Handling Policy

This project uses a consistent error handling strategy across all C++ components.

### Constructor Validation (Throw Exceptions)

**Rule:** Constructor parameters are validated immediately. Invalid parameters throw exceptions.

**Why:** Constructors should fail fast and clearly. An object in an invalid state is worse than no object at all.

**Example:**
```cpp
TankModel::TankModel(const Parameters& params) {
    if (params.area <= 0.0) {
        throw std::invalid_argument("Tank area must be positive");
    }
    if (params.k_v <= 0.0) {
        throw std::invalid_argument("Valve coefficient must be positive");
    }
    // ...
}
```

### Runtime Preconditions (Debug Assertions, Critical Paths Checked)

**Rule:** Method preconditions are checked with assertions in debug builds. Critical paths may also check at runtime.

**Why:**
- Assertions help catch programming errors during development
- Critical paths (e.g., state dimension validation) check even in release builds
- Assertions disappear in release builds (`NDEBUG` flag)

**Example:**
```cpp
Eigen::VectorXd TankModel::derivatives(
    const Eigen::VectorXd& state,
    const Eigen::VectorXd& inputs) const {
    
    // Debug-only assertions (disappear in release)
    assert(state.size() == 1 && "State vector must have size 1");
    assert(inputs.size() == 2 && "Input vector must have size 2");
    
    // Critical runtime check (always present)
    if (state.size() != 1) {
        throw std::runtime_error("State vector size must be 1");
    }
    // ...
}
```

### External Library Errors (Check and Throw)

**Rule:** Errors from external libraries (GSL, Eigen) are checked and converted to meaningful exceptions.

**Why:** Callers should understand what went wrong in terms of our domain, not GSL's internals.

**Example:**
```cpp
Stepper::Stepper(size_t state_dimension) : state_dimension_(state_dimension) {
    stepper_ = gsl_odeiv2_step_alloc(gsl_odeiv2_step_rk4, state_dimension);
    if (stepper_ == nullptr) {
        throw std::runtime_error("Failed to allocate GSL stepper");
    }
}
```

### Documentation

Use Doxygen `@throws` tags to document exceptions:

```cpp
/**
 * @brief Compute the derivative of tank level.
 * 
 * @param state Current state vector [h]
 * @param inputs Input vector [q_in, x]
 * @return Derivative vector [dh/dt]
 * 
 * @throws std::runtime_error if state or input size is invalid
 */
Eigen::VectorXd derivatives(
    const Eigen::VectorXd& state,
    const Eigen::VectorXd& inputs) const;
```

### Summary Table

| Scenario | Approach | Example |
|----------|----------|---------|
| Invalid constructor parameter | Throw exception | `throw std::invalid_argument(...)` |
| Invalid method precondition | Assert (debug) + check critical | `assert(...); if (...) throw;` |
| External library error | Check result, throw with context | `if (gsl_result != GSL_SUCCESS) throw;` |
| Resource allocation failure | Throw exception | `if (ptr == nullptr) throw;` |

---

## Control System Design

### Direct-Acting vs. Reverse-Acting Control

**CRITICAL LESSON LEARNED:** One of the most common pitfalls in process control is getting the control action direction wrong. This section documents a bug discovered during Task 9 testing that caused the controller to push the system in the wrong direction.

#### The Problem

When testing the Simulator with step response tests, the following incorrect behavior was observed:
- Setpoint increased from 2.5 m → 3.0 m, but tank level **decreased** to 1.3 m
- Setpoint decreased from 2.5 m → 2.0 m, but tank level **increased** to 4.2 m

The system was responding exactly backwards!

#### Root Cause Analysis

**The Tank Level Control Loop:**
```
                    ┌─────────────────────────────────────────────┐
                    │                                             │
   Setpoint ───►(+)─┴─►[PID Controller]───► Valve Position ───►[Tank]───► Level
                (-)                              │                         │
                 ▲                               │                         │
                 │                               ▼                         │
                 │                          More Open                      │
                 │                               │                         │
                 │                               ▼                         │
                 │                         More Outlet Flow                │
                 │                               │                         │
                 │                               ▼                         │
                 │                         Level DECREASES                 │
                 │                                                         │
                 └─────────────────────────────────────────────────────────┘
```

**Analysis:**
1. Valve position controls outlet flow: `q_out = k_v × valve_position × √h`
2. **Opening the valve (higher position) → more outlet flow → level DECREASES**
3. **Closing the valve (lower position) → less outlet flow → level INCREASES**

**With standard (direct-acting) control (Kc > 0):**
- Level below setpoint → positive error
- Positive error × positive Kc → positive output change → valve opens more
- Valve opens → more outlet → level drops further → **UNSTABLE!**

**This is a reverse-acting control loop.** The manipulated variable (valve) has an inverse relationship with the controlled variable (level).

#### The Fix

Use **negative proportional gain** (Kc < 0) for reverse-acting control:

```cpp
// WRONG - Direct-acting control (destabilizes this system)
ctrl_config.gains = PIDController::Gains{
    1.0,   // Kc: POSITIVE - pushes system in wrong direction!
    10.0,  // tau_I
    0.0    // tau_D
};

// CORRECT - Reverse-acting control
ctrl_config.gains = PIDController::Gains{
    -1.0,  // Kc: NEGATIVE - proper reverse-acting control
    10.0,  // tau_I
    0.0    // tau_D
};
```

**With reverse-acting control (Kc < 0):**
- Level below setpoint → positive error
- Positive error × negative Kc → **negative** output change → valve closes
- Valve closes → less outlet → level rises toward setpoint → **STABLE!**

#### How to Determine Control Action Direction

For any control loop, trace the signal path and count the inversions:

| Step | Signal Change | Inversion? |
|------|--------------|------------|
| Error increases (+) | → | — |
| PID output | → | Kc sign determines |
| Valve position increases | → | — |
| Outlet flow increases | → | — |
| Tank level **decreases** | ← | **YES - INVERSION** |

**Rule:** If there is an **odd number of inversions** in the loop, use Kc < 0 (reverse-acting).

#### Common Process Control Examples

| Process | Manipulated Variable | Controlled Variable | Action |
|---------|---------------------|--------------------:|--------|
| Tank level via outlet valve | Valve position | Level | **Reverse** (Kc < 0) |
| Tank level via inlet valve | Valve position | Level | Direct (Kc > 0) |
| Temperature via cooling | Coolant flow | Temperature | **Reverse** (Kc < 0) |
| Temperature via heating | Heater power | Temperature | Direct (Kc > 0) |
| Pressure via vent valve | Valve position | Pressure | **Reverse** (Kc < 0) |

#### Testing for Correct Control Action

Always include tests that verify the control **direction** is correct:

```cpp
// Test that level INCREASES when setpoint increases
TEST_F(SimulatorTest, StepResponseLevelIncrease) {
    // ... setup at steady state ...
    sim.setSetpoint(0, 3.0);  // Increase setpoint
    // ... run simulation ...
    
    // Level should have INCREASED (not decreased!)
    EXPECT_GT(state(0), TANK_NOMINAL_HEIGHT);
    
    // Valve should have CLOSED (not opened!)
    EXPECT_LT(inputs(1), TEST_VALVE_POSITION);
}
```

**WARNING:** If tests are failing because the system moves in the wrong direction, do NOT weaken the tests to make them pass. Fix the control action sign!

#### Lesson for Future Projects

When building control systems on this foundation:

1. **Always analyze the control loop** before writing code
2. **Document the expected control action** (direct or reverse) in specifications
3. **Write tests that verify direction**, not just magnitude
4. **If system goes wrong direction**, check Kc sign first
5. **Never "fix" failing tests** by making assertions less specific

This lesson applies to any project using PID control, not just tank level systems.

---

## Performance Considerations

### Profiling

The simulation must run at 1 Hz real-time. To verify performance:

```bash
# Time a simulation run
time ./build/tests/test_simulator

# Profile with perf (Linux)
perf record ./build/tests/test_simulator
perf report
```

### Optimization Flags

Release builds use `-O3` by default:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

For development, use Debug mode for faster compilation:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
```

## Next Steps

1. If implementing a new class, follow "Adding a New Class" above
2. Check `docs/next.md` for the next task
3. Read the task specification in `docs/next.md`
4. Read the detailed class specification (e.g., `docs/Stepper Class.md`)
5. Create a branch and start implementing

For questions about architecture or design decisions, see `docs/plan.md`.

---

**Last Updated:** 2026-02-04  
**For:** Phase 1 - C++ Simulation Core with Constants Management
