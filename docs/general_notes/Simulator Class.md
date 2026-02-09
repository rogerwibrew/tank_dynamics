# Simulator Class

## Purpose

The master orchestrator that coordinates the Model, Controllers, and Stepper into a complete simulation system. Owns instances of all components, maintains simulation state, and exposes the public API for calling code (Python bindings, SCADA frontend, etc.).

## Responsibilities

- Own and manage Model, multiple Controllers, and Stepper instances
- Maintain state vector, input vector, time, and setpoints
- Coordinate the simulation loop in the correct order
- Provide getters for all variables for reporting/logging
- Allow operators to change setpoints and disturbance inputs
- Reset simulation to initial conditions
- Expose clean API suitable for binding and remote calls

## Steady-State Initialization (Critical)

**The simulation MUST be initialized at or very close to steady state.** This is the programmer's responsibility before configuring the Simulator.

At steady state:
- All derivatives are zero (the system is at equilibrium)
- All state variables equal their setpoints (error = 0)
- All inputs (disturbances + controller outputs) are at values that maintain equilibrium

**Why this matters:**
- If the system starts far from steady state, controllers will immediately respond to large errors
- This can cause unrealistic transients, saturation, and windup
- The simulation should begin at a realistic operating point, then respond to operator changes
- If there are recycles in the model and the state is not near to steady-state, then the model may not coverage properly.

**What the programmer must do:**
1. Choose setpoint values for each controlled variable
2. Choose disturbance values (feed flows, ambient conditions, etc.)
3. Solve for the steady-state: find state values and controller outputs where all derivatives = 0
4. This often requires solving the Model equations analytically or numerically
5. Verify: `model.derivatives(steady_state, steady_inputs) ≈ [0, 0, ...]`

**At initialization:**
- `state = steady_state_values` (which equal the setpoints)
- `disturbance_inputs = values consistent with steady state`
- `controller_outputs = bias values` (since error ≈ 0 and integral = 0)
- `integral_state = 0` for all controllers

The Simulator does not compute steady state - it assumes the programmer has done this work.

## State vs. Inputs

**State variables** are governed by ODEs and evolve through integration. They are NEVER manipulated directly. The state emerges from the physics - it is the result of integrating the derivatives over time.

**Inputs** (also called disturbances or manipulated variables) feed INTO the ODEs and affect the derivatives. Changing an input changes how state evolves, but does not set state directly.

Example: To raise tank level, you don't set `state[level] = 3.0`. You increase the inlet flow (an input), which increases the derivative `d(level)/dt`, which causes level to rise over time through integration.

## Input Vector Structure

The input vector U is a single vector containing ALL inputs to the Model:
- Controller outputs (e.g., valve positions) - updated each step by control logic
- Operator inputs (e.g., feed flow rates) - changed by operator action via `setInput()`

The Model doesn't care where input values come from - it just receives the vector and computes derivatives. Each controller knows which index it writes to (`output_index`).

## Controller Structure

For systems with multiple control loops, the Simulator owns a LIST of controllers:

```
controllers = [
    {
        controller: PIDController instance,
        measured_index: which state variable to read,
        setpoint: current setpoint value,
        output_index: which input to write controller output to
    },
    ...
]
```

Each controller:
- Reads ONE state variable (the measured/process variable)
- Compares to its setpoint
- Writes to ONE input (the manipulated variable)

This assumes SISO (single-input single-output) control loops, which covers most industrial control.

## Order of Operations (Critical)

In discrete-time simulation, there is an inherent one-step delay between measurement and control action. This reflects real digital control systems where:
1. Sensors sample the process
2. Controller computes output
3. Actuator moves
4. Process evolves over the next interval

**Correct order for each step:**

```
At the START of step, we have:
- Current state S(t)
- Current inputs U(t) [computed at END of PREVIOUS step]
- Current setpoints SP

STEP 1: Integrate the model forward (this updates state)
    S(t+dt) = stepper.step(t, dt, S(t), U(t), model.derivatives)

    Note: We step using inputs computed at end of PREVIOUS step.
    This is the realistic one-step delay of digital control.

STEP 2: Update simulation time
    t = t + dt

STEP 3: For each controller, compute output for NEXT step
    For each controller i:
        measured = S(t+dt)[controller.measured_index]
        error = controller.setpoint - measured
        error_dot = compute_error_derivative(...)  // see below
        U[controller.output_index] = controller.compute(error, error_dot, dt)

At the END of step:
- State is S(t+dt)
- Inputs are ready for next step
- Time is updated
```

**Why step FIRST, then controller?**
- The controller reads the RESULT of the previous step
- It computes an output that will be USED in the next step
- This models the sample-compute-actuate cycle of real digital control

## Computing the Error Derivative

The PID D term needs error_dot = d(setpoint)/dt - d(measured)/dt.

If setpoint is constant (common case): error_dot = -d(measured)/dt

**Options for d(measured)/dt:**

1. **From the model derivatives (recommended):**
   ```
   derivatives = model.derivatives(S(t+dt), U(t))
   state_rate = derivatives[measured_index]
   error_dot = setpoint_rate - state_rate
   ```
   Most accurate, but requires an extra derivative evaluation.

2. **From backward difference:**
   ```
   state_rate = (S(t+dt)[measured_index] - S(t)[measured_index]) / dt
   error_dot = setpoint_rate - state_rate
   ```
   Requires storing previous state. Simple but less accurate.

3. **Ignore D term:**
   Many industrial controllers use PI only (tau_D = 0), avoiding this complexity.

## Interface

**Configuration Structure:**
```cpp
struct ControllerConfig {
    PIDController::Gains gains;
    double bias;                 // Output at zero error (steady-state output)
    double min_output;
    double max_output;
    double max_integral;
    int measured_index;          // Which state variable to measure
    int output_index;            // Which input to control
    double initial_setpoint;     // Must equal initial_state[measured_index] at startup
};

struct Config {
    Model::Parameters model_params;
    std::vector<ControllerConfig> controllers;
    Eigen::VectorXd initial_state;   // Steady-state values (= setpoints)
    Eigen::VectorXd initial_inputs;  // All inputs at steady state
    double dt;
};
```

**Important:** The `bias` in each ControllerConfig should equal the corresponding value in `initial_inputs` for that controller's output_index. Both represent the steady-state controller output.

**State Getters:**
```cpp
double getTime() const
Eigen::VectorXd getState() const
Eigen::VectorXd getInputs() const
double getSetpoint(int controller_index) const
double getControllerOutput(int controller_index) const
double getError(int controller_index) const
```

**Setters (for operator control):**
```cpp
void setInput(int input_index, double value)        // Change any input (e.g., feed flow)
void setSetpoint(int controller_index, double sp)   // Change a controller setpoint
void setControllerGains(int controller_index, Gains gains)  // Retune
```

## Algorithm Pseudocode

```
CLASS Simulator

    INIT(config)
        this.model = new Model(config.model_params)
        this.stepper = new Stepper(state_dimension)
        this.config = config  // Store for reset()

        // Create list of controllers
        this.controllers = []
        FOR each ctrl_config in config.controllers:
            controller = {
                pid: new PIDController(ctrl_config.gains, ctrl_config.bias, ...),
                measured_index: ctrl_config.measured_index,
                output_index: ctrl_config.output_index,
                setpoint: ctrl_config.initial_setpoint
            }
            this.controllers.append(controller)
        END FOR

        // Initialize at steady state (programmer's responsibility to find this)
        this.state = config.initial_state
        this.previous_state = config.initial_state
        this.inputs = config.initial_inputs

        this.time = 0.0
        this.dt = config.dt
    END INIT


    FUNCTION step() RETURNS void

        // Save current state for backward difference calculation
        this.previous_state = this.state

        // STEP 1: Integrate model forward - this computes the new state
        //         Uses inputs computed at end of PREVIOUS step (one-step delay)
        this.state = this.stepper.step(
            this.time,
            this.dt,
            this.previous_state,
            this.inputs,
            this.model.derivatives
        )

        // STEP 2: Update time
        this.time = this.time + this.dt

        // STEP 3: For each controller, compute output for NEXT step
        FOR each ctrl in this.controllers:

            // Get measured variable from NEW state
            measured = this.state[ctrl.measured_index]

            // Compute error
            error = ctrl.setpoint - measured

            // Compute error derivative (using backward difference)
            prev_measured = this.previous_state[ctrl.measured_index]
            measured_rate = (measured - prev_measured) / this.dt
            setpoint_rate = 0.0  // Assume setpoint constant within step
            error_dot = setpoint_rate - measured_rate

            // Compute controller output for NEXT step
            output = ctrl.pid.compute(error, error_dot, this.dt)

            // Store in inputs vector for NEXT iteration
            this.inputs[ctrl.output_index] = output

        END FOR

    END FUNCTION


    FUNCTION setInput(input_index, value) RETURNS void
        // Operator can change any input directly
        this.inputs[input_index] = value
    END FUNCTION


    FUNCTION setSetpoint(controller_index, new_setpoint) RETURNS void
        this.controllers[controller_index].setpoint = new_setpoint
    END FUNCTION


    FUNCTION setControllerGains(controller_index, new_gains) RETURNS void
        this.controllers[controller_index].pid.setGains(new_gains)
    END FUNCTION


    FUNCTION reset() RETURNS void
        // Reset to initial steady state
        this.state = this.config.initial_state
        this.previous_state = this.config.initial_state
        this.inputs = this.config.initial_inputs
        this.time = 0.0

        // Reset each controller
        FOR each ctrl in this.controllers:
            ctrl.setpoint = ctrl.initial_setpoint
            ctrl.pid.reset()  // Clear integral state
        END FOR
    END FUNCTION

END CLASS
```

## Example: Multi-Loop System

Consider a reactor with:
- State: [level, temperature]
- Inputs: [feed_flow (disturbance), coolant_valve (controlled), steam_valve (controlled)]
- Controllers: level controller (manipulates coolant_valve), temperature controller (manipulates steam_valve)

**Programmer's steady-state calculation (done before configuring Simulator):**
```
Given: setpoints level=2.5m, temp=350K, feed_flow=1.0 m³/s
Find: valve positions where all derivatives = 0

Solve model equations at steady state:
  d(level)/dt = 0  →  coolant_valve = 0.5 (50% open)
  d(temp)/dt = 0   →  steam_valve = 0.4 (40% open)

Verify: model.derivatives([2.5, 350], [1.0, 0.5, 0.4]) ≈ [0, 0]
```

**Configuration:**
```cpp
Config config;

// Steady-state values
config.initial_state = {2.5, 350.0};   // state = setpoints
config.initial_inputs = {1.0, 0.5, 0.4};  // [feed_flow, coolant_valve, steam_valve]
config.dt = 0.01;

// Level controller: measures state[0], controls inputs[1]
// bias = 0.5 (must match initial_inputs[1])
config.controllers.push_back({
    .gains = {Kc: 2.0, tau_I: 10.0, tau_D: 0.0},
    .bias = 0.5,
    .measured_index = 0,
    .output_index = 1,
    .initial_setpoint = 2.5
});

// Temperature controller: measures state[1], controls inputs[2]
// bias = 0.4 (must match initial_inputs[2])
config.controllers.push_back({
    .gains = {Kc: 0.5, tau_I: 30.0, tau_D: 5.0},
    .bias = 0.4,
    .measured_index = 1,
    .output_index = 2,
    .initial_setpoint = 350.0
});

Simulator sim(config);
// At this point:
//   state = [2.5, 350.0]
//   inputs = [1.0, 0.5, 0.4]
//   All derivatives ≈ 0, system is at equilibrium

// Run simulation - nothing happens because we're at steady state
for (int i = 0; i < 100; ++i) {
    sim.step();
}

// NOW introduce a disturbance - feed flow increases
sim.setInput(0, 1.5);
// Controllers will respond to bring system back to setpoints

// Or change a setpoint - operator wants higher temperature
sim.setSetpoint(1, 360.0);
// Temperature controller will drive temp toward 360K
```

## Key Points

- **State is never manipulated directly:** State evolves from ODEs through integration. Inputs affect derivatives, which affect state evolution
- **Steady-state initialization is mandatory:** Programmer must find equilibrium point before configuring. States = setpoints, controller bias = steady-state output, all derivatives ≈ 0
- **One input vector:** Model doesn't care where inputs come from. Controllers write to their indices, operators can change any input
- **Step FIRST, then controller:** Controller reads result of integration, computes output for NEXT step
- **One-step delay is realistic:** Models sample-compute-actuate cycle of digital control
- **Multiple controllers:** Each controller has its own measured variable, setpoint, and output index
- **Error derivative:** Use backward difference or model derivatives; or just use PI control

## Related

- [[Model Class]]
- [[PID Controller Class]]
- [[Stepper Class]]
- [[Simulation Architecture]]
