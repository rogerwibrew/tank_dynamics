
# Project Plan: Tank Dynamics Simulator

## Executive Summary

A real-time tank level control simulator with a SCADA-style interface. The system models a tank with variable inlet flow and PID-controlled outlet valve, allowing operators to experiment with control parameters and observe process dynamics. Built with a C++ simulation backend (using GSL and Eigen), exposed via pybind11 to a FastAPI server, with a Next.js frontend featuring WebSocket-based real-time updates.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                        Browser (Next.js)                        │
│  ┌─────────────────────────┐  ┌─────────────────────────────┐  │
│  │    Process View (Tab)   │  │     Trends View (Tab)       │  │
│  │  - Tank visualization   │  │  - Level vs Setpoint plot   │  │
│  │  - PID controls         │  │  - Flow plots (in/out)      │  │
│  │  - Flow indicators      │  │  - Valve position           │  │
│  │  - Setpoint input       │  │  - Historical data          │  │
│  └─────────────────────────┘  └─────────────────────────────┘  │
└────────────────────────────────┬────────────────────────────────┘
                                 │ WebSocket
                                 ▼
┌─────────────────────────────────────────────────────────────────┐
│                    FastAPI Server (Python)                      │
│  - WebSocket endpoint for real-time data                        │
│  - REST endpoints for control actions                           │
│  - Simulation orchestration (1 Hz tick rate)                    │
│  - Data history buffer (ring buffer, ~2 hours)                  │
└────────────────────────────────┬────────────────────────────────┘
                                 │ pybind11
                                 ▼
┌─────────────────────────────────────────────────────────────────┐
│                 C++ Simulation Library                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐  │
│  │  Tank Model  │  │ PID Control  │  │  RK4 Stepper (GSL)   │  │
│  │  - ODEs      │  │  - Error     │  │  - Fixed timestep    │  │
│  │  - Valve     │  │  - Integral  │  │  - State integration │  │
│  └──────────────┘  └──────────────┘  └──────────────────────┘  │
│                           Eigen (vectors/matrices)              │
└─────────────────────────────────────────────────────────────────┘
```

## Technology Decisions

| Component | Technology | Rationale |
|-----------|------------|-----------|
| Simulation Core | C++17 | Performance, numerical precision, Tennessee Eastman style |
| ODE Solver | GNU Scientific Library (GSL) | Robust RK4 implementation, well-documented |
| Linear Algebra | Eigen3 | Header-only, fast, industry standard |
| Build System | CMake | Cross-platform, FetchContent for dependencies |
| C++/Python Binding | pybind11 | Mature, excellent C++ integration |
| API Server | FastAPI | Async support, WebSockets, Python ecosystem |
| Frontend Framework | Next.js 14 (App Router) | React-based, good DX, SSR capability |
| Real-time Transport | WebSockets | Bidirectional, low latency for 1 Hz updates |
| Charting | Recharts | React-native, good performance, simpler than Plotly for this use case |
| Styling | Tailwind CSS | Rapid UI development, dark theme support |
| C++ Testing | GoogleTest | Industry standard, good CMake integration |
| Python Testing | pytest | Standard Python testing |
| E2E Testing | Playwright | Modern, reliable, good Next.js integration |

**Note on Recharts vs Plotly:** While you've used Plotly before, Recharts integrates more naturally with React's component model and has lighter bundle size. For the relatively simple time-series plots needed here (level, flow, valve position), Recharts is sufficient. If you prefer Plotly, it's a straightforward substitution.

## Component Breakdown

### C++ Simulation Library (`libsim`)

The simulation follows the Tennessee Eastman architecture pattern: the process model only computes derivatives, while integration is handled by an external stepper. This separation makes testing easier and follows established process simulation conventions.

**Detailed class specifications are maintained in separate documents:**
- `docs/Model Class.md` - Stateless physics model
- `docs/Stepper Class.md` - GSL RK4 wrapper
- `docs/PID Controller Class.md` - Feedback control with anti-windup
- `docs/Simulator Class.md` - Master orchestrator

#### Class 1: Model (Stateless Physics)

**Purpose:** Stateless physics model - computes derivatives given current state and inputs. Pure computation with no memory or side effects.

**Design Principle:** Given the same inputs, always produces the same outputs. This makes it easy to test in isolation, safe to call from any numerical integrator, and compatible with different solvers and stepping strategies.

**Responsibilities:**
- Compute time derivatives of state variables (dstate/dt) from the ODEs
- Evaluate algebraic (supplementary) equations internally as helpers
- Accept current state vector and input vector (manipulated variables)
- Return derivative vector for use by the stepper class
- Remain completely stateless - no internal state persistence

**For the tank system specifically:**
- Compute tank material balance derivative: `dh/dt = (q_in - q_out) / A`
- Compute outlet flow from valve equation: `q_out = k_v * x * sqrt(h)` (internal algebraic)

**Interface:**
```cpp
class Model {
public:
    struct Parameters {
        // Physical constants and configuration
        double area;        // Cross-sectional area (m²)
        double k_v;         // Valve coefficient (m^2.5/s)
        double max_height;  // Maximum tank height (m)
    };

    explicit Model(const Parameters& params);

    // Core method: compute derivatives given state and inputs
    // state: Current values of all state variables [n]
    // inputs: Current values of all inputs/manipulated variables [m]
    // Returns: Time derivatives [dstate/dt] of same size as state
    Eigen::VectorXd derivatives(
        const Eigen::VectorXd& state,
        const Eigen::VectorXd& inputs
    ) const;
};
```

**Note:** Algebraic equations (like outlet flow calculation) are kept private - called within `derivatives()` as helper functions. Some algebraic equations in more complex models may need to call external solvers for iterative solutions (e.g., flash calculations).

#### Class 2: Stepper (GSL RK4 Wrapper)

**Purpose:** Wraps GSL ODE solver (RK4 fixed-step integrator). Advances state vector forward in time by calling the Model's derivative function.

**Design Principle:** Thin wrapper around GSL providing a clean interface. Agnostic to the specific model - works with any derivative function signature.

**Responsibilities:**
- Configure and manage GSL ODE stepper (RK4 fixed step)
- Call the Model's `derivatives()` method at intermediate points as required by RK4
- Advance state vector by time step dt
- Return updated state vector after integration

**Key Implementation Notes:**
- RK4 is a single-step method - only requires current state (no history needed)
- Calls the derivative function multiple times per step (typically 4 times)
- Each integration step is independent (stateless with respect to simulation)

**Interface:**
```cpp
class Stepper {
public:
    // Derivative function signature
    using DerivativeFunc = std::function<Eigen::VectorXd(
        double t,
        const Eigen::VectorXd& state,
        const Eigen::VectorXd& inputs
    )>;

    explicit Stepper(size_t state_dim);
    ~Stepper();

    // Single public method: advance state by dt using RK4
    // derivative_func is called multiple times per step
    Eigen::VectorXd step(
        double t,
        double dt,
        const Eigen::VectorXd& state,
        const Eigen::VectorXd& inputs,
        DerivativeFunc derivative_func
    );
};
```

#### Class 3: PIDController (Feedback Control)

**Purpose:** Computes a manipulated variable (control output) from a measured variable error. Implements discrete-time PID with saturation and anti-windup.

**Responsibilities:**
- Track the integral of error over time (internal state)
- Compute PID output from proportional, integral, and derivative terms
- Clamp output to physical or logical limits (min/max)
- Prevent integral windup during output saturation
- Allow dynamic tuning of Kc, tau_I, and tau_D gains
- Provide reset capability for initialization or retuning

**Anti-Windup Implementation:**
1. **Conditional Integration:** Only update integral when output is NOT saturated
2. **Integral Clamping:** Additional safety limit on integral state magnitude

**Algorithm Order (Critical):**
1. Calculate P, I, D terms using CURRENT integral state
2. Compute unsaturated output
3. Clamp to physical limits
4. Update integral for NEXT timestep only if output was NOT saturated

**Interface:**
```cpp
class PIDController {
public:
    struct Gains {
        double Kc;      // Proportional gain (dimensionless)
        double tau_I;   // Integral time constant (seconds), 0 = no integral action
        double tau_D;   // Derivative time constant (seconds), 0 = no derivative action
    };

    PIDController(const Gains& gains, double bias, double min_output,
                  double max_output, double max_integral);

    // Compute control output
    double compute(double error, double error_dot, double dt);

    // Dynamic tuning (allows bumpless transfer)
    void setGains(const Gains& gains);
    void setOutputLimits(double min_val, double max_val);

    // Reset integral state
    void reset();

    // Get current integral state (for logging)
    double getIntegralState() const;
};
```

#### Class 4: Simulator (Master Orchestrator)

**Purpose:** Coordinates the Model, Controllers, and Stepper into a complete simulation system. Owns all instances and exposes the public API.

**Critical Design Decisions:**

1. **Steady-State Initialization (MANDATORY):** The simulation MUST be initialized at or very close to steady state. This is the programmer's responsibility. At steady state: all derivatives ≈ 0, all state variables equal their setpoints, controller outputs equal their bias values.

2. **State vs. Inputs:** State variables are governed by ODEs and evolve through integration - NEVER manipulated directly. Inputs feed INTO the ODEs and affect derivatives.

3. **Order of Operations (Critical):**
   - Step 1: Integrate model forward using inputs from PREVIOUS step
   - Step 2: Update simulation time
   - Step 3: Compute controller outputs for NEXT step
   This models the one-step delay of real digital control systems.

4. **Input Vector Structure:** Single vector containing ALL inputs (controller outputs + operator inputs). Model doesn't care where values come from.

5. **Multiple Controllers:** Each controller has measured_index (which state to read), output_index (which input to write), and its own setpoint.

**Interface:**
```cpp
class Simulator {
public:
    struct ControllerConfig {
        PIDController::Gains gains;
        double bias;             // Output at zero error (must match initial_inputs)
        double min_output;
        double max_output;
        double max_integral;
        int measured_index;      // Which state variable to measure
        int output_index;        // Which input to control
        double initial_setpoint; // Must equal initial_state[measured_index]
    };

    struct Config {
        Model::Parameters model_params;
        std::vector<ControllerConfig> controllers;
        Eigen::VectorXd initial_state;   // Steady-state values (= setpoints)
        Eigen::VectorXd initial_inputs;  // All inputs at steady state
        double dt;
    };

    explicit Simulator(const Config& config);

    // Core simulation method
    void step();

    // State getters
    double getTime() const;
    Eigen::VectorXd getState() const;
    Eigen::VectorXd getInputs() const;
    double getSetpoint(int controller_index) const;
    double getControllerOutput(int controller_index) const;
    double getError(int controller_index) const;

    // Operator control
    void setInput(int input_index, double value);    // Change any input
    void setSetpoint(int controller_index, double sp);
    void setControllerGains(int controller_index, const Gains& gains);

    // Reset to initial conditions
    void reset();
};
```

#### Key Parameters (calculated from requirements)

| Parameter | Value | Derivation |
|-----------|-------|------------|
| Tank height | 5.0 m | Specified |
| Cross-sectional area | 120.0 m² | From fill time requirement |
| Tank volume | 600.0 m³ | height × area |
| Valve coefficient k_v | 1.2649 m^2.5/s | Steady-state at 50% level, 50% valve |
| Initial level | 2.5 m | 50% of height |
| Initial valve position | 0.5 | 50% open |
| Initial flows | 1.0 m³/s | Specified inlet, outlet matches at steady state |

### Component 2: Python Bindings (`tank_sim`)

**Purpose:** Expose C++ library to Python

**Responsibilities:**
- Wrap TankSimulator class via pybind11
- Convert between C++ types and Python/NumPy types
- Provide pythonic interface

**Interfaces:**
```python
import tank_sim

sim = tank_sim.TankSimulator(config)
sim.step(1.0)  # Advance 1 second
state = sim.get_state()  # Returns dict with all state variables
```

### Component 3: FastAPI Backend (`api/`)

**Purpose:** Web API and simulation orchestration

**Responsibilities:**
- Run simulation loop at 1 Hz (real-time)
- Broadcast state via WebSocket to connected clients
- Handle control commands (setpoint, PID tuning, inlet flow)
- Maintain ring buffer of last 2 hours of data (~7200 points)
- Provide REST endpoints for historical data and configuration

**Interfaces:**
```
WebSocket /ws
  -> Server sends: {"type": "state", "data": {...}} every 1s
  <- Client sends: {"type": "setpoint", "value": 3.0}
  <- Client sends: {"type": "pid", "Kc": 1.0, "tau_I": 10.0, "tau_D": 0.0}
  <- Client sends: {"type": "inlet_flow", "value": 1.2}
  <- Client sends: {"type": "inlet_mode", "mode": "brownian", "min": 0.8, "max": 1.2}

REST GET /api/history?duration=3600
  -> Returns last hour of data points

REST GET /api/config
  -> Returns current simulation configuration

REST POST /api/reset
  -> Resets simulation to initial steady state
```

### Component 4: Next.js Frontend (`frontend/`)

**Purpose:** SCADA-style user interface

**Responsibilities:**
- Display real-time process state (tank level, flows, valve position)
- Render tank visualization with animated level
- Provide controls for setpoint, PID parameters, inlet flow
- Display trend charts for process variables
- Manage WebSocket connection and reconnection

**Sub-components:**
1. **ProcessView** - Tank schematic, live values, controls
2. **TrendsView** - Time-series plots
3. **ControlPanel** - PID tuning, setpoint, inlet flow controls
4. **TankGraphic** - SVG tank with animated fill level

## Implementation Phases

### Phase 1: C++ Simulation Core ✅ COMPLETE
**Status:** Complete and thoroughly tested
**Deliverables:**
- ✅ CMakeLists.txt with FetchContent for Eigen, GSL, GoogleTest
- ✅ Tank model implementation (ODEs, valve physics)
- ✅ PID controller with anti-windup
- ✅ RK4 integration using GSL wrapper (Stepper)
- ✅ Master Simulator orchestrator coordinating all components
- ✅ 42 comprehensive unit tests (100% pass rate)
- ✅ Complete standalone test suite verifying all dynamics

**Tests:** 42/42 passing
- TankModel: 7 tests ✅
- PIDController: 10 tests ✅
- Stepper: 7 tests ✅
- Simulator: 18 tests ✅

**Dependencies:** None

### Phase 2: Python Bindings ✅ COMPLETE
**Status:** Complete with all recommendations implemented
**Deliverables:**
- ✅ pybind11 module exposing all C++ classes
- ✅ Modern Python package structure using scikit-build-core
- ✅ pyproject.toml with proper metadata and dependencies
- ✅ Automatic NumPy ↔ Eigen conversion
- ✅ Complete exception handling (C++ → Python)
- ✅ Helper function `create_default_config()`
- ✅ 28 comprehensive pytest tests (100% pass rate)
- ✅ Full documentation with examples and edge cases
- ✅ Code review with all recommendations implemented

**Tests:** 28/28 Python tests passing
- Configuration creation: 4 tests ✅
- Simulator construction: 3 tests ✅
- Steady-state stability: 1 test ✅
- Step response: 2 tests ✅
- Disturbance rejection: 2 tests ✅
- Reset functionality: 2 tests ✅
- Exception handling: 2 tests ✅
- NumPy arrays: 2 tests ✅
- Dynamic retuning: 1 test ✅
- Edge cases: 7 tests ✅
- Integration: 2 tests ✅

**Dependencies:** Phase 1

### Phase 3: FastAPI Backend
**Goals:** Real-time API server
**Deliverables:**
- FastAPI application with WebSocket support
- Simulation loop running at 1 Hz
- Ring buffer for 2-hour history
- REST endpoints for history and control
- API tests

**Dependencies:** Phase 2

### Phase 4: Next.js Frontend - Structure
**Goals:** Basic UI framework
**Deliverables:**
- Next.js project with App Router
- Tailwind CSS with dark theme
- Tab navigation (Process / Trends)
- WebSocket connection hook
- Basic layout and styling

**Dependencies:** Phase 3 (for testing, can start in parallel)

### Phase 5: Process View
**Goals:** SCADA-style process display
**Deliverables:**
- Tank SVG visualization with animated level
- Flow indicators (inlet, outlet)
- Valve position indicator
- Live value displays
- Control inputs (setpoint, inlet flow, PID parameters)

**Dependencies:** Phase 4

### Phase 6: Trends View
**Goals:** Historical trend charts
**Deliverables:**
- Level vs Setpoint chart
- Inlet Flow vs Outlet Flow chart
- Valve Position chart
- Time range selector
- Auto-scrolling with new data

**Dependencies:** Phase 4

### Phase 7: Integration and Polish
**Goals:** Complete, tested system
**Deliverables:**
- E2E tests with Playwright
- Error handling and reconnection logic
- Performance optimization
- Documentation updates

**Dependencies:** Phases 5, 6

## Project Structure

```
tank_dynamics/
├── CMakeLists.txt              # Top-level CMake
├── src/                        # C++ source
│   ├── CMakeLists.txt
│   ├── tank_model.h            # Class 1: Stateless derivative function (Model)
│   ├── tank_model.cpp
│   ├── stepper.h               # Class 2: GSL RK4 wrapper
│   ├── stepper.cpp
│   ├── pid_controller.h        # Class 3: PID with integral state
│   ├── pid_controller.cpp
│   ├── simulator.h             # Class 4: Master orchestrator
│   └── simulator.cpp
├── bindings/                   # pybind11 bindings
│   ├── CMakeLists.txt
│   └── bindings.cpp
├── tests/                      # C++ tests
│   ├── CMakeLists.txt
│   ├── test_tank_model.cpp     # Test derivative calculations
│   ├── test_stepper.cpp        # Test integration accuracy
│   ├── test_pid_controller.cpp # Test PID logic
│   └── test_simulator.cpp      # Test full orchestration
├── api/                        # FastAPI backend
│   ├── __init__.py
│   ├── main.py
│   ├── simulation.py
│   ├── models.py
│   └── tests/
│       └── test_api.py
├── frontend/                   # Next.js frontend
│   ├── package.json
│   ├── app/
│   │   ├── layout.tsx
│   │   ├── page.tsx
│   │   └── components/
│   ├── tailwind.config.js
│   └── tests/
│       └── e2e/
├── docs/
│   ├── specs.md                # Project specification
│   ├── plan.md                 # Implementation plan
│   ├── Model Class.md          # Detailed Model class specification
│   ├── Stepper Class.md        # Detailed Stepper class specification
│   ├── PID Controller Class.md # Detailed PIDController class specification
│   ├── Simulator Class.md      # Detailed Simulator class specification
│   └── ...
└── scripts/
    └── run_dev.sh              # Start all services
```

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| GSL integration complexity | Medium | Medium | Use CMake FetchContent; fallback to simpler Euler method if needed |
| WebSocket reliability | Low | Medium | Implement reconnection logic with exponential backoff |
| Real-time performance (1 Hz) | Low | Low | 1 Hz is easily achievable; profile if issues arise |
| pybind11 build issues | Medium | Medium | Pin versions; provide fallback Python simulation for frontend dev |
| Brownian motion inlet causing instability | Low | Medium | Implement rate limiting on random walk; clamp to safe bounds |

## Testing Strategy

### Unit Tests (C++ - GoogleTest)

**TankModel tests:**
- Verify `dh/dt = 0` at steady state (q_in = q_out)
- Verify `dh/dt > 0` when q_in > q_out
- Verify `dh/dt < 0` when q_in < q_out
- Verify outlet flow calculation: `q_out = k_v * x * sqrt(h)`
- Edge cases: h = 0, x = 0, x = 1

**PIDController tests:**
- Proportional only: output proportional to error
- Integral accumulation: output increases over time with constant error
- Derivative response: output responds to rate of change
- Saturation: output clamped to [0, 1]
- Anti-windup: integral doesn't wind up when saturated
- Reset: integral state clears

**Stepper tests:**
- Simple ODE with known solution (e.g., exponential decay)
- Verify 4th-order accuracy by comparing step sizes
- State dimension handling

**Simulator tests:**
- Steady-state: no change when at setpoint
- Step response: correct transient behavior
- Setpoint tracking: level follows setpoint changes
- Mode switching: Brownian inlet generates bounded random walk

### Unit Tests (Python - pytest)
- Binding functionality: ensure all methods accessible
- Type conversions: verify data passes correctly between C++/Python
- State consistency: run steps and verify state updates

### Integration Tests (pytest)
- API endpoints: test REST routes
- WebSocket: test connection, message format, command handling
- Simulation loop: verify timing and state updates

### E2E Tests (Playwright)
- UI renders correctly
- WebSocket connection established
- Controls update simulation
- Trends display data
- Tab navigation works

## Success Criteria

- [ ] Simulation runs in real-time (1 Hz) without drift
- [ ] Tank level responds correctly to setpoint changes
- [ ] PID controller stabilizes level at setpoint
- [ ] Inlet flow step changes cause expected transient response
- [ ] Brownian inlet mode produces realistic disturbances
- [ ] Frontend displays live process state
- [ ] Trend charts show 2 hours of history
- [ ] All unit tests pass
- [ ] E2E tests verify complete workflow
- [ ] System runs on Linux (primary), with potential for other platforms

## Next Steps

1. Initialize GitHub repository
2. Set up CMake project structure with FetchContent
3. Begin Phase 1: C++ Simulation Core

---

*Plan created: 2026-01-28*
*Architect: Claude (Opus)*
