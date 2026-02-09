# PID Controller Class

## Purpose

Computes a manipulated variable (control output) from a measured variable error. Implements a discrete-time PID controller with saturation and anti-windup to regulate a process around a setpoint. Suitable for any simulation that needs feedback control.

## Responsibilities

- Track the integral of error over time (internal state)
- Compute PID output from proportional, integral, and derivative terms
- Clamp the output to physical or logical limits (min/max)
- Prevent integral windup during output saturation
- Allow dynamic tuning of Kc, tau_I, and tau_D gains
- Provide reset capability for initialization or retuning

## Interface

**Data Structure: Gains**
```cpp
struct Gains {
    double Kc;      // Proportional gain (dimensionless)
    double tau_I;   // Integral time constant (seconds), 0 = no integral action
    double tau_D;   // Derivative time constant (seconds), 0 = no derivative action
};
```

**Constructor:**
- `PIDController(const Gains& gains, double bias, double min_output, double max_output, double max_integral)`
  - Initializes with gains, bias (output at zero error), output saturation limits, and integral clamp

**Methods:**
- `double compute(double error, double error_dot, double dt)` - Calculate control output
- `void setGains(const Gains& gains)` - Update tuning parameters dynamically
- `void setOutputLimits(double min_val, double max_val)` - Change saturation limits
- `void reset()` - Clear integral state
- `double getIntegralState() const` - Get current integral accumulator for logging

## Implementation Details

The controller discretizes the continuous PID law using forward Euler integration:

**Error calculation:** `e = setpoint - measured_value`

**Integral state update:** `z_new = z_old + e * dt` (conditional - see below)

**Error derivative:** `e_dot = d(setpoint)/dt - d(measured_value)/dt`

**Unsaturated PID output:**
```
output_unsat = bias + Kc * (e + (1/tau_I)*z + tau_D*e_dot)
```

**Saturation to physical limits:**
```
output = clamp(output_unsat, min_output, max_output)
```

## The Integral Windup Problem

The integral term accumulates the area under the error curve over time. This creates two related problems:

### Problem 1: Windup During Saturation

When output saturates (hits physical limits like fully open/closed):
1. Error persists because the system can't respond further
2. Integral keeps accumulating even though it can't affect the process
3. When saturation lifts, the accumulated integral causes excessive overshoot

**Solution: Conditional Integration**

Only update the integral when output is NOT saturated:
```
IF output_unsat == output THEN
    // Not saturated - safe to integrate
    integral_state = integral_state + error * dt
END IF
```

This prevents accumulating "phantom area" during periods when the controller cannot act.

### Problem 2: Normal Integral Overshoot

Even without saturation, integral action causes some overshoot:
1. Positive error persists → integral grows
2. Output at 60% (not saturated) → process responds
3. Process approaches setpoint → error reaches zero
4. Integral is still large → output stays elevated → overshoot past setpoint

**This is expected PID behavior.** The overshoot creates negative error, which then reduces the integral, and the system converges. This is not a bug—it's how integral action works.

**Strategies to manage normal overshoot:**

1. **Derivative action** - The D term sees the rate of approach. As error shrinks quickly, error_dot is negative, reducing output *before* overshoot occurs.

2. **Integral clamping** - Directly limit the integral state magnitude:
   ```
   integral_state = CLAMP(integral_state, -max_integral, +max_integral)
   ```

3. **Tuning tradeoffs:**
   - Larger tau_I → slower integral action → less overshoot, slower steady-state correction
   - Smaller Kc → less aggressive response → less overshoot, slower overall

## Algorithm Pseudocode

```
CLASS PIDController

    INIT(Kc, tau_I, tau_D, bias, min_output, max_output, max_integral)
        this.Kc = Kc
        this.tau_I = tau_I
        this.tau_D = tau_D
        this.bias = bias
        this.min_output = min_output
        this.max_output = max_output
        this.max_integral = max_integral
        this.integral_state = 0.0
    END INIT


    FUNCTION compute(error, error_dot, dt) RETURNS control_output

        // Step 1: Calculate proportional term
        p_term = error

        // Step 2: Calculate integral term using CURRENT state
        IF tau_I != 0 THEN
            i_term = (1.0 / tau_I) * this.integral_state
        ELSE
            i_term = 0.0
        END IF

        // Step 3: Calculate derivative term
        d_term = tau_D * error_dot

        // Step 4: Compute unsaturated output
        output_unsat = bias + Kc * (p_term + i_term + d_term)

        // Step 5: Clamp to physical limits
        output = CLAMP(output_unsat, min_output, max_output)

        // Step 6: Update integral for NEXT timestep
        //         Only if output was NOT saturated (anti-windup)
        IF output_unsat == output THEN
            this.integral_state = this.integral_state + error * dt
            // Also clamp integral state directly (belt and braces)
            this.integral_state = CLAMP(this.integral_state,
                                        -max_integral, max_integral)
        END IF

        RETURN output

    END FUNCTION


    FUNCTION CLAMP(value, min_val, max_val) RETURNS clamped_value
        IF value < min_val THEN
            RETURN min_val
        ELSE IF value > max_val THEN
            RETURN max_val
        ELSE
            RETURN value
        END IF
    END FUNCTION


    FUNCTION setGains(new_Kc, new_tau_I, new_tau_D)
        this.Kc = new_Kc
        this.tau_I = new_tau_I
        this.tau_D = new_tau_D
    END FUNCTION


    FUNCTION reset()
        this.integral_state = 0.0
    END FUNCTION


    FUNCTION getIntegralState() RETURNS this.integral_state
    END FUNCTION

END CLASS
```

## Direct-Acting vs. Reverse-Acting Control

**CRITICAL:** The sign of Kc determines whether the controller is direct-acting or reverse-acting. Getting this wrong will cause the control loop to be unstable!

### Direct-Acting Control (Kc > 0)

Use when: **Increasing controller output should INCREASE the controlled variable.**

Examples:
- Heater power controlling temperature (more power → higher temp)
- Inlet valve controlling tank level (more open → higher level)
- Pump speed controlling flow rate (faster → more flow)

### Reverse-Acting Control (Kc < 0)

Use when: **Increasing controller output should DECREASE the controlled variable.**

Examples:
- Outlet valve controlling tank level (more open → lower level)
- Cooling flow controlling temperature (more cooling → lower temp)
- Vent valve controlling pressure (more open → lower pressure)

### How to Determine

Trace the signal path from controller output to controlled variable:

1. If an increase in output leads to an increase in the controlled variable → **Direct (Kc > 0)**
2. If an increase in output leads to a decrease in the controlled variable → **Reverse (Kc < 0)**

### Tank Level Example (This Project)

In this tank dynamics project, the controller output goes to the **outlet valve**:
- More open valve → more outlet flow → **lower** tank level
- Therefore: **Kc must be NEGATIVE** for stable level control

```cpp
// CORRECT for outlet valve level control
PIDController::Gains gains{
    -1.0,   // Kc: NEGATIVE (reverse-acting)
    10.0,   // tau_I
    0.0     // tau_D
};
```

### Symptoms of Wrong Control Action

If Kc has the wrong sign, the system will:
- Move in the opposite direction from the setpoint
- Saturate at limits (0% or 100% valve position)
- Never reach steady state
- Appear "unstable" or "runaway"

**If you observe these symptoms, check Kc sign FIRST before investigating other causes.**

---

## Key Implementation Points

- **Order matters:** Compute output using current integral state FIRST, then decide whether to update integral for next iteration
- **Conditional integration:** Only accumulate integral when output is not saturated (anti-windup)
- **Integral clamping:** Additional safety limit on integral state magnitude
- **Special cases:** tau_I = 0 disables integral; tau_D = 0 disables derivative
- **Reset on retune:** Call `reset()` when changing gains to clear accumulated history
- **Control action:** Use negative Kc for reverse-acting loops (see section above)

## Example Usage

```cpp
// Initialize: gains, 50% bias, output [0,1], max integral of 10
PIDController::Gains gains{.Kc = 1.5, .tau_I = 10.0, .tau_D = 2.0};
PIDController pid(gains, 0.5, 0.0, 1.0, 10.0);

// In control loop:
double error = setpoint - measured_value;
double error_dot = 0.0 - measured_rate;  // if setpoint constant
double control_output = pid.compute(error, error_dot, dt);

// Runtime retuning:
gains.Kc = 2.0;
pid.setGains(gains);
pid.reset();
```

## Related

- [[Model Class]]
- [[Stepper Class]]
- [[Simulation Architecture]]
