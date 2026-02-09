# Model Class

## Purpose

A stateless physics model that computes derivatives of the system given current state and inputs. The model encapsulates all governing equations (ODEs and algebraic equations) of the process.

## Responsibilities

- Compute time derivatives of state variables (dstate/dt) from the ODEs
- Evaluate algebraic (supplementary) equations
- Accept current state vector and input vector (manipulated variables)
- Return derivative vector for use by the stepper class
- Remain completely stateless - no internal state persistence

## Design Principles

The Model class is **pure computation**. It has no memory or side effects. Given the same inputs, it always produces the same outputs. This makes it:
- Easy to test in isolation
- Safe to call from any numerical integrator
- Compatible with different solvers and stepping strategies

## Interface

```cpp
class Model {
public:
    struct Parameters {
        // Physical constants and configuration
        // Examples: areas, coefficients, densities, etc.
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

## Public API

**`derivatives(state, inputs)`** - The single public method that computes time derivatives of all state variables given current state and inputs.

## Implementation Details

### Libraries
- **Eigen** - For vector and matrix operations
- Standard C++ (no external solver dependencies in Model itself)

### State Vector Structure

Define how your state vector is organized:
- `state[0..n-1]` - Each element represents one state variable
- Example: [level, temperature, concentration, ...]
- Size = number of ODEs in the system

### Input Vector Structure

Define how your inputs are organized:
- Contains all manipulated variables (things you can control)
- Contains all disturbance variables (things you can't control)
- Size = number of inputs to the system

### Key Method: `derivatives(state, inputs)`

Implements the system of ODEs:
- Evaluates all governing equations at current conditions
- Returns vector with one derivative for each state variable
- Called repeatedly by the Stepper during integration

### Internal Algebraic Equations (Private)

The Model also computes supplementary algebraic equations internally:
- These are intermediate calculations needed to evaluate the ODEs
- Examples: rate constant calculations, specific heat evaluations, property calculations
- Kept private - not exposed to the Simulator or SCADA system
- Called within `derivatives()` as helper functions to compute the rates of change
- It should be noted that some algerbriac equations will need to call external solvers to find a  solution.  For example, with a flash calculation a non-linear equation will need to be solved literately.


## Example Usage

```cpp
// Create model with parameters
Model::Parameters params;
// ... configure physical constants ...

Model model(params);

// Current state and inputs
Eigen::VectorXd state = /* ... */;
Eigen::VectorXd inputs = /* ... */;

// Get derivatives for next integration step
Eigen::VectorXd dstate_dt = model.derivatives(state, inputs);
// Stepper will use this to advance the state

// Get additional outputs for reporting
Eigen::VectorXd outputs = model.algebraicOutputs(state, inputs);
```

## Notes for Implementation

- Keep the model focused on equations; don't add control logic
- All parameters should be passed at construction or via the state vector
- Don't modify any member variables in `derivatives()` - make it `const`
- Ensure numerical stability for the solver (avoid discontinuities if possible)
- Test individual equations before integrating into the full model

## Related

- [[Simulation Architecture]]
- [[Stepper Class]]
- [[Simulator Class]]
