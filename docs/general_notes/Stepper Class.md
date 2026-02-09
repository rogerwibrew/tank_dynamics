# Stepper Class

## Purpose

Wraps the GSL ODE solver (RK4 fixed-step integrator). Advances the state vector forward in time by calling the Model's derivative function.

## Responsibilities

- Configure and manage GSL ODE stepper (RK4 fixed step)
- Call the Model's `derivatives()` method at intermediate points as required by RK4
- Advance state vector by time step dt
- Return updated state vector after integration

## Design Principles

The Stepper is a thin wrapper around GSL's ODE solver. It:
- Handles all GSL initialization and memory management
- Provides a clean interface that accepts the derivative function
- Remains agnostic to the specific model - works with any derivative function signature
- Performs one integration step at a time (not a full simulation)

## Interface

The Stepper has a single public method:

**`step(t, dt, state, inputs, derivative_func)`** - Advances state by time step dt

Parameters:
- `t` - Current time
- `dt` - Time step to integrate
- `state` - Current state vector
- `inputs` - Current input vector
- `derivative_func` - Callback function that computes derivatives (from Model class)

Returns:
- Updated state vector after one RK4 step

The derivative function signature matches what the Model provides: takes time, state, and inputs; returns derivatives vector.

## Implementation Details

### GSL Integration

Uses GSL RK4 (4th order Runge-Kutta) fixed-step integrator:
- Chosen for reasonable accuracy vs. computational cost
- Fixed step size for deterministic behavior and control
- Calls the derivative function multiple times per step (typically 4 times)

### Single-Step Method

RK4 is a single-step method:
- Only requires the current state to compute the next state
- No history of previous states needed
- Each integration step is independent
- Simplifies state management - no need to retain old iterations

### State Management

- Owns state buffers for RK4 intermediate calculations
- Accepts current state, integrates, returns new state
- Stateless with respect to simulation - each step is independent

### Memory Management

- Allocates GSL stepper structures at construction
- Deallocates at destruction
- Handles GSL's C-style memory requirements

## Example Usage

```
// Create stepper for system with n state variables
Stepper stepper(n_state_variables);

// Current simulation time and state
double t = 0.0;
double dt = 0.001;  // 1ms time step
Eigen::VectorXd state = /* ... */;
Eigen::VectorXd inputs = /* ... */;

// Step forward in time
// Pass a lambda or function pointer to Model::derivatives()
state = stepper.step(t, dt, state, inputs,
                     [&model](double t, const Eigen::VectorXd& s,
                              const Eigen::VectorXd& u) {
                       return model.derivatives(s, u);
                     });

t += dt;
```

## Notes

- The Stepper calls the derivative function multiple times during a single step
- It does not accumulate state - each step is independent
- The Simulator class will call step() repeatedly in a loop to advance time
- Time step size affects accuracy and computational cost - typically chosen empirically

## Related

- [[Simulation Architecture]]
- [[Model Class]]
- [[Simulator Class]]
