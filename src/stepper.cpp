#include "stepper.h"
#include <stdexcept>

namespace tank_sim {

/**
 * @brief Constructor to initialize the Stepper object.
 *
 * Allocates the GSL stepper using the RK4 algorithm and validates dimensions.
 *
 * @param state_dimension The size of the state vector for the differential equations
 * @param input_dimension The size of the input vector for the differential equations
 * @throws std::invalid_argument if either dimension is zero
 */
Stepper::Stepper(size_t state_dimension, size_t input_dimension) 
    : state_dimension_(state_dimension), input_dimension_(input_dimension) {
  // Validate dimensions
  if (state_dimension == 0) {
    throw std::invalid_argument("State dimension must be greater than zero");
  }
  if (input_dimension == 0) {
    throw std::invalid_argument("Input dimension must be greater than zero");
  }
  
  // Allocate the GSL stepper using the RK4 algorithm
  stepper_ = gsl_odeiv2_step_alloc(gsl_odeiv2_step_rk4, state_dimension);
  if (stepper_ == nullptr) {
    throw std::runtime_error("Failed to allocate GSL stepper");
  }
}

/**
 * @brief Destructor to free GSL resources.
 *
 * Frees the allocated GSL stepper resource.
 */
Stepper::~Stepper() {
  if (stepper_ != nullptr) {
    gsl_odeiv2_step_free(stepper_);
  }
}

// Structure to hold context for GSL callback
struct StepperContext {
  Stepper::DerivativeFunc *deriv_func;
  double t;
  const Eigen::VectorXd *input;
};

/**
 * @brief GSL-compatible derivative function wrapper.
 *
 * Converts the C array input to an Eigen vector, calls the user's derivative
 * function, and converts the result back to a C array.
 *
 * @param t Current time in the differential equation.
 * @param y Array of state variables at time t.
 * @param dydt Array where the derivative values are stored.
 * @param params Pointer to the StepperContext structure containing additional
 * parameters.
 * @return GSL_SUCCESS if successful, otherwise an error code.
 */
static int gsl_derivative_wrapper(double t, const double y[], double dydt[],
                                  void *params) {
  // Step 1: Unpack the context from the void pointer
  // params was set up in step() method and contains:
  // - deriv_func: pointer to the user's derivative function
  // - input: the input vector to pass to the derivative function
  StepperContext *ctx = static_cast<StepperContext *>(params);

  // Step 2: Convert C array y to an Eigen vector (wrap, don't copy)
  // y is a C array of doubles (state values)
  // Eigen::Map creates a vector view of this array without copying data
  // Size is determined by the input vector size (both should match)
  Eigen::VectorXd state =
      Eigen::Map<const Eigen::VectorXd>(y, ctx->input->size());

  // Step 3: Call the user's derivative function
  // Pass:
  // - t: current time
  // - state: the wrapped C array as an Eigen vector
  // - *ctx->input: the input vector (dereference the pointer)
  // Returns: an Eigen vector of derivatives
  Eigen::VectorXd derivative = (*ctx->deriv_func)(t, state, *ctx->input);

  // Step 4: Copy the Eigen derivative vector back into the C array dydt
  // derivative.data() is a pointer to the first element
  // derivative.data() + derivative.size() is a pointer past the last element
  // std::copy copies all elements into dydt (where GSL expects the result)
  std::copy(derivative.data(), derivative.data() + derivative.size(), dydt);

  // Step 5: Return success to GSL
  // GSL checks this return value to know if computation succeeded
  return GSL_SUCCESS;
}

/**
 * @brief Performs one step of the RK4 integration.
 *
 * Verifies that the state vector size matches the stepper dimension, creates a
 * context for the GSL callback, sets up the GSL system structure, allocates
 * arrays for integration, copies the input state to a C array, performs the RK4
 * step using GSL, converts the result back to an Eigen vector, and cleans up
 * resources.
 *
 * @param t Current time in the differential equation.
 * @param dt Time step size for the integration.
 * @param state Current state vector of the system.
 * @param input Input vector for the differential equations.
 * @param deriv_func Function pointer to the user's derivative function.
 * @return Eigen::VectorXd The updated state vector after one RK4 step.
 */
Eigen::VectorXd Stepper::step(double t, double dt, const Eigen::VectorXd &state,
                              const Eigen::VectorXd &input,
                              DerivativeFunc deriv_func) {
  // Step 1: Validate input dimensions
  if (state.size() != static_cast<int>(state_dimension_)) {
    throw std::runtime_error(
        "State vector size does not match stepper dimension");
  }
  if (input.size() != static_cast<int>(input_dimension_)) {
    throw std::runtime_error(
        "Input vector size does not match stepper dimension");
  }

  // Step 2: Create context structure for the GSL callback
  StepperContext ctx{&deriv_func, t, &input};

  // Step 3: Create and configure the GSL system structure
  gsl_odeiv2_system sys = {gsl_derivative_wrapper, nullptr, state_dimension_,
                           &ctx};

  // Step 4: Allocate C arrays for the integration
  // y: the state array - modified IN PLACE by gsl_odeiv2_step_apply
  // yerr: the error estimate array (we don't use it but GSL requires it)
  double *y = new double[state_dimension_];
  double *yerr = new double[state_dimension_];

  // Step 5: Copy the input Eigen vector into the C array y
  std::copy(state.data(), state.data() + state_dimension_, y);

  // Step 6: Perform one RK4 integration step using GSL
  // gsl_odeiv2_step_apply modifies y IN PLACE and returns error in yerr
  // Parameters:
  // - stepper_: the RK4 stepper we allocated in constructor
  // - t: current time
  // - h: the time step to take
  // - y: state array - INPUT and OUTPUT (modified in place!)
  // - yerr: error estimate array (output)
  // - dydt_in: derivative at current state (can be nullptr)
  // - dydt_out: derivative at new state (can be nullptr)
  // - sys: the system of ODEs
  int status = gsl_odeiv2_step_apply(stepper_, t, dt, y, yerr, 
                                      nullptr, nullptr, &sys);

  // Step 7: Check for errors from GSL
  if (status != GSL_SUCCESS) {
    delete[] y;
    delete[] yerr;
    throw std::runtime_error("GSL RK4 step failed");
  }

  // Step 8: Copy the result to an Eigen vector
  // y now contains the updated state after the step
  Eigen::VectorXd result(state_dimension_);
  std::copy(y, y + state_dimension_, result.data());

  // Step 9: Free the dynamically allocated C arrays
  delete[] y;
  delete[] yerr;

  // Step 10: Return the result
  return result;
}

} // namespace tank_sim
