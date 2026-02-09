# Constants Architecture Guide

**Purpose:** Explain the organization and usage of the centralized constants system in tank_dynamics.

## Overview

The tank_dynamics project uses a centralized constants management system in `src/constants.h` to:

1. **Eliminate magic numbers** - No hardcoded values in source code
2. **Provide single source of truth** - Change once, takes effect everywhere
3. **Self-document code** - Named constants clarify intent
4. **Enable configuration** - Easy to adjust parameters without recompilation (future feature)

## Constants Organization

### Hierarchy

```
tank_sim::constants namespace
├── System Architecture (4)
│   ├── TANK_STATE_SIZE = 1
│   ├── TANK_INPUT_SIZE = 2
│   ├── INPUT_INDEX_INLET_FLOW = 0
│   └── INPUT_INDEX_VALVE_POSITION = 1
│
├── Physical Parameters (5)
│   ├── DEFAULT_TANK_AREA = 120.0
│   ├── DEFAULT_VALVE_COEFFICIENT = 1.2649
│   ├── TANK_MAX_HEIGHT = 5.0
│   ├── TANK_NOMINAL_HEIGHT = 2.5
│   └── GRAVITY = 9.81
│
├── Integration Parameters (6)
│   ├── MIN_DT = 0.001
│   ├── MAX_DT = 10.0
│   ├── DEFAULT_DT = 0.1
│   ├── RK4_MIN_ERROR_RATIO = 12.0
│   └── RK4_MAX_ERROR_RATIO = 20.0
│
├── Control System (10)
│   ├── DEFAULT_PID_PROPORTIONAL_GAIN = 1.0
│   ├── DEFAULT_PID_INTEGRAL_TIME = 10.0
│   ├── DEFAULT_PID_DERIVATIVE_TIME = 5.0
│   ├── DEFAULT_PID_BIAS = 0.5
│   ├── DEFAULT_PID_MIN_OUTPUT = 0.0
│   ├── DEFAULT_PID_MAX_OUTPUT = 1.0
│   ├── DEFAULT_PID_MAX_INTEGRAL = 10.0
│   └── DEFAULT_PID_DT = 1.0
│
├── Numerical Tolerances (6)
│   ├── DERIVATIVE_TOLERANCE = 0.001
│   ├── INTEGRATION_TOLERANCE = 0.0001
│   ├── OSCILLATOR_POSITION_TOLERANCE = 0.001
│   ├── OSCILLATOR_VELOCITY_TOLERANCE = 0.01
│   ├── TANK_STATE_TOLERANCE = 0.001
│   └── CONTROL_OUTPUT_TOLERANCE = 0.001
│
├── Test Parameters (10)
│   ├── TEST_ERROR_VALUE = 0.1
│   ├── TEST_DT = 1.0
│   ├── TEST_INLET_FLOW = 1.0
│   ├── TEST_VALVE_POSITION = 0.5
│   ├── TEST_OSCILLATOR_FREQUENCY = 1.0
│   ├── TEST_RK4_DT_COARSE = 0.1
│   ├── TEST_RK4_DT_FINE = 0.05
│   ├── TEST_NUM_STEPS = 10
│   └── TEST_NUM_STEPS_FINE = 20
│
└── Physics (1)
    └── TWO_PI = 6.283185307179586
```

## Constants by Purpose

### 1. System Dimensions (Never Change)

These define the fixed architecture of the simulation and should never change without major refactoring.

```cpp
constexpr int TANK_STATE_SIZE = 1;              // Single tank level
constexpr int TANK_INPUT_SIZE = 2;              // Inlet flow + valve position
constexpr int INPUT_INDEX_INLET_FLOW = 0;
constexpr int INPUT_INDEX_VALVE_POSITION = 1;
```

**Used in:**
- `simulator.cpp` - Constructor validation
- Test fixtures - State/input vector setup

**Never should be:**
- Hardcoded in vector initialization
- Used in array literals without constant reference
- Changed without updating multiple classes

### 2. Physical Parameters (Site/Configuration Specific)

These represent the actual tank and valve characteristics. May vary per installation.

```cpp
constexpr double DEFAULT_TANK_AREA = 120.0;         // m²
constexpr double DEFAULT_VALVE_COEFFICIENT = 1.2649; // m^2.5/s
constexpr double TANK_MAX_HEIGHT = 5.0;            // m
constexpr double TANK_NOMINAL_HEIGHT = 2.5;        // m
constexpr double GRAVITY = 9.81;                   // m/s²
```

**Physics:**
```
Material balance: dh/dt = (q_in - q_out) / area
Outlet flow: q_out = k_v * x * sqrt(h)
Steady-state: q_in = k_v * x * sqrt(h)
```

**Steady-state with defaults:**
```
q_in = 1.0 m³/s, x = 0.5, h = 2.5 m
q_out = 1.2649 * 0.5 * √2.5 ≈ 1.0 m³/s
dh/dt ≈ 0 (balanced)
```

**Used in:**
- `tank_model.cpp` - Physics calculations
- Test fixtures - Model initialization
- Simulator - Parameter configuration

**Future use:**
- Configuration files (JSON/YAML)
- HTTP endpoints for parameter querying
- Real-world commissioning data

### 3. Integration Parameters (RK4 Tuning)

These control the numerical integration method and verify its accuracy.

```cpp
constexpr double MIN_DT = 0.001;               // Can't go below 1 ms
constexpr double MAX_DT = 10.0;                // Can't exceed 10 seconds
constexpr double DEFAULT_DT = 0.1;             // 100 ms typical
constexpr double RK4_MIN_ERROR_RATIO = 12.0;  // Lower bound for 4th order
constexpr double RK4_MAX_ERROR_RATIO = 20.0;  // Upper bound for 4th order
```

**Convergence verification:**
```
For RK4, error scales as (dt)^4
Coarse: dt_c = 0.1, Fine: dt_f = 0.05
Expected ratio: (0.1/0.05)^4 = 2^4 = 16
Valid range: [12, 20] (allows numerical noise)
```

**Used in:**
- `simulator.cpp` - dt validation
- `test_stepper.cpp` - Accuracy convergence tests

**Guidelines:**
- Minimum dt prevents floating-point precision loss
- Maximum dt prevents instability and inaccuracy
- Default dt chosen for ~1 Hz real-time performance

### 4. Control System Defaults (PID Tuning)

These are default PID parameters. Not optimal for all systems, but reasonable starting points.

```cpp
constexpr double DEFAULT_PID_PROPORTIONAL_GAIN = 1.0;  // Kc
constexpr double DEFAULT_PID_INTEGRAL_TIME = 10.0;     // tau_I (seconds)
constexpr double DEFAULT_PID_DERIVATIVE_TIME = 5.0;    // tau_D (seconds)
constexpr double DEFAULT_PID_BIAS = 0.5;               // Nominal setpoint
constexpr double DEFAULT_PID_MIN_OUTPUT = 0.0;         // Lower saturation
constexpr double DEFAULT_PID_MAX_OUTPUT = 1.0;         // Upper saturation
constexpr double DEFAULT_PID_MAX_INTEGRAL = 10.0;      // Anti-windup limit
constexpr double DEFAULT_PID_DT = 1.0;                 // Update interval
```

**PID equation:**
```
u = bias + Kc * (error + (Kc/tau_I) * ∫error + Kc*tau_D * de/dt)
u_saturated = clamp(u, MIN, MAX)
```

**Typical tuning:**
- Level control: Kc=1.0-2.0, tau_I=5-20s, tau_D=0-5s
- Fast response: Lower tau_I, higher Kc
- Smooth response: Higher tau_I, lower Kc

**Anti-windup:**
- Integral only accumulates when output NOT saturated
- Prevents integrator windup during constraint violation
- Max integral clamps accumulated error to 10.0

**Used in:**
- `pid_controller.cpp` - Default gain values
- Test fixtures - Controller initialization

### 5. Numerical Tolerances (Testing Only)

These are used ONLY in GoogleTest assertions to allow for floating-point rounding.

```cpp
constexpr double DERIVATIVE_TOLERANCE = 0.001;
constexpr double INTEGRATION_TOLERANCE = 0.0001;
constexpr double OSCILLATOR_POSITION_TOLERANCE = 0.001;
constexpr double OSCILLATOR_VELOCITY_TOLERANCE = 0.01;
constexpr double TANK_STATE_TOLERANCE = 0.001;
constexpr double CONTROL_OUTPUT_TOLERANCE = 0.001;
```

**Never use for:**
- Runtime validation
- Production code decision-making
- Saturation limits

**Usage pattern:**
```cpp
EXPECT_NEAR(computed, expected, TANK_STATE_TOLERANCE);
EXPECT_NEAR(output, setpoint, CONTROL_OUTPUT_TOLERANCE);
```

**Why different tolerances:**
- `INTEGRATION_TOLERANCE` tighter (0.0001) - comparing against analytical solution
- `OSCILLATOR_VELOCITY_TOLERANCE` larger (0.01) - derivatives more sensitive to errors
- `TANK_STATE_TOLERANCE` moderate (0.001) - 0.1% of nominal height

### 6. Test Parameters (Test Data)

These define standard values used across all tests. Ensures consistency.

```cpp
constexpr double TEST_ERROR_VALUE = 0.1;           // m (4% of nominal)
constexpr double TEST_DT = 1.0;                    // seconds
constexpr double TEST_INLET_FLOW = 1.0;            // m³/s
constexpr double TEST_VALVE_POSITION = 0.5;        // 50% open
constexpr double TEST_OSCILLATOR_FREQUENCY = 1.0;  // Hz
constexpr double TEST_RK4_DT_COARSE = 0.1;         // seconds
constexpr double TEST_RK4_DT_FINE = 0.05;          // seconds
constexpr int TEST_NUM_STEPS = 10;                 // 10 * 0.1s = 1.0s
constexpr int TEST_NUM_STEPS_FINE = 20;            // 20 * 0.05s = 1.0s
```

**Rationale for values:**
- `TEST_ERROR_VALUE = 0.1 m` - 4% of nominal 2.5m = realistic error
- `TEST_INLET_FLOW = 1.0 m³/s` - Matches steady-state at nominal conditions
- `TEST_VALVE_POSITION = 0.5` - Represents 50% valve opening (typical operation)
- RK4 step ratio 2:1 (0.1 vs 0.05) verifies fourth-order accuracy

**Usage pattern:**
```cpp
// Before
Eigen::VectorXd state(1);
state << 2.5;
double dt = 0.1;

// After (better)
Eigen::VectorXd state(1);
state << TANK_NOMINAL_HEIGHT;
double dt = TEST_RK4_DT_COARSE;
```

## Usage Patterns

### Pattern 1: Basic Constant Use

```cpp
#include "constants.h"
using namespace tank_sim::constants;

// Validate input
if (dt < MIN_DT || dt > MAX_DT) {
    throw std::invalid_argument("Invalid dt");
}

// Create model with defaults
TankModel::Parameters params{
    DEFAULT_TANK_AREA,
    DEFAULT_VALVE_COEFFICIENT,
    TANK_MAX_HEIGHT
};
```

### Pattern 2: Test Fixture Setup

```cpp
class TankModelTest : public ::testing::Test {
protected:
    TankModel model{{
        DEFAULT_TANK_AREA,
        DEFAULT_VALVE_COEFFICIENT,
        TANK_MAX_HEIGHT
    }};
};

TEST_F(TankModelTest, SteadyState) {
    Eigen::VectorXd state(1);
    state << TANK_NOMINAL_HEIGHT;
    
    Eigen::VectorXd inputs(2);
    inputs << TEST_INLET_FLOW, TEST_VALVE_POSITION;
    
    auto deriv = model.derivatives(state, inputs);
    EXPECT_NEAR(deriv(0), 0.0, TANK_STATE_TOLERANCE);
}
```

### Pattern 3: Explicit Namespace

```cpp
// When not using namespace declaration
double area = tank_sim::constants::DEFAULT_TANK_AREA;
double k_v = tank_sim::constants::DEFAULT_VALVE_COEFFICIENT;
```

### Pattern 4: Derived Calculations

```cpp
// Calculate steady-state based on constants
double q_out = tank_sim::constants::DEFAULT_VALVE_COEFFICIENT * 
               tank_sim::constants::TEST_VALVE_POSITION * 
               std::sqrt(tank_sim::constants::TANK_NOMINAL_HEIGHT);
```

## Guidelines for Constants

### DO ✅

1. **Add constants to constants.h**
   - Not in source files or tests
   - Document with Doxygen comments
   - Include units, purpose, and usage

2. **Use UPPER_SNAKE_CASE**
   - Clearly distinguishes from variables
   - Conventional C++ style
   - Easy to grep

3. **Group related constants**
   - Keep comments organized
   - Use section headers
   - Cross-reference related values

4. **Document thoroughly**
   - What is this constant?
   - What units?
   - Where is it used?
   - What's a typical value?

5. **Test with related constants**
   - Use TEST_* values in tests
   - Ensures consistency
   - Easier to tune all at once

### DON'T ❌

1. **Don't hardcode numbers**
   ```cpp
   // Bad
   if (dt < 0.001) throw std::invalid_argument("dt too small");
   
   // Good
   if (dt < MIN_DT) throw std::invalid_argument("dt too small");
   ```

2. **Don't use undefined magic numbers**
   ```cpp
   // Bad
   double output = 0.5 + 1.0 * error;  // What are 0.5 and 1.0?
   
   // Good
   double output = DEFAULT_PID_BIAS + 
                   DEFAULT_PID_PROPORTIONAL_GAIN * error;
   ```

3. **Don't mix test and production constants**
   ```cpp
   // Bad
   const double TEST_HEIGHT = 2.5;  // Should use TANK_NOMINAL_HEIGHT
   
   // Good
   state << TANK_NOMINAL_HEIGHT;  // Shared constant
   ```

4. **Don't change constants without documentation**
   - Update comments explaining why
   - Consider impact on other code
   - Update tests if needed

5. **Don't create constants for one-time use**
   ```cpp
   // Unnecessary
   constexpr double CALCULATION_FACTOR = 1.5;
   double result = value * CALCULATION_FACTOR;
   
   // Better: Use inline or explain in comment
   double result = value * 1.5;  // Scale factor for sensitivity
   ```

## Maintenance

### When to Update Constants

1. **Physical system changes**
   - Tank area? Update `DEFAULT_TANK_AREA`
   - Different valve? Update `DEFAULT_VALVE_COEFFICIENT`

2. **Performance tuning**
   - RK4 accuracy issues? Adjust `MIN_DT` / `MAX_DT`
   - Faster control? Lower `DEFAULT_PID_DT`

3. **Test improvements**
   - New test scenario? Add `TEST_*` constants
   - New tolerance level? Update tolerance constants

### Adding New Constants

1. **Identify category**
   - System Architecture
   - Physical Parameters
   - Integration Parameters
   - Control System
   - Tolerances
   - Test Parameters
   - Physics

2. **Choose name** (UPPER_SNAKE_CASE)
   - Descriptive and unambiguous
   - Include unit information if not obvious
   - Prefix with TEST_ for test-only values

3. **Add value and comments**
   ```cpp
   /**
    * @brief What this is
    * 
    * More detailed explanation.
    * Unit: meters
    * Typical value: 2.5-5.0 m
    * Used in: TankModel, tests
    */
   constexpr double MY_CONSTANT = 3.5;
   ```

4. **Update documentation**
   - Add to relevant sections in DEVELOPER_GUIDE.md
   - Add to API_REFERENCE.md if user-facing
   - Update this file if architectural

## Future Enhancements

### Configuration File Support

```json
{
  "tank": {
    "area": 120.0,
    "k_v": 1.2649,
    "max_height": 5.0,
    "nominal_height": 2.5
  },
  "control": {
    "pid_kc": 1.0,
    "pid_tau_i": 10.0,
    "pid_tau_d": 5.0
  }
}
```

### REST API Endpoints

```
GET  /api/constants/tank          → Tank parameters
GET  /api/constants/control       → Control defaults
POST /api/constants/tank          → Update tank parameters
GET  /api/constants/limits/dt     → Integration bounds
```

### Runtime Tuning

```cpp
// Future: Load constants from config at runtime
ConfigFile config("tank_dynamics.json");
TankModel::Parameters params = config.getTankParameters();
```

## Related Documentation

- `src/constants.h` - Actual constant definitions
- `docs/DEVELOPER_GUIDE.md` - Usage guidelines
- `docs/API_REFERENCE.md` - Complete constant reference
- `docs/plan.md` - System architecture
- `docs/TankDynamics.md` - Physics equations

---

**Last Updated:** 2026-02-04  
**Status:** Production-Ready  
**Maintainer:** Development Team
