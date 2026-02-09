# Remaining Code Improvements

**Date:** 2026-02-02  
**Status:** Pending Implementation  
**Source:** Code Review (docs/feedback.md)

## Overview

The code review identified several improvements that should be implemented. Critical issue C2 (CTest discovery) has been resolved. Critical issue C1 (Stepper unit tests) will be implemented separately as requested.

This document tracks the remaining improvements from the code review.

## Completed

- ✅ **C2: CTest Discovery** - Fixed by manually fetching Eigen (see `docs/CTEST_FIX.md`)

## Major Issues (Should Address Before Phase 2)

### M1: Inconsistent Error Handling Strategy

**Files:** `src/tank_model.cpp`, `src/pid_controller.cpp`, `src/stepper.cpp`

**Current State:**
- TankModel: Uses `assert()` (debug-only)
- Stepper: Uses `throw std::runtime_error()`  
- PIDController: No validation

**Recommendation:**
1. Document error handling policy in `DEVELOPER_GUIDE.md`
2. Apply consistently:
   - Constructor validation: Throw exceptions for invalid parameters
   - Runtime preconditions: Assertions for debug, consider runtime checks for critical paths
   - External library errors: Always check and throw

**Implementation:**
```cpp
// TankModel constructor
TankModel::TankModel(const Parameters& params) {
    if (params.area <= 0.0) {
        throw std::invalid_argument("Tank area must be positive");
    }
    if (params.k_v <= 0.0) {
        throw std::invalid_argument("Valve coefficient must be positive");
    }
    // ...
}

// PIDController constructor  
PIDController::PIDController(const Gains& gains, ...) {
    if (gains.tau_I < 0.0) {
        throw std::invalid_argument("Integral time constant cannot be negative");
    }
    // ...
}
```

---

### M2: Stepper Input Dimension Validation

**File:** `src/stepper.cpp:74`

**Problem:**
Validates state vector size but not input vector size.

**Implementation:**
```cpp
class Stepper {
public:
    Stepper(size_t state_dimension, size_t input_dimension);
    
private:
    size_t state_dimension_;
    size_t input_dimension_;
};

Eigen::VectorXd Stepper::step(...) {
    if (state.size() != static_cast<int>(state_dimension_)) {
        throw std::runtime_error("State vector size mismatch");
    }
    if (input.size() != static_cast<int>(input_dimension_)) {
        throw std::runtime_error("Input vector size mismatch");
    }
    // ...
}
```

---

### M3: PID Anti-Windup Floating-Point Comparison

**File:** `src/pid_controller.cpp:30`

**Current Code:**
```cpp
if (output_unsat == output) {  // Fragile floating-point equality
    integral_state = integral_state + error * dt;
}
```

**Better Approach:**
```cpp
// Check if output was saturated
bool saturated = (output_unsat < min_output || output_unsat > max_output);
if (!saturated) {
    integral_state = integral_state + error * dt;
    integral_state = std::clamp(integral_state, -max_integral, max_integral);
}
```

Or with tolerance:
```cpp
const double tolerance = 1e-10;
bool saturated = (std::abs(output - output_unsat) > tolerance);
if (!saturated) {
    // accumulate
}
```

---

### M4: Missing GSL Memory Management Documentation

**File:** `src/stepper.h`, `src/stepper.cpp`

**Add Documentation:**
```cpp
/**
 * @brief Stepper manages GSL resources using RAII.
 * 
 * The GSL ODE stepper is allocated in the constructor and freed in the 
 * destructor. Copy operations are deleted because GSL steppers cannot be 
 * safely copied (they contain internal state pointers). Move operations 
 * are not implemented but could be added if needed.
 * 
 * Memory management follows the Rule of Five:
 * - Custom destructor (frees GSL resources)
 * - Deleted copy constructor
 * - Deleted copy assignment
 * - No move constructor (could be added)
 * - No move assignment (could be added)
 */
class Stepper {
    // ...
    
private:
    gsl_odeiv2_step* stepper_;  ///< GSL RK4 stepper (owned, freed in destructor)
    size_t state_dimension_;     ///< Number of state variables
};
```

---

## Minor Issues (Improve When Convenient)

### m1: Time Parameter in TankModel

**File:** `src/tank_model.h`

**Issue:**
Stepper expects `DerivativeFunc(double t, VectorXd state, VectorXd input)` but TankModel provides `derivatives(VectorXd state, VectorXd input)`.

**Solution:**
Document that TankModel is time-invariant and requires a wrapper:

```cpp
/**
 * @brief Computes the derivative of tank level (dh/dt).
 * 
 * @note This model is time-invariant (derivatives don't depend on time t).
 *       When using with Stepper, wrap in a lambda:
 *       @code
 *       auto deriv_func = [&model](double t, const VectorXd& state, const VectorXd& input) {
 *           return model.derivatives(state, input);  // Ignore time parameter
 *       };
 *       @endcode
 */
Eigen::VectorXd derivatives(
    const Eigen::VectorXd& state,
    const Eigen::VectorXd& inputs) const;
```

---

### m2: Magic Numbers in Tests

**Files:** `tests/test_tank_model.cpp:11-13`, `tests/test_pid_controller.cpp`

**Add Context:**
```cpp
// Standard test parameters from docs/plan.md
// These values establish steady state at:
//   h = 2.5 m (50% level)
//   q_in = q_out = 1.0 m³/s
//   valve_position = 0.5 (50% open)
// k_v calculated: k_v = q_out / (x * sqrt(h)) = 1.0 / (0.5 * sqrt(2.5)) ≈ 1.2649
TankModel::Parameters params{
    120.0,      // area: from fill time requirement (600 m³ / 5 m = 120 m²)
    1.2649,     // k_v: calibrated for steady state
    5.0         // max_height: specified requirement
};
```

---

### m3: PIDController Method Documentation

**File:** `src/pid_controller.h`

**Add Detailed Comments:**
```cpp
/**
 * @brief Update controller gains dynamically.
 * 
 * Changes Kc, tau_I, and tau_D without resetting integral state.
 * Allows bumpless transfer during retuning.
 * 
 * @param gains New controller gains
 * @note Does NOT modify integral_state (for bumpless transfer)
 */
void setGains(const Gains& gains);

/**
 * @brief Change the output saturation limits.
 * 
 * @param min_val New minimum output limit
 * @param max_val New maximum output limit
 * @note Does NOT reset integral state
 */
void setOutputLimits(double min_val, double max_val);
```

---

## Implementation Priority

**Before Task 7 (Simulator):**
1. M1: Document and apply consistent error handling
2. M2: Add input dimension validation to Stepper

**Before Phase 2 (Python Bindings):**
3. M3: Refactor PID anti-windup check
4. M4: Document GSL memory management

**When Convenient:**
5. m1-m3: Documentation improvements

---

## Testing After Implementation

After implementing each improvement:

```bash
# Rebuild
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure

# Verify stepper
./build/bindings/stepper_verify
```

All existing tests should continue to pass.

---

## Notes

- These improvements enhance code quality and maintainability
- None are blockers for current functionality
- Implement incrementally to avoid introducing bugs
- Each improvement should be a separate commit with clear message

---

**Next Steps:**
1. Review this document
2. Decide which improvements to implement now vs. later
3. Implement in priority order
4. Update this document as items are completed
