# Next Tasks - Tank Dynamics Simulator

## Current Phase: Phase 1 - C++ Simulation Core

**Status:** TankModel implementation complete and tested

**Progress:** 30% - TankModel complete with 7 passing unit tests

**Recent Commits:**
- Task 1: Initialize C++ Project Structure and Build System ✓
- Task 2: Implement TankModel Class ✓
- Task 3: Write Unit Tests for TankModel ✓

---

## Task 4: Implement PIDController Class

**Phase:** 1 - C++ Simulation Core
**Prerequisites:** Task 1 (build system must be in place)

### Files to Create

- Create `/home/roger/dev/tank_dynamics/src/pid_controller.h`
- Create `/home/roger/dev/tank_dynamics/src/pid_controller.cpp`
- Update `/home/roger/dev/tank_dynamics/src/CMakeLists.txt` to include these source files in the tank_sim_core library

### Requirements

The PIDController class implements a proportional-integral-derivative controller with anti-windup. Unlike the TankModel which is stateless, this class maintains internal state for the integral term.

#### pid_controller.h specifications:

The header should define a class called PIDController within the tank_sim namespace.

The class needs a public nested structure called Gains containing:
- A field for controller gain Kc (dimensionless, type: double)
- A field for integral time tau_I in seconds (type: double, zero means no integral action)
- A field for derivative time tau_D in seconds (type: double, zero means no derivative action)

The class should have:
- A constructor that accepts:
  - A const reference to Gains
  - An optional bias value (type: double, default 0.5) representing the controller output when error is zero
- A method called compute that accepts:
  - Error value (setpoint minus process variable, type: double)
  - Error derivative (rate of change of error, type: double)
  - Time step dt in seconds (type: double)
  - Returns the controller output clamped to the range zero to one (type: double)
- A method called setGains that accepts a const reference to Gains and updates the tuning parameters
- A method called reset with no parameters that clears the integral state
- A method called getIntegralState that returns the current integral accumulation value (type: double)

All methods except the constructor should be non-const since the controller maintains internal state.

#### pid_controller.cpp specifications:

The class should maintain private member variables for:
- The current gains (Gains struct)
- The bias value (double)
- The integral accumulation value (double, initialized to zero)

Implement the constructor to:
- Store the provided gains and bias in member variables
- Initialize the integral accumulation to zero

Implement the compute method using the PID equation in velocity form:
- Calculate the proportional term as: Kc times error
- Calculate the integral term as: (Kc divided by tau_I) times error times dt
  - Only add to integral if tau_I is greater than zero (avoid division by zero)
  - Accumulate this integral contribution in the member variable
- Calculate the derivative term as: (Kc times tau_D) times error_dot
  - Only include this if tau_D is greater than zero
- Sum all terms: bias plus proportional plus integral plus derivative
- Clamp the result to the range zero to one before returning
- Implement anti-windup: if the output before clamping was outside zero to one, do not add the current integral contribution to the accumulator (this prevents integral windup when saturated)

Implement setGains to:
- Update the gains member variable with the new values
- Do not reset the integral state (allow bumpless transfer)

Implement reset to:
- Set the integral accumulation back to zero

Implement getIntegralState to:
- Return the current value of the integral accumulation

#### Mathematics and Control Theory

Standard PID equation in velocity form:
```
output = bias + Kc * e + (Kc/tau_I) * integral(e*dt) + (Kc*tau_D) * de/dt
```

where:
- e is the error (setpoint minus process variable)
- de/dt is the rate of change of error
- integral(e*dt) is the accumulated integral over time
- Kc is the controller gain (positive for direct action, negative for reverse action)
- tau_I is the integral time constant in seconds
- tau_D is the derivative time constant in seconds
- bias is the output when error is zero (typically 0.5 for a valve)

Anti-windup logic:
- Calculate tentative output including new integral contribution
- If tentative output is outside bounds (less than zero or greater than one), discard the integral contribution
- This prevents the integral term from growing unbounded when the output is saturated

### Edge Cases

- **Zero integral time (tau_I equals zero):** Skip integral calculation entirely to avoid division by zero
- **Zero derivative time (tau_D equals zero):** Skip derivative calculation
- **Large errors causing saturation:** Anti-windup should prevent integral buildup
- **Negative gains:** Controller should handle reverse-acting controllers (though tank level typically uses direct action)
- **Very small dt values:** Should not cause numerical issues since we're just multiplying

### Verification

Manual verification before unit tests:
- With gains Kc equals 1.0, tau_I equals 10.0, tau_D equals 0.0, bias equals 0.5
- Given error equals 0.1, error_dot equals 0.0, dt equals 1.0
- Proportional contribution: 1.0 times 0.1 equals 0.1
- Integral contribution: (1.0 divided by 10.0) times 0.1 times 1.0 equals 0.01
- Derivative contribution: 0.0
- Output: 0.5 plus 0.1 plus 0.01 equals 0.61 (within bounds, no clamping)

Saturation test:
- With same gains, error equals 1.0, error_dot equals 0.0, dt equals 1.0
- Proportional: 1.0
- Output before clamping: 0.5 plus 1.0 equals 1.5 (exceeds 1.0)
- Output should be clamped to 1.0
- Integral contribution should NOT be added to accumulator (anti-windup)

### Acceptance Criteria

- [ ] pid_controller.h created in src/ directory with class declaration
- [ ] pid_controller.cpp created in src/ directory with implementation
- [ ] Gains struct contains Kc, tau_I, and tau_D
- [ ] Constructor accepts gains and optional bias
- [ ] compute() method implements PID calculation with anti-windup
- [ ] Output is clamped to range zero to one
- [ ] setGains() method updates tuning parameters
- [ ] reset() method clears integral state
- [ ] getIntegralState() returns current integral value
- [ ] Code uses tank_sim namespace
- [ ] Handles edge cases: zero tau_I, zero tau_D
- [ ] Anti-windup prevents integral growth during saturation
- [ ] src/CMakeLists.txt updated to compile these files
- [ ] Build succeeds: `cmake --build build`
- [ ] No compilation errors or warnings

---

## Task 5: Write Unit Tests for PIDController

**Phase:** 1 - C++ Simulation Core
**Prerequisites:** Task 4 (PIDController must be implemented)

### Files to Create

- Create `/home/roger/dev/tank_dynamics/tests/test_pid_controller.cpp`
- Update `/home/roger/dev/tank_dynamics/tests/CMakeLists.txt` to compile this test file

### Requirements

This task creates comprehensive unit tests for the PIDController class using GoogleTest framework.

The test file should include the necessary headers:
- GoogleTest headers (gtest/gtest.h)
- The pid_controller.h header

Create a test fixture class using GoogleTest TEST_F to share common setup across tests, or use simple TEST macros for independent tests.

#### Test Cases to Implement

**Test: Proportional Only Response**
- Create PIDController with Kc equals 1.0, tau_I equals 0.0 (no integral), tau_D equals 0.0 (no derivative), bias equals 0.5
- Call compute with error equals 0.1, error_dot equals 0.0, dt equals 1.0
- Expected output: 0.5 plus (1.0 times 0.1) equals 0.6
- Assert output matches expected value (use EXPECT_NEAR with tolerance 0.001)
- Verify output is proportional to error by testing with error equals 0.2 (should give 0.7)

**Test: Integral Accumulation Over Time**
- Create PIDController with Kc equals 1.0, tau_I equals 10.0, tau_D equals 0.0, bias equals 0.5
- Call compute three times with constant error equals 0.1, error_dot equals 0.0, dt equals 1.0
- First call: output should be approximately 0.5 plus 0.1 plus 0.01 equals 0.61
- Second call: output should increase by another 0.01 (integral accumulating)
- Third call: output should increase by another 0.01
- Verify integral term grows linearly with time for constant error

**Test: Derivative Response**
- Create PIDController with Kc equals 1.0, tau_I equals 0.0, tau_D equals 5.0, bias equals 0.5
- Call compute with error equals 0.0, error_dot equals 0.1 (error increasing), dt equals 1.0
- Derivative contribution: (1.0 times 5.0) times 0.1 equals 0.5
- Expected output: 0.5 plus 0.0 plus 0.5 equals 1.0
- Assert output matches expected value

**Test: Output Saturation at Upper Bound**
- Create PIDController with Kc equals 1.0, tau_I equals 0.0, tau_D equals 0.0, bias equals 0.5
- Call compute with error equals 1.0, error_dot equals 0.0, dt equals 1.0
- Raw output would be 0.5 plus 1.0 equals 1.5
- Assert output is clamped to exactly 1.0
- Verify clamping works

**Test: Output Saturation at Lower Bound**
- Create PIDController with Kc equals 1.0, tau_I equals 0.0, tau_D equals 0.0, bias equals 0.5
- Call compute with error equals -1.0, error_dot equals 0.0, dt equals 1.0
- Raw output would be 0.5 minus 1.0 equals -0.5
- Assert output is clamped to exactly 0.0

**Test: Anti-Windup During Saturation**
- Create PIDController with Kc equals 2.0, tau_I equals 1.0, tau_D equals 0.0, bias equals 0.5
- Call compute multiple times with large positive error that causes saturation
- After several calls, check integral state using getIntegralState()
- Reset the controller using reset()
- Now call compute with moderate error that doesn't saturate
- Verify that integral didn't grow excessively during saturation period
- Compare with a controller that didn't experience saturation
- The saturated controller's integral should be much smaller

**Test: Reset Clears Integral State**
- Create PIDController with Kc equals 1.0, tau_I equals 10.0, tau_D equals 0.0, bias equals 0.5
- Call compute several times to build up integral
- Call getIntegralState() and verify it's non-zero
- Call reset()
- Call getIntegralState() and verify it's now exactly zero
- Call compute with same error and verify output is as if no history existed

**Test: SetGains Updates Behavior**
- Create PIDController with Kc equals 1.0, tau_I equals 0.0, tau_D equals 0.0, bias equals 0.5
- Call compute with error equals 0.1 and record output (should be 0.6)
- Create new gains with Kc equals 2.0 (double the gain)
- Call setGains with new gains
- Call compute again with same error equals 0.1
- Expected output: 0.5 plus (2.0 times 0.1) equals 0.7
- Assert output increased appropriately

**Test: Zero Error Produces Bias Output**
- Create PIDController with any gains and bias equals 0.5
- Ensure integral is zero (newly constructed or after reset)
- Call compute with error equals 0.0, error_dot equals 0.0, dt equals 1.0
- Assert output equals exactly 0.5 (the bias value)

**Test: Combined PID Action**
- Create PIDController with Kc equals 1.0, tau_I equals 10.0, tau_D equals 2.0, bias equals 0.5
- Call compute with error equals 0.1, error_dot equals 0.05, dt equals 1.0
- Proportional: 1.0 times 0.1 equals 0.1
- Integral: (1.0 divided by 10.0) times 0.1 times 1.0 equals 0.01
- Derivative: (1.0 times 2.0) times 0.05 equals 0.1
- Expected: 0.5 plus 0.1 plus 0.01 plus 0.1 equals 0.71
- Assert output matches expected (use EXPECT_NEAR)

### Test Execution

After implementation:
- Update tests/CMakeLists.txt to include test_pid_controller.cpp
- Build the project: `cmake --build build`
- Run tests: `ctest --test-dir build --output-on-failure` or `./build/tests/test_tank_sim_core`
- All tests should pass

### Edge Cases

- **Numerical precision:** Use EXPECT_NEAR with tolerance of 0.001 for floating-point comparisons
- **Anti-windup verification:** This is the trickiest test - ensure integral doesn't grow during saturation
- **Division by zero:** Verify tau_I equals zero doesn't cause crashes

### Acceptance Criteria

- [ ] test_pid_controller.cpp created in tests/ directory
- [ ] File includes GoogleTest and pid_controller.h headers
- [ ] Proportional-only test implemented and passes
- [ ] Integral accumulation test implemented and passes
- [ ] Derivative response test implemented and passes
- [ ] Upper saturation test implemented and passes
- [ ] Lower saturation test implemented and passes
- [ ] Anti-windup test implemented and passes
- [ ] Reset test implemented and passes
- [ ] SetGains test implemented and passes
- [ ] Zero error bias test implemented and passes
- [ ] Combined PID test implemented and passes
- [ ] tests/CMakeLists.txt updated to compile test file
- [ ] Build succeeds: `cmake --build build`
- [ ] All tests pass: `./build/tests/test_tank_sim_core`

---

## Task 6: Implement Stepper Class with GSL RK4

**Phase:** 1 - C++ Simulation Core
**Prerequisites:** Task 1 (build system with GSL dependency)

### Files to Create

- Create `/home/roger/dev/tank_dynamics/src/stepper.h`
- Create `/home/roger/dev/tank_dynamics/src/stepper.cpp`
- Update `/home/roger/dev/tank_dynamics/src/CMakeLists.txt` to include these source files and link against GSL

### Requirements

The Stepper class wraps the GNU Scientific Library's Runge-Kutta 4th order (RK4) ODE solver. This class is responsible for advancing the state vector forward in time by calling a user-provided derivative function multiple times per step as required by the RK4 algorithm.

This follows the Tennessee Eastman pattern where the integration logic is separate from the physics model. The physics model only computes derivatives; the stepper handles the numerical integration.

#### stepper.h specifications:

The header should define a class called Stepper within the tank_sim namespace.

The class needs a type alias for the derivative function. This function type should:
- Accept current time t (type: double)
- Accept current state vector (type: const reference to Eigen::VectorXd)
- Accept input vector (type: const reference to Eigen::VectorXd)
- Return derivative vector (type: Eigen::VectorXd)

Suggested name for the type alias: DerivativeFunc

The class should have:
- A constructor that accepts:
  - State dimension (type: size_t) - the number of state variables
- A destructor to clean up GSL resources
- A method called step that accepts:
  - Current time t in seconds (type: double)
  - Time step dt in seconds (type: double)
  - Current state vector (type: const reference to Eigen::VectorXd)
  - Input vector (type: const reference to Eigen::VectorXd)
  - Derivative function (type: DerivativeFunc)
  - Returns the updated state vector after advancing by dt (type: Eigen::VectorXd)

The class should follow the rule of five (or rule of zero if using smart pointers):
- If managing GSL resources with raw pointers, declare and define:
  - Destructor to free GSL memory
  - Deleted copy constructor and copy assignment (Stepper cannot be copied)
  - Optionally move constructor and move assignment
- Alternative: use unique_ptr with custom deleters for RAII

#### stepper.cpp specifications:

The class should maintain private member variables for:
- GSL stepper object (type: gsl_odeiv2_step pointer)
- GSL control object for step size control (type: gsl_odeiv2_control pointer) - though for fixed step we may not need adaptive control
- GSL evolution object (type: gsl_odeiv2_evolve pointer)
- State dimension (type: size_t)

Implement the constructor to:
- Store the state dimension
- Allocate GSL stepper using gsl_odeiv2_step_alloc with RK4 algorithm (gsl_odeiv2_step_rk4)
- The GSL function requires dimension as an argument
- Initialize with fixed step size (no adaptive control needed for this application)

Implement the destructor to:
- Free the GSL stepper using gsl_odeiv2_step_free
- Free any other GSL objects that were allocated

Implement the step method:
- Create a wrapper that converts the user's derivative function into the format GSL expects
- GSL's derivative function signature is: int func(double t, const double y[], double dydt[], void* params)
- Use a lambda or helper to bridge between Eigen vectors and C arrays
- Store the user's derivative function and inputs in a structure that can be passed via void* params
- Call the GSL RK4 step function (gsl_odeiv2_step_apply) with:
  - Current time t
  - Time step dt
  - Current state as C array (use state.data() to get pointer)
  - Allocated array for updated state
  - The derivative function wrapper
- Convert the resulting C array back to Eigen::VectorXd and return it

#### GSL Integration Details

GSL's ODE solver requires:
- A system definition (gsl_odeiv2_system) containing:
  - Function pointer to derivative function
  - Jacobian function (can be nullptr for RK4)
  - Dimension
  - Parameters pointer
- Step type specification (gsl_odeiv2_step_rk4 for RK4)

For fixed-step RK4, we can use gsl_odeiv2_step_apply directly without the evolution framework. This simplifies the implementation.

The RK4 algorithm will call the derivative function four times per step to compute intermediate slopes. Our wrapper must handle these calls correctly.

### Edge Cases

- **State dimension mismatch:** Document assumption that state vector size matches constructor dimension
- **Input vector size:** Document expected size based on system design (for tank: two inputs - q_in and x)
- **Null derivative function:** In C++ using std::function, this will throw - document requirement for valid function
- **Negative dt:** Document assumption that dt is positive
- **GSL allocation failure:** Check return values and handle errors appropriately

### Verification

Manual verification strategy (before writing formal tests):
- Test with a simple exponential decay ODE: dy/dt equals negative k times y
- Analytical solution: y(t) equals y0 times exp(negative k times t)
- Choose k equals 1.0, y0 equals 1.0, integrate from t equals 0 to t equals 1 with dt equals 0.1
- Compare numerical result with analytical: exp(negative 1.0) equals approximately 0.3679
- RK4 should be very close to analytical solution

Order verification:
- RK4 is fourth-order accurate: error should decrease as dt to the fourth power
- Run same test with dt equals 0.1 and dt equals 0.05
- Error with dt equals 0.05 should be approximately (0.05 divided by 0.1) to the fourth power equals 1/16 times the error with dt equals 0.1

### Acceptance Criteria

- [ ] stepper.h created in src/ directory with class declaration
- [ ] stepper.cpp created in src/ directory with implementation
- [ ] DerivativeFunc type alias defined for derivative function signature
- [ ] Constructor accepts state dimension and allocates GSL resources
- [ ] Destructor properly frees all GSL resources
- [ ] Copy constructor and copy assignment are deleted or properly implemented
- [ ] step() method correctly interfaces with GSL RK4
- [ ] Conversion between Eigen vectors and C arrays handled correctly
- [ ] Code uses tank_sim namespace
- [ ] Proper error handling for GSL allocation failures
- [ ] src/CMakeLists.txt updated to compile these files and link GSL
- [ ] Build succeeds: `cmake --build build`
- [ ] No compilation errors, warnings, or linker errors
- [ ] No memory leaks (use valgrind or asan if available)

---

## Upcoming Work (After Task 6)

Once the Stepper is implemented and tested, the next tasks will be:

7. Write integration accuracy tests for Stepper (verify RK4 order and accuracy)
8. Implement Simulator orchestrator class (simulator.h/cpp) - brings together TankModel, PIDController, and Stepper
9. Write comprehensive tests for Simulator (full system behavior)
10. Create standalone executable to run simulation and output time-series data for verification
11. Review and refine C++ implementation before moving to Phase 2

After Phase 1 is complete, we'll move to Phase 2: Python bindings using pybind11.

---

## Notes

**Testing Strategy:** For Task 6, we're focusing on implementation first, then writing dedicated integration tests in Task 7. This differs slightly from the previous pattern where tests immediately followed implementation, but makes sense for the Stepper since its tests require more sophisticated verification (numerical accuracy, order verification).

**GSL Dependency:** If GSL is not installed on your system, install it via package manager:
- Ubuntu/Debian: `sudo apt-get install libgsl-dev`
- Arch Linux: `sudo pacman -S gsl`
- macOS: `brew install gsl`

**Memory Management:** Special attention needed for GSL resource management. Consider using RAII wrappers or smart pointers with custom deleters to avoid leaks.

---

*Generated: 2026-01-29*
*Senior Engineer: Claude (Sonnet)*
