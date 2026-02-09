# Project Specification

## Overview

This is a simulation of a tank.  It will look like a scada screen that a process operator would use.  The user will be able to monitor process conditions and adjust varibles (PID) settings to tune the process.

## Goals

- This project will develop a dynamic model of a tank with variable inlet flow and and outlet flow controlled by a PID loop controlling tank level.
- We build the logic for the ODEs and algerbric equations in cpp.
- We will use the RK4 ODE solver provided by the GNU scientific library with fixed time step
- We will use Eigen for matricies and matrix manipulation
- The state vector consists of one variable - h or height of fluid in the tank.
- The manipulated variable vector consists of severak variables - qin, the liquid flowrate into the tank and x, the position of the valve, and the PID settings.  The tank level setpoint.
- We will need to store the manipulated varibles and state variables.  For display in the front end as well as for use later in a machine learing library as data.

## Non-Goals

- No scope drift.  This needs to be a proof of concept before taking on furher work.

## Users

1. A process operator using the tank SCADA screen to experiment with tank control and PID settings.

## Features

### Feature 1: SCADA Interface

**Description:** Allows the user to control the simulation.  It looks like a screen you would see in a control room.  A tank will be represented in the screen and all measured process variables will be visible.  One would normally be able to make custom plots, but this simulation will have a second screen that shows a pre-define set of plots.  The PID seetings will be availble on the screen to change.

**User Story:** As a conrol room operator I want to use the simuator so I can understand how these pocesses work better.

**Requirements:**
- A screen represntig the process with all process varibles present.
- All manipulated variables on the screen so the operator can contorl them.
- A second screen that allows the opertor to view process trends.
  - A tank level against setpont plot 
  - Inlet flow vs outlet flow
  - any others
  - Written as a next.js app.
  
**Acceptance Criteria:**
- [ ] The user can see the state of the process
- [ ] The user can manipulate the process with the tools at his disposal
- [ ] The user can see the process history by looking at plots.

### Feature 2: Process Simuator

**Description** Is the backend for the application.  Runs the process and communicates with the frontend.

**User Story** The user will not see this as it is the backend.

**Requirements**
- Should be similar to the Tenesee Eastman Fortran Sructure as much as possible.  See reference docs
- Written in cpp with pybind11 exposing an API to frontend.
- Uses GNU scientific library for ODE solving
- Uses Eigen for matricies

## Technical Requirements

### Performance
- Ideally we will be stepping through the simulation at 60fps.

### Security
- This is a proof of concept that will only run on my machine.  It will be pushed to github for others to use but security is their concern.

### Compatibility
- This will be compiled for the platform it is running on and will communcate to a react app in the browser.

### Dependencies
- Eigen
- GNU sceintific library
- CMake to build
- GoogleTest to test
- NodeJS

## Core Class Architecture

The simulation follows the Tennessee Eastman architecture pattern with four core classes:

### Class 1: Model (Stateless Physics)

A stateless physics model that computes derivatives of the system given current state and inputs. The model encapsulates all governing equations (ODEs and algebraic equations) of the process.

**Responsibilities:**
- Compute time derivatives of state variables (dstate/dt) from the ODEs
- Evaluate algebraic (supplementary) equations
- Accept current state vector and input vector (manipulated variables)
- Return derivative vector for use by the stepper class
- Remain completely stateless - no internal state persistence

**Design Principle:** Pure computation with no memory or side effects. Given the same inputs, always produces the same outputs.

**Interface:**
- `derivatives(state, inputs)` - The single public method that computes time derivatives

**Detailed specification:** See `docs/Model Class.md`

### Class 2: Stepper (GSL RK4 Wrapper)

Wraps the GSL ODE solver (RK4 fixed-step integrator). Advances the state vector forward in time by calling the Model's derivative function.

**Responsibilities:**
- Configure and manage GSL ODE stepper (RK4 fixed step)
- Call the Model's `derivatives()` method at intermediate points as required by RK4
- Advance state vector by time step dt
- Return updated state vector after integration

**Design Principle:** Thin wrapper around GSL providing a clean interface. Agnostic to specific model - works with any derivative function signature.

**Interface:**
- `step(t, dt, state, inputs, derivative_func)` - Advances state by time step dt

**Detailed specification:** See `docs/Stepper Class.md`

### Class 3: PIDController (Feedback Control)

Computes a manipulated variable (control output) from a measured variable error. Implements a discrete-time PID controller with saturation and anti-windup.

**Responsibilities:**
- Track the integral of error over time (internal state)
- Compute PID output from proportional, integral, and derivative terms
- Clamp the output to physical or logical limits (min/max)
- Prevent integral windup during output saturation
- Allow dynamic tuning of Kc, tau_I, and tau_D gains
- Provide reset capability for initialization or retuning

**Anti-Windup Strategy:**
- Conditional integration: Only update integral when output is NOT saturated
- Integral clamping: Direct limit on integral state magnitude

**Interface:**
- `compute(error, error_dot, dt)` - Calculate control output
- `setGains(gains)` - Update tuning parameters dynamically
- `setOutputLimits(min, max)` - Change saturation limits
- `reset()` - Clear integral state
- `getIntegralState()` - Get current integral accumulator for logging

**Detailed specification:** See `docs/PID Controller Class.md`

### Class 4: Simulator (Master Orchestrator)

The master orchestrator that coordinates the Model, Controllers, and Stepper into a complete simulation system.

**Responsibilities:**
- Own and manage Model, multiple Controllers, and Stepper instances
- Maintain state vector, input vector, time, and setpoints
- Coordinate the simulation loop in the correct order
- Provide getters for all variables for reporting/logging
- Allow operators to change setpoints and disturbance inputs
- Reset simulation to initial conditions
- Expose clean API suitable for binding and remote calls

**Critical Design Decisions:**

1. **Steady-State Initialization:** The simulation MUST be initialized at or very close to steady state. This is the programmer's responsibility before configuring the Simulator.

2. **State vs. Inputs:** State variables are governed by ODEs and evolve through integration (NEVER manipulated directly). Inputs feed INTO the ODEs and affect derivatives.

3. **Order of Operations:** Step FIRST (integrate model forward using inputs from previous step), then compute controller outputs for NEXT step. This models the one-step delay of real digital control.

4. **Input Vector Structure:** Single vector containing ALL inputs (controller outputs + operator inputs). Model doesn't care where values come from.

**Interface:**
- `step()` - Advance simulation by dt
- `getState()`, `getInputs()`, `getTime()` - State getters
- `setInput(index, value)` - Change any input (disturbances)
- `setSetpoint(controller_index, sp)` - Change controller setpoint
- `setControllerGains(index, gains)` - Retune controllers
- `reset()` - Reset to initial steady state

**Detailed specification:** See `docs/Simulator Class.md`

## Constraints

None at this stage

## Open Questions

1. There is nothing in our process for testing. We need to add testing and a testing engineer prompt. This should be added to our workflow. Preferably the local llm will write tests as the senior engineer instruction.

## References

The technical background is in docs/TankDynamics
Class specifications are in:
- docs/Model Class.md
- docs/Stepper Class.md
- docs/PID Controller Class.md
- docs/Simulator Class.md

---

## Notes for Architect

When creating the plan, please address:

1. [Specific concern or question]
2. [Another area needing attention]

Preferred technologies (if any):
- [Technology preferences]

Timeline considerations:
- [Any deadline or time constraints]
