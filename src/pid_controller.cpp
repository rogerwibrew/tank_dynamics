#include "pid_controller.h"
#include <algorithm>
#include <stdexcept>

namespace tank_sim {

PIDController::PIDController(const Gains& gains, double bias, double min_output,
                             double max_output, double max_integral)
    : gains(gains), bias(bias), min_output(min_output), max_output(max_output),
      max_integral(max_integral), integral_state(0.0) {
    // Validate parameters - fail fast
    if (gains.tau_I < 0.0) {
        throw std::invalid_argument("Integral time constant (tau_I) cannot be negative");
    }
    if (gains.tau_D < 0.0) {
        throw std::invalid_argument("Derivative time constant (tau_D) cannot be negative");
    }
    if (min_output > max_output) {
        throw std::invalid_argument("min_output must be <= max_output");
    }
    if (max_integral < 0.0) {
        throw std::invalid_argument("max_integral must be non-negative");
    }
}

double PIDController::compute(double error, double error_dot, double dt) {
    // Step 1: Calculate proportional term
    double p_term = error;

    // Step 2: Calculate integral term using CURRENT state
    double i_term = 0.0;
    if (gains.tau_I != 0.0) {
        i_term = (1.0 / gains.tau_I) * integral_state;
    }

    // Step 3: Calculate derivative term
    double d_term = gains.tau_D * error_dot;

    // Step 4: Compute unsaturated output
    double output_unsat = bias + gains.Kc * (p_term + i_term + d_term);

    // Step 5: Clamp to physical limits
    double output = std::clamp(output_unsat, min_output, max_output);

    // Step 6: Anti-windup: Update integral for NEXT timestep only if NOT saturated
    // Check if output was saturated (output differs from unsaturated value)
    bool saturated = (output_unsat < min_output || output_unsat > max_output);
    if (!saturated) {
        // Only accumulate integral when we're in the linear operating range
        integral_state = integral_state + error * dt;
        // Also clamp integral state directly (secondary safety limit)
        integral_state = std::clamp(integral_state, -max_integral, max_integral);
    }

    return output;
}

void PIDController::setGains(const Gains& gains) {
    this->gains = gains;
}

void PIDController::setOutputLimits(double min_val, double max_val) {
    min_output = min_val;
    max_output = max_val;
}

void PIDController::reset() {
    integral_state = 0.0;
}

double PIDController::getIntegralState() const {
    return integral_state;
}

}  // namespace tank_sim
