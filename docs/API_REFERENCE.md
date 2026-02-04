# Tank Dynamics API Reference

Complete API documentation for the Tank Dynamics Simulator C++ library.

## Table of Contents

1. [Constants and Configuration](#constants-and-configuration)
2. [TankModel Class](#tankmodel-class)
3. [PIDController Class](#pidcontroller-class)
4. [Stepper Class](#stepper-class)
5. [Simulator Class (Planned)](#simulator-class-planned)
6. [Type Definitions](#type-definitions)
7. [Example Usage](#example-usage)

## Constants and Configuration

All numerical constants and configuration values are centralized in `src/constants.h`. Using constants instead of magic numbers improves maintainability and makes the code self-documenting.

### Include

```cpp
#include "constants.h"
using namespace tank_sim::constants;
```

### System Architecture Constants

**State and Input Dimensions:**

```cpp
constexpr int TANK_STATE_SIZE = 1;           // Single state: tank level [h]
constexpr int TANK_INPUT_SIZE = 2;           // Two inputs: [q_in, x]
constexpr int INPUT_INDEX_INLET_FLOW = 0;    // Index for inlet flow in input vector
constexpr int INPUT_INDEX_VALVE_POSITION = 1; // Index for valve position in input vector
```

### Physical Parameters

**Tank and Valve Properties:**

```cpp
constexpr double DEFAULT_TANK_AREA = 120.0;              // m²
constexpr double DEFAULT_VALVE_COEFFICIENT = 1.2649;     // m^2.5/s
constexpr double TANK_MAX_HEIGHT = 5.0;                  // m
constexpr double TANK_NOMINAL_HEIGHT = 2.5;              // m (nominal operating point)
constexpr double GRAVITY = 9.81;                         // m/s² (gravitational acceleration)
```

**Typical steady-state at nominal conditions:**
```
With q_in = 1.0 m³/s, x = 0.5, h = 2.5 m:
q_out = 1.2649 * 0.5 * √2.5 ≈ 1.0 m³/s (balanced)
dh/dt ≈ 0.0 (steady state)
```

### Integration Parameters

**RK4 Time Step Bounds:**

```cpp
constexpr double MIN_DT = 0.001;               // Minimum time step (seconds)
constexpr double MAX_DT = 10.0;                // Maximum time step (seconds)
constexpr double DEFAULT_DT = 0.1;             // Recommended step size (seconds)
```

**RK4 Convergence Validation:**

```cpp
constexpr double RK4_MIN_ERROR_RATIO = 12.0;  // Fourth-order accuracy lower bound
constexpr double RK4_MAX_ERROR_RATIO = 20.0;  // Fourth-order accuracy upper bound
```

When verifying RK4 convergence, the error ratio between coarse and fine integration should be:
```
error_ratio = error_coarse / error_fine ≈ (dt_coarse / dt_fine)^4 ∈ [12, 20]
```

### Control System Defaults

**PID Gains and Limits:**

```cpp
constexpr double DEFAULT_PID_PROPORTIONAL_GAIN = 1.0;   // Kc
constexpr double DEFAULT_PID_INTEGRAL_TIME = 10.0;      // tau_I (seconds)
constexpr double DEFAULT_PID_DERIVATIVE_TIME = 5.0;     // tau_D (seconds)
constexpr double DEFAULT_PID_BIAS = 0.5;                // Nominal operating point
constexpr double DEFAULT_PID_MIN_OUTPUT = 0.0;          // Saturation lower bound
constexpr double DEFAULT_PID_MAX_OUTPUT = 1.0;          // Saturation upper bound
constexpr double DEFAULT_PID_MAX_INTEGRAL = 10.0;       // Anti-windup limit
constexpr double DEFAULT_PID_DT = 1.0;                  // Default control update interval
```

**Typical Tuning:**
- Level control: Kc=1.0, tau_I=10s, tau_D=2-5s
- Fast response: Lower tau_I, higher Kc
- Smooth response: Higher tau_I, lower Kc

### Numerical Tolerances

**Testing and Validation Tolerances:**

```cpp
constexpr double DERIVATIVE_TOLERANCE = 0.001;              // For derivative checks
constexpr double INTEGRATION_TOLERANCE = 0.0001;            // For integration accuracy
constexpr double OSCILLATOR_POSITION_TOLERANCE = 0.001;     // For harmonic oscillator position
constexpr double OSCILLATOR_VELOCITY_TOLERANCE = 0.01;      // For harmonic oscillator velocity
constexpr double TANK_STATE_TOLERANCE = 0.001;              // For tank level validation (m)
constexpr double CONTROL_OUTPUT_TOLERANCE = 0.001;          // For control signal validation
```

**Usage in Tests:**
```cpp
EXPECT_NEAR(computed_value, expected_value, TANK_STATE_TOLERANCE);
EXPECT_NEAR(output, expected_output, CONTROL_OUTPUT_TOLERANCE);
```

### Test-Specific Parameters

**Standard Test Values:**

```cpp
constexpr double TEST_ERROR_VALUE = 0.1;                // Standard error for PID tests (m)
constexpr double TEST_DT = 1.0;                         // Standard time step (seconds)
constexpr double TEST_INLET_FLOW = 1.0;                 // Standard inlet flow (m³/s)
constexpr double TEST_VALVE_POSITION = 0.5;             // Standard valve position [0,1]
constexpr double TEST_OSCILLATOR_FREQUENCY = 1.0;       // Harmonic test frequency (Hz)
constexpr double TEST_RK4_DT_COARSE = 0.1;             // Coarse step for convergence test (seconds)
constexpr double TEST_RK4_DT_FINE = 0.05;              // Fine step for convergence test (seconds)
constexpr int TEST_NUM_STEPS = 10;                      // Standard step count for tests
constexpr int TEST_NUM_STEPS_FINE = 20;                // Fine step count (2x NUM_STEPS)
```

**Why These Values:**
- `TEST_ERROR_VALUE = 0.1 m` provides realistic tank level errors (~4% of 2.5m nominal)
- `TEST_INLET_FLOW = 1.0 m³/s` matches steady-state inlet at nominal conditions
- `TEST_VALVE_POSITION = 0.5` represents 50% valve opening
- RK4 step ratio of 2:1 (0.1 vs 0.05) verifies fourth-order accuracy

### Physics Constants

**Derived Constants:**

```cpp
constexpr double TWO_PI = 6.283185307179586;  // 2π for angular frequency calculations
```

Used in oscillatory system tests:
```cpp
double omega = TWO_PI * TEST_OSCILLATOR_FREQUENCY;  // rad/s
```

---

## TankModel Class

Physics model for a tank with inlet/outlet flows. Stateless - computes derivatives suitable for numerical integration.

### Include

```cpp
#include "tank_model.h"
using namespace tank_sim;
```

### Class Definition

```cpp
class TankModel {
public:
    struct Parameters { /* ... */ };
    explicit TankModel(const Parameters& params);
    Eigen::VectorXd derivatives(
        const Eigen::VectorXd& state,
        const Eigen::VectorXd& inputs) const;
    double getOutletFlow(
        const Eigen::VectorXd& state,
        const Eigen::VectorXd& inputs) const;
};
```

### Nested Type: Parameters

Configuration structure for tank physics.

**Members:**
- `double area` - Cross-sectional area of tank in m²
- `double k_v` - Valve coefficient in m^2.5/s
- `double max_height` - Maximum tank height in m

**Example:**
```cpp
TankModel::Parameters params{
    .area = 120.0,      // m²
    .k_v = 1.2649,      // m^2.5/s
    .max_height = 5.0   // m
};
```

### Constructor

```cpp
explicit TankModel(const Parameters& params);
```

Constructs a TankModel with physical parameters.

**Parameters:**
- `params` - Configuration structure containing tank dimensions and valve coefficient

**Throws:** Nothing

**Example:**
```cpp
TankModel::Parameters params{120.0, 1.2649, 5.0};
TankModel model(params);
```

### Method: derivatives

```cpp
Eigen::VectorXd derivatives(
    const Eigen::VectorXd& state,
    const Eigen::VectorXd& inputs) const;
```

Computes the derivative of tank level (dh/dt) based on current state and inputs.

**Parameters:**
- `state` - Current state vector `[h]` where h is tank level in meters
  - Size: 1
  - Range: h ≥ 0
- `inputs` - Input vector `[q_in, x]` where:
  - `q_in` is inlet flow rate in m³/s
  - `x` is valve position, dimensionless in [0, 1]
  - Size: 2

**Returns:**
- Derivative vector `[dh/dt]` in m/s (size 1)

**Mathematics:**
```
dh/dt = (q_in - q_out) / A
q_out = k_v * x * sqrt(h)
```

**Preconditions:**
- `state(0) >= 0` (tank level must be non-negative)
- `inputs(1) in [0, 1]` (valve position must be valid)

**Postconditions:**
- Return value is defined for all valid inputs
- Return value ≈ 0 at steady state (q_in ≈ q_out)
- Return value > 0 when q_in > q_out (level rising)
- Return value < 0 when q_in < q_out (level falling)

**Example:**
```cpp
Eigen::VectorXd state(1);
state << 2.5;  // h = 2.5 m

Eigen::VectorXd inputs(2);
inputs << 1.0, 0.5;  // q_in = 1.0 m³/s, x = 0.5

Eigen::VectorXd deriv = model.derivatives(state, inputs);
// deriv[0] ≈ 0.0 (at steady state)
```

### Method: getOutletFlow

```cpp
double getOutletFlow(
    const Eigen::VectorXd& state,
    const Eigen::VectorXd& inputs) const;
```

Gets the current outlet flow rate for reporting and logging.

**Parameters:**
- `state` - Current state vector [h]
- `inputs` - Input vector [q_in, x]

**Returns:**
- Outlet flow rate in m³/s (non-negative)

**Formula:**
```
q_out = k_v * x * sqrt(h)
```

**Behavior:**
- Returns 0 if h ≤ 0 (empty tank)
- Returns 0 if x = 0 (valve closed)
- Increases with h and x

**Example:**
```cpp
double q_out = model.getOutletFlow(state, inputs);
std::cout << "Outlet flow: " << q_out << " m³/s" << std::endl;
```

---

## PIDController Class

Proportional-Integral-Derivative controller with anti-windup for feedback control.

### Include

```cpp
#include "pid_controller.h"
using namespace tank_sim;
```

### Class Definition

```cpp
class PIDController {
public:
    struct Gains { /* ... */ };
    PIDController(const Gains& gains, double bias, double min_output,
                  double max_output, double max_integral);
    double compute(double error, double error_dot, double dt);
    void setGains(const Gains& gains);
    void setOutputLimits(double min_val, double max_val);
    void reset();
    double getIntegralState() const;
};
```

### Nested Type: Gains

PID controller tuning parameters.

**Members:**
- `double Kc` - Proportional gain (dimensionless)
- `double tau_I` - Integral time constant in seconds (0 = no integral action)
- `double tau_D` - Derivative time constant in seconds (0 = no derivative action)

**Example:**
```cpp
PIDController::Gains gains{
    .Kc = 1.0,      // Proportional gain
    .tau_I = 10.0,  // Integral time (seconds)
    .tau_D = 2.0    // Derivative time (seconds)
};
```

**Typical Values:**
- **Level control:** Kc=0.5-2.0, tau_I=5-20s, tau_D=0-5s
- **Flow control:** Kc=1.0-3.0, tau_I=1-10s, tau_D=0-2s
- **Proportional only:** tau_I=0, tau_D=0

### Constructor

```cpp
PIDController(const Gains& gains, double bias, double min_output,
              double max_output, double max_integral);
```

Constructs a PID controller with tuning and saturation limits.

**Parameters:**
- `gains` - Controller gain parameters
- `bias` - Output bias when error is zero (typically 0.5 for valve position)
- `min_output` - Minimum output saturation limit (typically 0.0)
- `max_output` - Maximum output saturation limit (typically 1.0)
- `max_integral` - Maximum magnitude for integral state clamping (prevents windup)

**Example:**
```cpp
PIDController::Gains gains{1.0, 10.0, 2.0};
PIDController pid(gains, 0.5, 0.0, 1.0, 10.0);
```

### Method: compute

```cpp
double compute(double error, double error_dot, double dt);
```

Computes control output from error signals. Maintains integral state internally.

**Parameters:**
- `error` - Current error (setpoint - measured value)
- `error_dot` - Rate of change of error (de/dt)
- `dt` - Time step in seconds

**Returns:**
- Control output clamped to [min_output, max_output]

**Algorithm:**
1. Calculate proportional term: `Kc * error`
2. Calculate integral term: `(Kc / tau_I) * integral_state`
3. Calculate derivative term: `Kc * tau_D * error_dot`
4. Sum: `output = bias + p_term + i_term + d_term`
5. Saturate to [min_output, max_output]
6. Update integral (anti-windup): only if output was NOT saturated

**Anti-Windup:**
- Integral only accumulates when output is not saturated
- Prevents integral windup during sustained saturation
- Allows fast recovery when saturation is released

**Example:**
```cpp
double error = setpoint - measured_level;
double error_dot = (error - prev_error) / dt;
double output = pid.compute(error, error_dot, dt);
valve_position = output;
prev_error = error;
```

### Method: setGains

```cpp
void setGains(const Gains& gains);
```

Dynamically updates controller gains without resetting integral state (bumpless transfer).

**Parameters:**
- `gains` - New controller gain parameters

**Behavior:**
- Integral state is preserved (allows smooth gain changes)
- New gains take effect on next `compute()` call
- Use for tuning without disruption

**Example:**
```cpp
// Increase proportional gain at runtime
PIDController::Gains new_gains{2.0, 10.0, 2.0};
pid.setGains(new_gains);
```

### Method: setOutputLimits

```cpp
void setOutputLimits(double min_val, double max_val);
```

Changes output saturation limits after construction.

**Parameters:**
- `min_val` - New minimum output limit
- `max_val` - New maximum output limit

**Example:**
```cpp
// Limit valve to 25-75% range
pid.setOutputLimits(0.25, 0.75);
```

### Method: reset

```cpp
void reset();
```

Clears integral accumulator state to zero. Call when retuning or initializing.

**Effect:**
- Sets integral_state = 0
- Next compute() call will start with zero integral contribution

**Use Cases:**
- System reinitialization
- After major setpoint changes
- When switching control modes

**Example:**
```cpp
pid.reset();  // Clear integral before steady-state initialization
```

### Method: getIntegralState

```cpp
double getIntegralState() const;
```

Returns current integral accumulator for monitoring and logging.

**Returns:**
- Current value of accumulated integral error

**Use:**
- Diagnose controller behavior
- Check for windup during saturation
- Log for debugging

**Example:**
```cpp
double integral = pid.getIntegralState();
if (integral > 8.0) {
    std::cout << "Warning: Integral approaching windup limit" << std::endl;
}
```

---

## Stepper Class

Wraps GSL ODE solver (RK4 fixed-step integrator). Advances state vector forward in time.

### Include

```cpp
#include "stepper.h"
using namespace tank_sim;
```

### Class Definition

```cpp
class Stepper {
public:
    using DerivativeFunc = std::function<Eigen::VectorXd(
        double, const Eigen::VectorXd&, const Eigen::VectorXd&)>;
    
    explicit Stepper(size_t state_dimension);
    ~Stepper();
    
    // Non-copyable
    Stepper(const Stepper&) = delete;
    Stepper& operator=(const Stepper&) = delete;
    
    Eigen::VectorXd step(double t, double dt, const Eigen::VectorXd& state,
                         const Eigen::VectorXd& input, DerivativeFunc deriv_func);
};
```

### Type: DerivativeFunc

Function signature for derivative calculation.

```cpp
std::function<Eigen::VectorXd(
    double,                        // time t
    const Eigen::VectorXd&,        // state vector
    const Eigen::VectorXd&)        // input vector
>
```

The derivative function should:
- Accept current time, state, and inputs
- Return time derivative of state (same size as input state)
- Be deterministic (same inputs → same outputs)
- Handle edge cases gracefully (e.g., sqrt of small numbers)

**Example:**
```cpp
auto deriv_func = [&model](double t, const Eigen::VectorXd& state,
                           const Eigen::VectorXd& input) {
    return model.derivatives(state, input);
};
```

### Constructor

```cpp
explicit Stepper(size_t state_dimension);
```

Constructs a Stepper configured for the given state dimension.

**Parameters:**
- `state_dimension` - Size of the state vector (e.g., 1 for single tank)

**Behavior:**
- Allocates GSL ODE stepper resources
- Configures RK4 fixed-step method
- Does not allocate state vectors (caller manages)

**Example:**
```cpp
Stepper stepper(1);  // For single-state system
```

### Destructor

```cpp
~Stepper();
```

Frees GSL resources. Automatic cleanup (RAII).

**Note:** Destructor is defined in `.cpp` file. Do not manually call.

### Method: step

```cpp
Eigen::VectorXd step(double t, double dt, const Eigen::VectorXd& state,
                     const Eigen::VectorXd& input, DerivativeFunc deriv_func);
```

Advances state vector by one time step using RK4 integration.

**Parameters:**
- `t` - Current simulation time (passed to derivative function)
- `dt` - Integration time step in seconds
- `state` - Current state vector
- `input` - Current input vector (constant during step)
- `deriv_func` - Callback function computing derivatives

**Returns:**
- Updated state vector after integration

**Integration Method:**
- Fourth-order Runge-Kutta (RK4)
- Evaluates derivative function 4 times per step
- Fixed time step (no adaptive stepping)
- Local truncation error: O(dt^5)
- Global error: O(dt^4)

**Accuracy:**
- Error scales with dt^4
- For dt=0.01s: error ≈ dt^4 ≈ 1e-8
- Test with multiple dt values to verify convergence

**Example:**
```cpp
Eigen::VectorXd state(1);
state << 2.5;  // Initial level

Eigen::VectorXd input(2);
input << 1.0, 0.5;  // q_in, valve position

auto deriv = [&model](double t, const Eigen::VectorXd& s,
                      const Eigen::VectorXd& i) {
    return model.derivatives(s, i);
};

double t = 0.0, dt = 0.01;
Eigen::VectorXd next_state = stepper.step(t, dt, state, input, deriv);
```

---

## Simulator Class (Planned)

Master orchestrator coordinating Model, Controllers, and Stepper into a complete simulation system. Implementation pending.

**Planned responsibilities:**
- Own instances of Model, multiple Controllers, and Stepper
- Maintain state, inputs, time, and setpoints
- Coordinate simulation loop (integrate → update controllers)
- Provide getters for reporting/logging
- Allow runtime operator input changes
- Reset to initial steady-state conditions

See `docs/Simulator Class.md` for detailed specification.

---

## Type Definitions

### Eigen Types

```cpp
// Single value
double x = 1.5;

// Vectors
Eigen::VectorXd state(1);   // Single element
state << 2.5;
Eigen::VectorXd input(2);   // Two elements
input << 1.0, 0.5;

// Accessing elements
double h = state(0);
double q_in = input(0);
```

### Namespace

All classes are in the `tank_sim` namespace:

```cpp
using namespace tank_sim;
// Now can use TankModel, PIDController, Stepper directly

// Or explicit:
tank_sim::TankModel model(params);
```

---

## Example Usage

### Complete Control Loop Example

```cpp
#include "tank_model.h"
#include "pid_controller.h"
#include "stepper.h"
#include <iostream>
#include <Eigen/Dense>

using namespace tank_sim;

int main() {
    // 1. Configure tank model
    TankModel::Parameters model_params{
        .area = 120.0,
        .k_v = 1.2649,
        .max_height = 5.0
    };
    TankModel model(model_params);
    
    // 2. Configure PID controller
    PIDController::Gains pid_gains{
        .Kc = 1.0,      // Proportional gain
        .tau_I = 10.0,  // Integral time
        .tau_D = 2.0    // Derivative time
    };
    PIDController controller(pid_gains, 0.5, 0.0, 1.0, 10.0);
    
    // 3. Create stepper for RK4 integration
    Stepper stepper(1);  // 1D state (tank level)
    
    // 4. Initialize state at steady-state
    Eigen::VectorXd state(1);
    state << 2.5;  // 50% level = steady state
    
    Eigen::VectorXd input(2);
    input << 1.0, 0.5;  // q_in = 1.0, valve = 0.5
    
    // 5. Simulation parameters
    double t = 0.0;
    double dt = 0.01;
    double setpoint = 3.0;  // Target level 3.0 m
    double prev_error = 0.0;
    
    // 6. Simulation loop
    for (int step = 0; step < 1000; ++step) {
        // Measure error
        double error = setpoint - state(0);
        double error_dot = (error - prev_error) / dt;
        
        // Compute control output (valve position)
        double valve = controller.compute(error, error_dot, dt);
        input(1) = valve;
        
        // Integrate one step using RK4
        auto deriv = [&model](double t, const Eigen::VectorXd& s,
                             const Eigen::VectorXd& i) {
            return model.derivatives(s, i);
        };
        state = stepper.step(t, dt, state, input, deriv);
        
        // Advance time
        t += dt;
        prev_error = error;
        
        // Logging every 1 second
        if (step % 100 == 0) {
            double q_out = model.getOutletFlow(state, input);
            std::cout << "t=" << t << " h=" << state(0)
                     << " error=" << error
                     << " valve=" << valve
                     << " q_out=" << q_out << std::endl;
        }
    }
    
    return 0;
}
```

### Key Points:
1. Initialize at steady-state (q_in = q_out, level = setpoint)
2. Measure error each step (setpoint - measured)
3. Compute controller output
4. Integrate model forward using Stepper
5. Repeat

---

**Last Updated:** 2026-02-04  
**Version:** 1.1 (Phase 1 - C++ Core with Constants)

For more information:
- Complete architecture: `docs/plan.md`
- Process theory: `docs/TankDynamics.md`
- Next tasks: `docs/next.md`
- Constants reference: `src/constants.h`
- Developer guide: `docs/DEVELOPER_GUIDE.md`
