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
         * Implements the discrete-time PID control law with anti-windup:
         *
         *   u = bias + Kc * (error + (1/tau_I) * integral + tau_D * error_dot)
         *
         * where:
         * - Kc is the proportional gain (dimensionless)
         * - (1/tau_I) is the integral gain (1/seconds); set tau_I=0 to disable
         * - tau_D is the derivative gain (seconds); set tau_D=0 to disable
         *
         * ## Anti-Windup
         *
         * The integral accumulates only when output is not saturated:
         *
         *   integral += error * dt  (only if min_output < u_unsat < max_output)
         *
         * This prevents the integral term from growing when the controller cannot
         * affect the system due to physical constraints (actuator limits).
         *
         * ## Output Clamping
         *
         * Final output is clamped to [min_output, max_output]. The integral state
         * is separately clamped to ±max_integral.
         *
         * @param error Current error (setpoint - measured value)
         * @param error_dot Rate of change of error (for derivative term)
         * @param dt Time step in seconds
         * @return Control output clamped to [min_output, max_output]
         *
         * @note If tau_I = 0, integral action is disabled and integral state
         *       remains at its current value (typically zero after reset()).
         * @note If tau_D = 0, derivative action is disabled and error_dot is ignored.
         * @note This method is stateful: it updates internal integral_state based on
         *       the error and saturation condition. Call reset() to clear the state.
         */
        double compute(double error, double error_dot, double dt);

        /**
         * @brief Update the controller gains dynamically.
         *
         * Changes Kc (proportional gain), tau_I (integral time constant), and
         * tau_D (derivative time constant) without resetting integral state.
         * This enables bumpless transfer when retuning the controller.
         *
         * @param gains New controller gains with updated Kc, tau_I, tau_D
         *
         * @note The integral_state is NOT reset, allowing smooth gain transitions
         *       without an output step. However, if tuning significantly changes
         *       the integral time constant, consider calling reset() first.
         *
         * @throws std::invalid_argument if tau_I < 0 or tau_D < 0 (checked in constructor)
         */
        void setGains(const Gains& gains);

        /**
         * @brief Change the output saturation limits.
         *
         * Updates the range [min_output, max_output] that the control output is
         * clamped to. This is useful for adjusting the effective actuator range
         * during runtime.
         *
         * @param min_val New minimum output limit
         * @param max_val New maximum output limit
         *
         * @note The integral state is NOT reset when limits change. If you change
         *       limits and want to clear accumulated integral error, call reset().
         * @note max_val should be >= min_val; if not, behavior is undefined.
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
