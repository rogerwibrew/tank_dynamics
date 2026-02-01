#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

namespace tank_sim {
    /**
     * @brief Proportional-Integral-Derivative (PID) controller with anti-windup.
     *
     * Implements a discrete-time PID controller with saturation and anti-windup.
     * Tracks the integral of error over time and prevents integral windup during
     * output saturation. Supports dynamic tuning and configurable output limits.
     */
    class PIDController {
    public:
        /**
         * @brief Struct containing PID controller gain parameters.
         */
        struct Gains {
            double Kc;      // Proportional gain (dimensionless)
            double tau_I;   // Integral time constant (seconds), 0 = no integral action
            double tau_D;   // Derivative time constant (seconds), 0 = no derivative action
        };

        /**
         * @brief Construct a new PIDController object.
         *
         * @param gains The initial controller gains.
         * @param bias Output bias when error is zero.
         * @param min_output Minimum output saturation limit.
         * @param max_output Maximum output saturation limit.
         * @param max_integral Maximum magnitude for integral state clamping.
         */
        PIDController(const Gains& gains, double bias, double min_output,
                      double max_output, double max_integral);

        /**
         * @brief Compute the control output based on error signals.
         *
         * Implements discrete PID with anti-windup: integral only accumulates when
         * output is not saturated. Output is clamped to [min_output, max_output].
         *
         * @param error Current error (setpoint - measured value).
         * @param error_dot Rate of change of error.
         * @param dt Time step in seconds.
         * @return Control output clamped to [min_output, max_output].
         */
        double compute(double error, double error_dot, double dt);

        /**
         * @brief Update the controller gains dynamically.
         *
         * Changes Kc, tau_I, and tau_D without resetting integral state
         * (allows bumpless transfer).
         *
         * @param gains New controller gains.
         */
        void setGains(const Gains& gains);

        /**
         * @brief Change the output saturation limits.
         *
         * @param min_val New minimum output limit.
         * @param max_val New maximum output limit.
         */
        void setOutputLimits(double min_val, double max_val);

        /**
         * @brief Reset the integral state to zero.
         *
         * Call this when retuning or initializing.
         */
        void reset();

        /**
         * @brief Get the current integral accumulator state.
         *
         * @return Current integral accumulation value.
         */
        double getIntegralState() const;

    private:
        Gains gains;
        double bias;
        double min_output;
        double max_output;
        double max_integral;
        double integral_state;
    };
}

#endif // PID_CONTROLLER_H
