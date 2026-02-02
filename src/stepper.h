#pragma once

#include <Eigen/Dense>
#include <cstddef>
#include <functional>
#include <gsl/gsl_odeiv2.h>
#include <gsl/gsl_errno.h>


namespace tank_sim {

/**
 * @class Stepper
 * @brief Wrapper around GSL's RK4 ODE integrator for fixed-step integration.
 *
 * This class encapsulates the GNU Scientific Library (GSL) RK4 (Runge-Kutta 4th
 * order) integrator, providing a C++ interface for integrating systems of ODEs:
 *
 *   dy/dt = f(t, y, u)
 *
 * where y is the state vector, u is the input vector, and t is time.
 *
 * ## Memory Management Strategy
 *
 * The Stepper manages GSL's `gsl_odeiv2_step` resource, which is a C structure
 * that cannot be safely copied. The GSL stepper maintains internal state
 * pointers and algorithm-specific data that are tightly coupled to the original
 * allocation. Copying would result in:
 *
 * - Multiple objects pointing to the same GSL resource (use-after-free)
 * - Double-free errors when destructors run
 * - Corrupted integration state if two Steppers try to use the same resource
 *
 * To prevent these issues, Stepper implements the Rule of Five:
 *
 * 1. **Explicit Destructor**: Frees the GSL stepper with gsl_odeiv2_step_free()
 * 2. **Deleted Copy Constructor**: Prevents implicit copying
 * 3. **Deleted Copy Assignment**: Prevents implicit assignment
 * 4. **Implicitly-Deleted Move Constructor**: Move operations not implemented
 *    (could be added in future if needed for container compatibility)
 * 5. **Implicitly-Deleted Move Assignment**: Move operations not implemented
 *
 * This means:
 * - Stepper objects CANNOT be copied or moved
 * - Pass Stepper by pointer or reference, never by value
 * - Store Stepper in containers using pointers (e.g., std::unique_ptr<Stepper>)
 * - Cannot be returned by value from functions
 *
 * ## GSL Integration Details
 *
 * The step() method:
 * - Allocates temporary C arrays for GSL's algorithm
 * - Wraps these arrays with Eigen::Map for zero-copy interop with the
 *   user-provided derivative function
 * - Uses the gsl_derivative_wrapper callback to convert between C and C++
 * - Frees temporary arrays immediately after the step completes
 *
 * All GSL resource management follows RAII principles: resources are acquired
 * in the constructor and released in the destructor, ensuring exception safety.
 */
class Stepper {
public:
  using DerivativeFunc = std::function<Eigen::VectorXd(
      double, const Eigen::VectorXd &, const Eigen::VectorXd &)>;

public:
  /**
   * @brief Constructs a Stepper with the given state and input dimensions.
   *
   * Allocates an RK4 stepper from GSL for the given state dimension. The input
   * dimension is stored for runtime validation in step().
   *
   * @param state_dimension Number of state variables (must be > 0)
   * @param input_dimension Number of input variables (must be > 0)
   *
   * @throws std::invalid_argument if either dimension is zero
   * @throws std::runtime_error if GSL allocation fails
   *
   * @note The input dimension is not used directly by GSL but is validated
   *       at runtime to ensure consistent vector sizes during integration.
   */
  Stepper(size_t state_dimension, size_t input_dimension);

  /**
   * @brief Destructor that releases the GSL stepper resource.
   *
   * Calls gsl_odeiv2_step_free() to deallocate the internal GSL stepper,
   * ensuring proper cleanup of the C resource.
   *
   * @note The destructor is exception-safe and will not throw.
   */
  ~Stepper();

  // Copy operations are deleted because GSL resources cannot be safely copied.
  // See "Memory Management Strategy" in class documentation.
  Stepper(const Stepper &) = delete;
  Stepper &operator=(const Stepper &) = delete;

  /**
   * @brief Performs one RK4 integration step.
   *
   * Advances the system state by one time step using the 4th-order Runge-Kutta
   * method. The derivative function is called internally by the GSL stepper at
   * intermediate points during the step.
   *
   * ## Behavior
   *
   * Given state y, input u, and time t, computes the new state y' such that:
   *
   *   y' ≈ y + ∫[t, t+dt] f(τ, y(τ), u) dτ
   *
   * using 4th-order Runge-Kutta integration.
   *
   * ## Input Validation
   *
   * This method validates that:
   * - state.size() == state_dimension (set in constructor)
   * - input.size() == input_dimension (set in constructor)
   *
   * Mismatched sizes indicate a programming error (e.g., wrong state vector
   * passed, or Model interface change not reflected in controller code).
   *
   * @param t Current time in the differential equation
   * @param dt Time step size for integration (typically positive)
   * @param state Current state vector of the system
   * @param input Input vector for the derivative function
   * @param deriv_func Callable that computes y' = f(t, y, u)
   *
   * @return The updated state vector after the RK4 step
   *
   * @throws std::runtime_error if state or input dimensions don't match
   * @throws std::runtime_error if GSL integration fails
   *
   * @note The input state and input vectors are not modified.
   * @note The derivative function may be called multiple times (RK4 requires
   *       4 evaluations per step).
   * @note This method is exception-safe: if an exception is thrown, the
   *       Stepper remains in a valid state and can be used again.
   */
  Eigen::VectorXd step(double t, double dt, const Eigen::VectorXd &state,
                       const Eigen::VectorXd &input, DerivativeFunc deriv_func);

private:
  gsl_odeiv2_step *stepper_;      ///< GSL RK4 stepper (managed, freed in ~Stepper)
  size_t state_dimension_;        ///< Cached state vector size for validation
  size_t input_dimension_;        ///< Cached input vector size for validation
};

} // namespace tank_sim
