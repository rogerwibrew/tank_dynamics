#include <gtest/gtest.h>
#include <Eigen/Dense>
#include <cmath>
#include "../src/tank_model.h"

using namespace tank_sim;

class TankModelTest : public ::testing::Test {
protected:
    // Standard test parameters from the plan
    // Using C++17 positional initialization (area, k_v, max_height)
    //
    // These parameters are chosen such that at h = 2.5 m with valve position
    // x = 0.5, the outlet flow equals 1.0 m³/s (steady state with inlet = 1.0):
    //   q_out = k_v * x * sqrt(h) = 1.2649 * 0.5 * sqrt(2.5) ≈ 1.0 m³/s
    // This allows tests to verify material balance (dh/dt ≈ 0) at steady state.
    TankModel::Parameters params{
        120.0,      // area: cross-sectional area (m²)
        1.2649,     // k_v: valve coefficient (m^2.5/s), chosen for steady-state testing
        5.0         // max_height: maximum tank height (m)
    };
    
    TankModel model{params};
};

// Test: Steady State Zero Derivative
TEST_F(TankModelTest, SteadyStateZeroDerivative) {
    // At steady state with inlet = 1.0, valve = 0.5, level = 2.5
    // outlet flow should equal inlet flow, so derivative should be zero
    
    Eigen::VectorXd state(1);
    state << 2.5;  // tank level in meters
    
    Eigen::VectorXd inputs(2);
    inputs << 1.0,   // inlet flow: 1.0 cubic meters per second
              0.5;   // valve position: 0.5 (50% open)
    
    Eigen::VectorXd derivative = model.derivatives(state, inputs);
    
    // At steady state: q_in = q_out, so dh/dt should be ~0
    EXPECT_NEAR(derivative(0), 0.0, 0.001);
}

// Test: Positive Derivative When Inlet Exceeds Outlet
TEST_F(TankModelTest, PositiveDerivativeWhenInletExceedsOutlet) {
    Eigen::VectorXd state(1);
    state << 2.5;  // tank level in meters
    
    Eigen::VectorXd inputs(2);
    inputs << 1.5,   // inlet flow: 1.5 cubic meters per second
              0.5;   // valve position: 0.5
    
    Eigen::VectorXd derivative = model.derivatives(state, inputs);
    
    // Calculate expected outlet flow: k_v * x * sqrt(h)
    double expected_outlet = 1.2649 * 0.5 * std::sqrt(2.5);  // ~1.0
    double expected_derivative = (1.5 - expected_outlet) / 120.0;
    
    EXPECT_GT(derivative(0), 0.0);  // Should be positive (tank filling)
    EXPECT_NEAR(derivative(0), expected_derivative, 0.001);
}

// Test: Negative Derivative When Outlet Exceeds Inlet
TEST_F(TankModelTest, NegativeDerivativeWhenOutletExceedsInlet) {
    Eigen::VectorXd state(1);
    state << 2.5;  // tank level in meters
    
    Eigen::VectorXd inputs(2);
    inputs << 0.5,   // inlet flow: 0.5 cubic meters per second
              0.5;   // valve position: 0.5
    
    Eigen::VectorXd derivative = model.derivatives(state, inputs);
    
    // Calculate expected outlet flow
    double expected_outlet = 1.2649 * 0.5 * std::sqrt(2.5);  // ~1.0
    double expected_derivative = (0.5 - expected_outlet) / 120.0;
    
    EXPECT_LT(derivative(0), 0.0);  // Should be negative (tank draining)
    EXPECT_NEAR(derivative(0), expected_derivative, 0.001);
}

// Test: Outlet Flow Calculation via Public Getter
TEST_F(TankModelTest, OutletFlowCalculation) {
    Eigen::VectorXd state(1);
    state << 2.5;  // tank level in meters
    
    Eigen::VectorXd inputs(2);
    inputs << 1.0,   // inlet flow (not used for outlet calculation)
              0.5;   // valve position: 50% open
    
    double outlet_flow = model.getOutletFlow(state, inputs);
    
    // Expected: k_v * x * sqrt(h) = 1.2649 * 0.5 * sqrt(2.5)
    double expected = 1.2649 * 0.5 * std::sqrt(2.5);
    
    EXPECT_NEAR(outlet_flow, expected, 0.001);
}

// Test: Zero Outlet Flow When Valve Closed
TEST_F(TankModelTest, ZeroOutletFlowWhenValveClosed) {
    Eigen::VectorXd state(1);
    state << 5.0;  // any positive level
    
    Eigen::VectorXd inputs(2);
    inputs << 1.0,   // inlet flow
              0.0;   // valve fully closed
    
    double outlet_flow = model.getOutletFlow(state, inputs);
    
    EXPECT_EQ(outlet_flow, 0.0);
}

// Test: Zero Outlet Flow When Tank Empty
TEST_F(TankModelTest, ZeroOutletFlowWhenTankEmpty) {
    Eigen::VectorXd state(1);
    state << 0.0;  // empty tank
    
    Eigen::VectorXd inputs(2);
    inputs << 1.0,   // inlet flow
              1.0;   // valve fully open
    
    double outlet_flow = model.getOutletFlow(state, inputs);
    
    EXPECT_EQ(outlet_flow, 0.0);
}

// Test: Full Valve Opening
TEST_F(TankModelTest, FullValveOpening) {
    Eigen::VectorXd state(1);
    state << 5.0;  // max height
    
    Eigen::VectorXd inputs(2);
    inputs << 1.0,   // inlet flow
              1.0;   // valve fully open
    
    double outlet_flow = model.getOutletFlow(state, inputs);
    
    // Expected: k_v * 1.0 * sqrt(5.0)
    double expected = 1.2649 * 1.0 * std::sqrt(5.0);
    
    EXPECT_NEAR(outlet_flow, expected, 0.001);
}
