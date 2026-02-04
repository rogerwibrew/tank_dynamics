#include <gtest/gtest.h>
#include <Eigen/Dense>
#include <cmath>
#include "../src/tank_model.h"
#include "../src/constants.h"

using namespace tank_sim;
using namespace tank_sim::constants;

class TankModelTest : public ::testing::Test {
protected:
    // Standard test parameters from the plan
    // Using C++17 positional initialization (area, k_v, max_height)
    //
    // These parameters are chosen such that at h = TANK_NOMINAL_HEIGHT with valve position
    // x = TEST_VALVE_POSITION, the outlet flow equals TEST_INLET_FLOW (steady state):
    //   q_out = k_v * x * sqrt(h) = DEFAULT_VALVE_COEFFICIENT * TEST_VALVE_POSITION * sqrt(TANK_NOMINAL_HEIGHT) ≈ TEST_INLET_FLOW m³/s
    // This allows tests to verify material balance (dh/dt ≈ 0) at steady state.
    TankModel::Parameters params{
        DEFAULT_TANK_AREA,              // area: cross-sectional area (m²)
        DEFAULT_VALVE_COEFFICIENT,      // k_v: valve coefficient (m^2.5/s), chosen for steady-state testing
        TANK_MAX_HEIGHT                 // max_height: maximum tank height (m)
    };
    
    TankModel model{params};
};

// Test: Steady State Zero Derivative
TEST_F(TankModelTest, SteadyStateZeroDerivative) {
    // At steady state with inlet = TEST_INLET_FLOW, valve = TEST_VALVE_POSITION, level = TANK_NOMINAL_HEIGHT
    // outlet flow should equal inlet flow, so derivative should be zero
    
    Eigen::VectorXd state(1);
    state << TANK_NOMINAL_HEIGHT;  // tank level in meters
    
    Eigen::VectorXd inputs(2);
    inputs << TEST_INLET_FLOW,     // inlet flow (cubic meters per second)
              TEST_VALVE_POSITION; // valve position (normalized to [0,1])
    
    Eigen::VectorXd derivative = model.derivatives(state, inputs);
    
    // At steady state: q_in = q_out, so dh/dt should be ~0
    EXPECT_NEAR(derivative(0), 0.0, TANK_STATE_TOLERANCE);
}

// Test: Positive Derivative When Inlet Exceeds Outlet
TEST_F(TankModelTest, PositiveDerivativeWhenInletExceedsOutlet) {
    Eigen::VectorXd state(1);
    state << TANK_NOMINAL_HEIGHT;  // tank level in meters
    
    Eigen::VectorXd inputs(2);
    inputs << 1.5,                  // inlet flow: higher than nominal
              TEST_VALVE_POSITION;  // valve position
    
    Eigen::VectorXd derivative = model.derivatives(state, inputs);
    
    // Calculate expected outlet flow: k_v * x * sqrt(h)
    double expected_outlet = DEFAULT_VALVE_COEFFICIENT * TEST_VALVE_POSITION * std::sqrt(TANK_NOMINAL_HEIGHT);
    double expected_derivative = (1.5 - expected_outlet) / DEFAULT_TANK_AREA;
    
    EXPECT_GT(derivative(0), 0.0);  // Should be positive (tank filling)
    EXPECT_NEAR(derivative(0), expected_derivative, TANK_STATE_TOLERANCE);
}

// Test: Negative Derivative When Outlet Exceeds Inlet
TEST_F(TankModelTest, NegativeDerivativeWhenOutletExceedsInlet) {
    Eigen::VectorXd state(1);
    state << TANK_NOMINAL_HEIGHT;  // tank level in meters
    
    Eigen::VectorXd inputs(2);
    inputs << 0.5,                  // inlet flow: lower than nominal
              TEST_VALVE_POSITION;  // valve position
    
    Eigen::VectorXd derivative = model.derivatives(state, inputs);
    
    // Calculate expected outlet flow
    double expected_outlet = DEFAULT_VALVE_COEFFICIENT * TEST_VALVE_POSITION * std::sqrt(TANK_NOMINAL_HEIGHT);
    double expected_derivative = (0.5 - expected_outlet) / DEFAULT_TANK_AREA;
    
    EXPECT_LT(derivative(0), 0.0);  // Should be negative (tank draining)
    EXPECT_NEAR(derivative(0), expected_derivative, TANK_STATE_TOLERANCE);
}

// Test: Outlet Flow Calculation via Public Getter
TEST_F(TankModelTest, OutletFlowCalculation) {
    Eigen::VectorXd state(1);
    state << TANK_NOMINAL_HEIGHT;  // tank level in meters
    
    Eigen::VectorXd inputs(2);
    inputs << TEST_INLET_FLOW,     // inlet flow (not used for outlet calculation)
              TEST_VALVE_POSITION; // valve position
    
    double outlet_flow = model.getOutletFlow(state, inputs);
    
    // Expected: k_v * x * sqrt(h)
    double expected = DEFAULT_VALVE_COEFFICIENT * TEST_VALVE_POSITION * std::sqrt(TANK_NOMINAL_HEIGHT);
    
    EXPECT_NEAR(outlet_flow, expected, TANK_STATE_TOLERANCE);
}

// Test: Zero Outlet Flow When Valve Closed
TEST_F(TankModelTest, ZeroOutletFlowWhenValveClosed) {
    Eigen::VectorXd state(1);
    state << TANK_MAX_HEIGHT;  // any positive level
    
    Eigen::VectorXd inputs(2);
    inputs << TEST_INLET_FLOW,  // inlet flow
              0.0;              // valve fully closed
    
    double outlet_flow = model.getOutletFlow(state, inputs);
    
    EXPECT_EQ(outlet_flow, 0.0);
}

// Test: Zero Outlet Flow When Tank Empty
TEST_F(TankModelTest, ZeroOutletFlowWhenTankEmpty) {
    Eigen::VectorXd state(1);
    state << 0.0;  // empty tank
    
    Eigen::VectorXd inputs(2);
    inputs << TEST_INLET_FLOW,  // inlet flow
              1.0;              // valve fully open
    
    double outlet_flow = model.getOutletFlow(state, inputs);
    
    EXPECT_EQ(outlet_flow, 0.0);
}

// Test: Full Valve Opening
TEST_F(TankModelTest, FullValveOpening) {
    Eigen::VectorXd state(1);
    state << TANK_MAX_HEIGHT;  // max height
    
    Eigen::VectorXd inputs(2);
    inputs << TEST_INLET_FLOW,  // inlet flow
              1.0;              // valve fully open
    
    double outlet_flow = model.getOutletFlow(state, inputs);
    
    // Expected: k_v * 1.0 * sqrt(max_height)
    double expected = DEFAULT_VALVE_COEFFICIENT * 1.0 * std::sqrt(TANK_MAX_HEIGHT);
    
    EXPECT_NEAR(outlet_flow, expected, TANK_STATE_TOLERANCE);
}
