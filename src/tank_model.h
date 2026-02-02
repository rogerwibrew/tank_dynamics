#ifndef TANK_SIM_TANK_MODEL_H
#define TANK_SIM_TANK_MODEL_H

#include <Eigen/Dense>

namespace tank_sim {

/**
 * @brief Physics model for a tank with inlet/outlet flows.
 * 
 * TankModel represents the material balance of a liquid tank with variable inlet flow
 * and outlet flow controlled by a valve. This is a stateless model that computes
 * derivatives suitable for numerical integration.
 * 
 * The model implements:
 * - Material balance: dh/dt = (q_in - q_out) / A
 * - Valve equation: q_out = k_v * x * sqrt(h)
 * 
 * @note This class is stateless - it computes derivatives without maintaining
 *       internal state. Integration is handled externally by the Stepper class.
 */
class TankModel {
public:
    /**
     * @brief Configuration parameters for the tank model.
     */
    struct Parameters {
        double area;          ///< Cross-sectional area (m²)
        double k_v;           ///< Valve coefficient (m^2.5/s)
        double max_height;    ///< Maximum tank height (m)
    };

    /**
     * @brief Constructs a TankModel with the given parameters.
     * 
     * @param params Physical parameters of the tank system
     */
    explicit TankModel(const Parameters& params);

    /**
     * @brief Computes the derivative of tank level (dh/dt).
     * 
     * Calculates the rate of change of water level based on the material balance
     * equation: dh/dt = (q_in - q_out) / A
     * 
     * @param state Current state vector [h] where h is tank level (m)
     * @param inputs Input vector [q_in, x] where:
     *               - q_in is inlet flow rate (m³/s)
     *               - x is valve position (dimensionless, 0 to 1)
     * @return Derivative vector [dh/dt] in m/s
     * 
     * @pre state(0) >= 0 (tank level must be non-negative)
     * @pre inputs(1) in [0, 1] (valve position must be in valid range)
     * 
     * @note This model is **time-invariant**: the dynamics depend only on the
     *       current state and inputs, not on absolute time. The time parameter
     *       passed by the Stepper during integration is not used. This is
     *       appropriate for physical systems without explicitly time-dependent
     *       terms (e.g., no scheduled valve changes, no time-dependent flows).
     *       If time-dependent behavior is needed in the future, add a time
     *       parameter here and update the model equations accordingly.
     */
    Eigen::VectorXd derivatives(
        const Eigen::VectorXd& state,
        const Eigen::VectorXd& inputs) const;

    /**
     * @brief Gets the current outlet flow rate for reporting/logging.
     * 
     * Public getter that wraps the internal valve flow calculation.
     * Use this to report outlet flow to the frontend/SCADA system.
     * 
     * @param state Current state vector [h] where h is tank level (m)
     * @param inputs Input vector [q_in, x] where x is valve position
     * @return Outlet flow rate (m³/s)
     */
    double getOutletFlow(
        const Eigen::VectorXd& state,
        const Eigen::VectorXd& inputs) const;

private:
    double area_;         ///< Cross-sectional area (m²)
    double k_v_;          ///< Valve coefficient (m^2.5/s)
    double max_height_;   ///< Maximum tank height (m)

    /**
     * @brief Internal algebraic equation: calculates outlet flow through valve.
     * 
     * Implements the valve flow equation: q_out = k_v * x * sqrt(h)
     * Returns zero if tank level is zero or negative.
     * 
     * @param h Current tank level (m)
     * @param x Valve position (dimensionless, 0 to 1)
     * @return Outlet flow rate (m³/s)
     */
    double outletFlow(double h, double x) const;
};

}  // namespace tank_sim

#endif  // TANK_SIM_TANK_MODEL_H
