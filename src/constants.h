#ifndef TANK_DYNAMICS_CONSTANTS_H
#define TANK_DYNAMICS_CONSTANTS_H

/**
 * @file constants.h
 * @brief Central definition of all numerical constants and configuration values
 *
 * This header consolidates magic numbers throughout the tank dynamics simulation
 * project, improving maintainability and providing a single source of truth for
 * system parameters. All constants use modern C++ constexpr for compile-time
 * evaluation and type safety.
 *
 * Constants are organized into logical categories:
 * - System Architecture: Fixed dimensions of the simulation
 * - Physical Parameters: Tank model properties and fluid dynamics
 * - Integration Parameters: RK4 stepper configuration and validation
 * - Control System: PID controller defaults
 * - Numerical Tolerances: For testing and validation
 */

namespace tank_sim::constants {

// ============================================================================
// SYSTEM ARCHITECTURE
// ============================================================================

/**
 * @brief Number of state variables in the tank model
 *
 * The tank model has a single state: liquid height [h] in meters.
 * Fixed by the physics model design.
 */
constexpr int TANK_STATE_SIZE = 1;

/**
 * @brief Number of input variables to the tank model
 *
 * Two inputs control the system:
 *  - q_in: Inlet volumetric flow rate (m³/s)
 *  - x: Valve position, normalized to [0, 1] where 1 = fully open
 */
constexpr int TANK_INPUT_SIZE = 2;

// Index constants for clarity when accessing input arrays
constexpr int INPUT_INDEX_INLET_FLOW = 0;  ///< Index for q_in in input vector
constexpr int INPUT_INDEX_VALVE_POSITION = 1;  ///< Index for x in input vector

// ============================================================================
// PHYSICAL PARAMETERS
// ============================================================================

/**
 * @brief Cross-sectional area of the cylindrical tank
 *
 * Unit: m²
 * Used in the material balance equation: dh/dt = (q_in - q_out) / A
 * Default value based on typical industrial tank dimensions.
 */
constexpr double DEFAULT_TANK_AREA = 120.0;

/**
 * @brief Valve flow coefficient
 *
 * Unit: m^2.5/s (converts sqrt(h) to volumetric flow rate)
 * Empirical parameter from valve characterization: q_out = k_v * x * sqrt(h)
 * Where x is valve position [0, 1] and h is liquid height [m]
 * Default: 1.2649 m^2.5/s
 */
constexpr double DEFAULT_VALVE_COEFFICIENT = 1.2649;

/**
 * @brief Maximum liquid height in the tank
 *
 * Unit: m
 * Physical constraint representing the tank geometry.
 * Used for state validation and control saturation.
 */
constexpr double TANK_MAX_HEIGHT = 5.0;

/**
 * @brief Nominal/equilibrium tank level for baseline operation
 *
 * Unit: m
 * A representative operating point used in tests and commissioning.
 * At this height with standard conditions, the system demonstrates
 * typical material balance dynamics.
 */
constexpr double TANK_NOMINAL_HEIGHT = 2.5;

/**
 * @brief Standard gravitational acceleration
 *
 * Unit: m/s²
 * Physical constant included for completeness; not currently used in the
 * tank model but may be relevant for future hydrostatic pressure calculations.
 */
constexpr double GRAVITY = 9.81;

// ============================================================================
// INTEGRATION PARAMETERS (RK4 Stepper)
// ============================================================================

/**
 * @brief Minimum allowable time step for integration
 *
 * Unit: seconds
 * Prevents numerical instability from excessively small steps.
 * dt < 0.001 s may introduce floating-point precision issues.
 * Validated in Simulator::validateTimestep()
 */
constexpr double MIN_DT = 0.001;

/**
 * @brief Maximum allowable time step for integration
 *
 * Unit: seconds
 * Prevents loss of accuracy and instability from overly large steps.
 * dt > 10.0 s is inappropriate for the tank time constants.
 * Validated in Simulator::validateTimestep()
 */
constexpr double MAX_DT = 10.0;

/**
 * @brief Minimum expected error ratio for RK4 convergence validation
 *
 * Unitless ratio used to verify RK4 order of accuracy.
 * When comparing coarse step (dt_c) vs. fine step (dt_f) integration:
 * Expected error ratio ≈ (dt_c / dt_f)^4 ≈ 16 for RK4
 * Min threshold: 12.0 (allows some numerical noise)
 * Test: test_stepper.cpp::TestStepperAccuracy
 */
constexpr double RK4_MIN_ERROR_RATIO = 12.0;

/**
 * @brief Maximum expected error ratio for RK4 convergence validation
 *
 * Unitless ratio used to verify RK4 order of accuracy.
 * Max threshold: 20.0 (accounts for integration variations)
 * Helps detect if the integrator is not behaving as an RK4 method.
 * Test: test_stepper.cpp::TestStepperAccuracy
 */
constexpr double RK4_MAX_ERROR_RATIO = 20.0;

/**
 * @brief Default time step for simulation
 *
 * Unit: seconds
 * A reasonable middle-ground step size for most tank dynamics scenarios.
 * Provides good accuracy-to-computation-cost ratio.
 */
constexpr double DEFAULT_DT = 0.1;

// ============================================================================
// CONTROL SYSTEM (PID Controller)
// ============================================================================

/**
 * @brief Default proportional gain for PID controller
 *
 * Unitless
 * Controls responsiveness to current error signal.
 * Typical value: 1.0 (proportional output = Kc * error)
 * Test baseline: test_pid_controller.cpp
 */
constexpr double DEFAULT_PID_PROPORTIONAL_GAIN = 1.0;

/**
 * @brief Default integral time constant for PID controller
 *
 * Unit: seconds
 * Large value (10.0 s) means minimal integral action by default.
 * Prevents steady-state offset but requires tuning for specific applications.
 * tau_I = 0 disables integral action (pure P control)
 */
constexpr double DEFAULT_PID_INTEGRAL_TIME = 10.0;

/**
 * @brief Default derivative time constant for PID controller
 *
 * Unit: seconds
 * Controls damping via derivative of error.
 * tau_D = 0 disables derivative action (PI or P control)
 * tau_D = 5.0 s provides typical damping for tank systems.
 */
constexpr double DEFAULT_PID_DERIVATIVE_TIME = 5.0;

/**
 * @brief Default bias/offset for PID controller output
 *
 * Unitless, typically in [0, 1] range
 * Added to computed control signal: u = bias + Kc*(e + integral + derivative)
 * Default 0.5 represents 50% nominal operating point for valve position.
 * Helps maintain flow balance without excessive error.
 */
constexpr double DEFAULT_PID_BIAS = 0.5;

/**
 * @brief Minimum saturation limit for PID controller output
 *
 * Unitless, typically 0.0
 * Prevents control signal from going negative (e.g., negative valve position).
 * Hard lower bound on actuator output.
 */
constexpr double DEFAULT_PID_MIN_OUTPUT = 0.0;

/**
 * @brief Maximum saturation limit for PID controller output
 *
 * Unitless, typically 1.0
 * Prevents control signal from exceeding physical limits (e.g., valve fully open).
 * Hard upper bound on actuator output.
 */
constexpr double DEFAULT_PID_MAX_OUTPUT = 1.0;

/**
 * @brief Maximum integral accumulation for anti-windup protection
 *
 * Unit: (same as error units * tau_I)
 * Limits how large the integral term can grow during sustained error.
 * Prevents integrator windup when system is saturated.
 * Default: 10.0 × proportional error = reasonable limit for most tuning.
 */
constexpr double DEFAULT_PID_MAX_INTEGRAL = 10.0;

/**
 * @brief Default discrete time step for PID controller
 *
 * Unit: seconds
 * Time interval between control updates.
 * Typically matches or is a multiple of the simulation time step.
 */
constexpr double DEFAULT_PID_DT = 1.0;

// ============================================================================
// NUMERICAL TOLERANCES (Testing and Validation)
// ============================================================================

/**
 * @brief Tolerance for derivative calculations in testing
 *
 * Unit: (varies by quantity being tested)
 * Used in EXPECT_NEAR() assertions when checking computed derivatives.
 * Example: Oscillator velocity derivatives checked to within ±0.001
 * Test: test_stepper.cpp::TestHarmonicOscillator
 */
constexpr double DERIVATIVE_TOLERANCE = 0.001;

/**
 * @brief Tolerance for integration accuracy verification
 *
 * Unit: (varies by quantity)
 * Used when comparing numerical integration results against analytical solutions.
 * Tighter than DERIVATIVE_TOLERANCE for end-state verification.
 * Example: Step response position checked to ±0.0001
 * Test: test_stepper.cpp
 */
constexpr double INTEGRATION_TOLERANCE = 0.0001;

/**
 * @brief Tolerance for oscillator position in harmonic tests
 *
 * Unit: meters (or appropriate state units)
 * Position of a test oscillator must match analytical solution within this bound.
 * Allows for small integration errors accumulated over multiple steps.
 * Test: test_stepper.cpp::TestHarmonicOscillator
 */
constexpr double OSCILLATOR_POSITION_TOLERANCE = 0.001;

/**
 * @brief Tolerance for oscillator velocity in harmonic tests
 *
 * Unit: m/s (or appropriate state derivative units)
 * Velocity of a test oscillator must match analytical solution within this bound.
 * Slightly tighter tolerance than position (velocity is more sensitive to phase).
 * Test: test_stepper.cpp::TestHarmonicOscillator
 */
constexpr double OSCILLATOR_VELOCITY_TOLERANCE = 0.01;

/**
 * @brief General tolerance for tank model state validation
 *
 * Unit: meters (liquid height)
 * Used in test assertions for tank model derivatives and state evolution.
 * Example: Material balance errors checked to within ±0.001 m/s
 * Test: test_tank_model.cpp
 */
constexpr double TANK_STATE_TOLERANCE = 0.001;

/**
 * @brief Tolerance for PID controller output validation
 *
 * Unit: normalized [0, 1]
 * Used in test assertions when checking control signal computation.
 * Accounts for floating-point rounding in accumulated calculations.
 * Test: test_pid_controller.cpp
 */
constexpr double CONTROL_OUTPUT_TOLERANCE = 0.001;

// ============================================================================
// TEST-SPECIFIC PARAMETERS
// ============================================================================

/**
 * @brief Standard test error value for PID step response
 *
 * Unit: meters (or state units)
 * Typical error signal magnitude used in PID controller unit tests.
 * Represents a small deviation from setpoint.
 */
constexpr double TEST_ERROR_VALUE = 0.1;

/**
 * @brief Standard test time step for discrete PID updates
 *
 * Unit: seconds
 * Synchronizes discrete-time PID calculations in test scenarios.
 * Typical sampling interval for control logic.
 */
constexpr double TEST_DT = 1.0;

/**
 * @brief Standard inlet flow rate for tank model tests
 *
 * Unit: m³/s
 * Represents typical inflow condition in unit tests.
 * Chosen to maintain reasonable tank levels without extreme dynamics.
 */
constexpr double TEST_INLET_FLOW = 1.0;

/**
 * @brief Standard valve position for tank model tests
 *
 * Unit: normalized [0, 1]
 * Represents partial valve opening (50%) in test scenarios.
 * Allows material balance testing with non-zero outlet flow.
 */
constexpr double TEST_VALVE_POSITION = 0.5;

/**
 * @brief Test oscillator frequency
 *
 * Unit: Hz
 * Used in harmonic oscillator validation tests.
 * Equivalent to angular frequency ω = 2π rad/s ≈ 6.283 rad/s
 * Test: test_stepper.cpp::TestHarmonicOscillator
 */
constexpr double TEST_OSCILLATOR_FREQUENCY = 1.0;

/**
 * @brief Coarse integration time step for RK4 convergence testing
 *
 * Unit: seconds
 * Used in step doubling verification to test RK4 error scaling.
 * Larger step size for initial comparison.
 */
constexpr double TEST_RK4_DT_COARSE = 0.1;

/**
 * @brief Fine integration time step for RK4 convergence testing
 *
 * Unit: seconds
 * Used in step doubling verification to test RK4 error scaling.
 * Smaller step size (half of coarse) for refined comparison.
 */
constexpr double TEST_RK4_DT_FINE = 0.05;

/**
 * @brief Number of integration steps for standard test
 *
 * Unitless count
 * Determines integration duration: total_time = num_steps * dt
 * With DEFAULT_DT = 0.1, 10 steps = 1.0 second simulation.
 */
constexpr int TEST_NUM_STEPS = 10;

/**
 * @brief Number of fine steps for RK4 convergence verification
 *
 * Unitless count
 * Used when comparing coarse vs. fine integration: 20 fine steps vs 10 coarse.
 * Maintains same total simulation time with finer resolution.
 */
constexpr int TEST_NUM_STEPS_FINE = 20;

// ============================================================================
// PHYSICS CONSTANTS
// ============================================================================

/**
 * @brief Pi constant for circular/oscillatory calculations
 *
 * Unitless
 * 2π appears in angular frequency conversions (ω = 2π*f).
 * Precomputed for convenience in test calculations.
 */
constexpr double TWO_PI = 6.283185307179586;

}  // namespace tank_sim::constants

#endif  // TANK_DYNAMICS_CONSTANTS_H
