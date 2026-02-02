#include "tank_model.h"
#include <cmath>
#include <stdexcept>

namespace tank_sim {

TankModel::TankModel(const Parameters& params)
    : area_(params.area),
      k_v_(params.k_v),
      max_height_(params.max_height) {
    // Validate parameters immediately - fail fast
    if (area_ <= 0.0) {
        throw std::invalid_argument("Tank area must be positive");
    }
    if (k_v_ <= 0.0) {
        throw std::invalid_argument("Valve coefficient must be positive");
    }
    if (max_height_ <= 0.0) {
        throw std::invalid_argument("Maximum tank height must be positive");
    }
}

Eigen::VectorXd TankModel::derivatives(
    const Eigen::VectorXd& state,
    const Eigen::VectorXd& inputs) const {
    
    // Validate preconditions (debug mode only)
    assert(state.size() == 1 && "State vector must have size 1");
    assert(inputs.size() == 2 && "Input vector must have size 2");
    
    double h = state(0);              // Current tank level (m)
    double q_in = inputs(0);          // Inlet flow rate (m³/s)
    double valve_position = inputs(1); // Valve position (0 to 1)
    
    // Validate preconditions
    assert(h >= 0.0 && "Tank level must be non-negative");
    assert(valve_position >= 0.0 && valve_position <= 1.0 && 
           "Valve position must be in [0, 1]");
    
    // Calculate outlet flow using valve equation
    double q_out = outletFlow(h, valve_position);
    
    // Material balance equation: dh/dt = (q_in - q_out) / A
    double dh_dt = (q_in - q_out) / area_;
    
    // Return derivative vector
    Eigen::VectorXd derivative(1);
    derivative(0) = dh_dt;
    return derivative;
}

double TankModel::getOutletFlow(
    const Eigen::VectorXd& state,
    const Eigen::VectorXd& inputs) const {
    
    double h = state(0);
    double valve_position = inputs(1);
    return outletFlow(h, valve_position);
}

double TankModel::outletFlow(double h, double valve_position) const {
    // Validate preconditions
    assert(h >= 0.0 && "Tank level must be non-negative");
    assert(valve_position >= 0.0 && valve_position <= 1.0 && 
           "Valve position must be in [0, 1]");
    
    // No flow if tank is empty
    if (h <= 0.0) {
        return 0.0;
    }
    
    // Valve flow equation: q_out = k_v * x * sqrt(h)
    return k_v_ * valve_position * std::sqrt(h);
}

}  // namespace tank_sim
