#include <gtest/gtest.h>
#include <Eigen/Dense>
#include <cmath>
#include "../src/simulator.h"
#include "../src/constants.h"

using namespace tank_sim;
using namespace tank_sim::constants;

class SimulatorTest : public ::testing::Test {
protected:
    // Standard test configuration at steady state
    // Tank level: 2.5 m (50% of 5 m max)
    // Inlet flow: 1.0 m³/s
    // Valve position: 0.5 (50% open)
    // At this condition, outlet flow = k_v * 0.5 * sqrt(2.5) ≈ 1.0 m³/s (steady state)
    Simulator::Config createSteadyStateConfig(double setpoint = TANK_NOMINAL_HEIGHT) {
        Simulator::Config config;
        config.params = TankModel::Parameters{
            DEFAULT_TANK_AREA,
            DEFAULT_VALVE_COEFFICIENT,
            TANK_MAX_HEIGHT
        };
        
        config.initialState = Eigen::VectorXd(1);
        config.initialState << TANK_NOMINAL_HEIGHT;  // 2.5 m
        
        config.initialInputs = Eigen::VectorXd(2);
        config.initialInputs << TEST_INLET_FLOW,     // 1.0 m³/s
                                TEST_VALVE_POSITION; // 0.5 (valve 50% open)
        
        config.dt = 1.0;  // 1 second time step
        
        // Controller configuration for level setpoint
        // NOTE: Kc is NEGATIVE for reverse-acting control.
        // In this system, increasing valve position increases outlet flow,
        // which DECREASES tank level. So when level is below setpoint (positive error),
        // we need to DECREASE controller output (close valve) to raise the level.
        // This requires negative Kc.
        Simulator::ControllerConfig ctrl_config;
        ctrl_config.gains = PIDController::Gains{
            -1.0,  // Kc: NEGATIVE for reverse-acting control
            10.0,  // tau_I: integral time (10 s)
            0.0    // tau_D: derivative time (disabled)
        };
        ctrl_config.bias = 0.5;  // 50% nominal valve position
        ctrl_config.minOutputLimit = 0.0;
        ctrl_config.maxOutputLimit = 1.0;
        ctrl_config.maxIntegralAccumulation = 10.0;
        ctrl_config.measuredIndex = 0;  // Read tank level from state[0]
        ctrl_config.outputIndex = 1;    // Write valve position to inputs[1]
        ctrl_config.initialSetpoint = setpoint;
        
        config.controllerConfig.push_back(ctrl_config);
        
        return config;
    }
};

// Test: Constructor Validation - Empty state vector
TEST_F(SimulatorTest, ConstructorValidationEmptyState) {
    Simulator::Config config = createSteadyStateConfig();
    config.initialState = Eigen::VectorXd(0);  // Empty!
    
    EXPECT_THROW(Simulator sim(config), std::invalid_argument);
}

// Test: Constructor Validation - Empty inputs vector
TEST_F(SimulatorTest, ConstructorValidationEmptyInputs) {
    Simulator::Config config = createSteadyStateConfig();
    config.initialInputs = Eigen::VectorXd(0);  // Empty!
    
    EXPECT_THROW(Simulator sim(config), std::invalid_argument);
}

// Test: Constructor Validation - Negative dt
TEST_F(SimulatorTest, ConstructorValidationNegativeDt) {
    Simulator::Config config = createSteadyStateConfig();
    config.dt = -0.1;
    
    EXPECT_THROW(Simulator sim(config), std::invalid_argument);
}

// Test: Constructor Validation - Zero dt
TEST_F(SimulatorTest, ConstructorValidationZeroDt) {
    Simulator::Config config = createSteadyStateConfig();
    config.dt = 0.0;
    
    EXPECT_THROW(Simulator sim(config), std::invalid_argument);
}

// Test: Constructor Validation - Controller measured_index out of bounds
TEST_F(SimulatorTest, ConstructorValidationMeasuredIndexOutOfBounds) {
    Simulator::Config config = createSteadyStateConfig();
    config.controllerConfig[0].measuredIndex = 5;  // State size is 1, index 5 is invalid
    
    EXPECT_THROW(Simulator sim(config), std::invalid_argument);
}

// Test: Constructor Validation - Controller output_index out of bounds
TEST_F(SimulatorTest, ConstructorValidationOutputIndexOutOfBounds) {
    Simulator::Config config = createSteadyStateConfig();
    config.controllerConfig[0].outputIndex = 5;  // Input size is 2, index 5 is invalid
    
    EXPECT_THROW(Simulator sim(config), std::invalid_argument);
}

// Test: Steady State Remains Steady
TEST_F(SimulatorTest, SteadyStateRemainsSteady) {
    // Tank level at 2.5 m with inlet 1.0 m³/s and valve at 0.5
    // This is carefully tuned to be at steady state: q_out ≈ q_in
    Simulator::Config config = createSteadyStateConfig(TANK_NOMINAL_HEIGHT);
    Simulator sim(config);
    
    // Run for 100 steps (100 seconds at dt=1.0)
    for (int i = 0; i < 100; ++i) {
        sim.step();
    }
    
    // After 100 steps, verify steady state is maintained
    Eigen::VectorXd state = sim.getState();
    Eigen::VectorXd inputs = sim.getInputs();
    
    // Tank level should remain at 2.5 m
    EXPECT_NEAR(state(0), TANK_NOMINAL_HEIGHT, 0.01);
    
    // Inlet flow should remain at 1.0 m³/s
    EXPECT_NEAR(inputs(0), TEST_INLET_FLOW, 1e-6);
    
    // Valve position should remain at 0.5
    EXPECT_NEAR(inputs(1), TEST_VALVE_POSITION, 0.01);
    
    // Controller output should be near bias (0.5)
    EXPECT_NEAR(sim.getControllerOutput(0), TEST_VALVE_POSITION, 0.01);
}

// Test: Step Response - Level Increase
TEST_F(SimulatorTest, StepResponseLevelIncrease) {
    // Start at steady state: level 2.5 m, setpoint 2.5 m
    // Change setpoint to 3.0 m (increase by 0.5 m)
    Simulator::Config config = createSteadyStateConfig(TANK_NOMINAL_HEIGHT);
    Simulator sim(config);
    
    // Run a few steps to establish steady state
    for (int i = 0; i < 10; ++i) {
        sim.step();
    }
    
    // Change setpoint to 3.0 m
    sim.setSetpoint(0, 3.0);
    
    // Run for 200 steps to observe response
    for (int i = 0; i < 200; ++i) {
        sim.step();
    }
    
    Eigen::VectorXd state = sim.getState();
    Eigen::VectorXd inputs = sim.getInputs();
    
    // Tank level should have increased from 2.5 m
    EXPECT_GT(state(0), TANK_NOMINAL_HEIGHT);
    
    // Tank level should be close to new setpoint (within 0.1 m)
    EXPECT_NEAR(state(0), 3.0, 0.1);
    
    // Valve should be in a lower position than nominal (to reduce outlet flow)
    EXPECT_LT(inputs(1), TEST_VALVE_POSITION);
    
    // Error should be small
    EXPECT_NEAR(sim.getError(0), 0.0, 0.1);
}

// Test: Step Response - Level Decrease
TEST_F(SimulatorTest, StepResponseLevelDecrease) {
    // Start at steady state: level 2.5 m, setpoint 2.5 m
    // Change setpoint to 2.0 m (decrease by 0.5 m)
    Simulator::Config config = createSteadyStateConfig(TANK_NOMINAL_HEIGHT);
    Simulator sim(config);
    
    // Run a few steps to establish steady state
    for (int i = 0; i < 10; ++i) {
        sim.step();
    }
    
    // Change setpoint to 2.0 m
    sim.setSetpoint(0, 2.0);
    
    // Run for 200 steps to observe response
    for (int i = 0; i < 200; ++i) {
        sim.step();
    }
    
    Eigen::VectorXd state = sim.getState();
    Eigen::VectorXd inputs = sim.getInputs();
    
    // Tank level should have decreased from 2.5 m
    EXPECT_LT(state(0), TANK_NOMINAL_HEIGHT);
    
    // Tank level should be close to new setpoint (within 0.1 m)
    EXPECT_NEAR(state(0), 2.0, 0.1);
    
    // Valve should be in a higher position than nominal (to increase outlet flow)
    EXPECT_GT(inputs(1), TEST_VALVE_POSITION);
    
    // Error should be small
    EXPECT_NEAR(sim.getError(0), 0.0, 0.1);
}

// Test: Disturbance Rejection
TEST_F(SimulatorTest, DisturbanceRejection) {
    // Start at steady state
    Simulator::Config config = createSteadyStateConfig(TANK_NOMINAL_HEIGHT);
    Simulator sim(config);
    
    // Run to establish steady state
    for (int i = 0; i < 10; ++i) {
        sim.step();
    }
    
    // Apply disturbance: increase inlet flow from 1.0 to 1.2 m³/s at t=50s
    for (int i = 0; i < 50; ++i) {
        sim.step();
    }
    
    // Apply disturbance
    sim.setInput(0, 1.2);  // Increase inlet flow
    
    // Continue for 200 more steps
    for (int i = 0; i < 200; ++i) {
        sim.step();
    }
    
    Eigen::VectorXd state = sim.getState();
    
    // Level will rise initially but controller should bring it back
    // After 200 steps, level should return to setpoint (within 0.1 m)
    EXPECT_NEAR(state(0), TANK_NOMINAL_HEIGHT, 0.1);
    
    // Error should be small
    EXPECT_NEAR(sim.getError(0), 0.0, 0.1);
}

// Test: Controller Saturation and Recovery
TEST_F(SimulatorTest, ControllerSaturationAndRecovery) {
    // Start at steady state
    Simulator::Config config = createSteadyStateConfig(TANK_NOMINAL_HEIGHT);
    Simulator sim(config);
    
    // Run a few steps to establish steady state
    for (int i = 0; i < 10; ++i) {
        sim.step();
    }
    
    // Set setpoint very high (4.5 m, near maximum 5.0 m)
    sim.setSetpoint(0, 4.5);
    
    // Run for 300 steps
    for (int i = 0; i < 300; ++i) {
        sim.step();
    }
    
    Eigen::VectorXd state = sim.getState();
    Eigen::VectorXd inputs = sim.getInputs();
    
    // Valve should saturate at 0.0 (fully closed) during initial response
    // to maximize level increase
    EXPECT_GE(inputs(1), 0.0);
    EXPECT_LE(inputs(1), 1.0);
    
    // Level should have risen and be approaching 4.5 m
    EXPECT_GT(state(0), TANK_NOMINAL_HEIGHT);
    EXPECT_LT(state(0), 4.6);  // Approaching 4.5 m
    
    // Controller output should be reasonable (not infinitely large)
    double ctrl_output = sim.getControllerOutput(0);
    EXPECT_GE(ctrl_output, 0.0);
    EXPECT_LE(ctrl_output, 1.0);
}

// Test: Reset Functionality
TEST_F(SimulatorTest, ResetFunctionality) {
    Simulator::Config config = createSteadyStateConfig(TANK_NOMINAL_HEIGHT);
    Simulator sim(config);
    
    // Run for 50 steps at initial setpoint
    for (int i = 0; i < 50; ++i) {
        sim.step();
    }
    
    // Record state after first 50 steps
    double time_after_50 = sim.getTime();
    Eigen::VectorXd state_after_50 = sim.getState();
    
    // Change setpoint and run more steps (system in transient)
    sim.setSetpoint(0, 3.5);
    for (int i = 0; i < 50; ++i) {
        sim.step();
    }
    
    // Now in a transient state; reset
    sim.reset();
    
    // After reset, verify initial conditions
    EXPECT_EQ(sim.getTime(), 0.0);
    EXPECT_NEAR(sim.getSetpoint(0), TANK_NOMINAL_HEIGHT, 1e-10);
    
    Eigen::VectorXd reset_state = sim.getState();
    EXPECT_NEAR(reset_state(0), config.initialState(0), 1e-10);
    
    Eigen::VectorXd reset_inputs = sim.getInputs();
    EXPECT_NEAR(reset_inputs(0), config.initialInputs(0), 1e-10);
    EXPECT_NEAR(reset_inputs(1), config.initialInputs(1), 1e-10);
    
    // Run the same sequence again and verify behavior is identical
    // Run 50 steps at initial setpoint
    for (int i = 0; i < 50; ++i) {
        sim.step();
    }
    
    // State after 50 steps should be identical to first run
    Eigen::VectorXd state_after_reset_50 = sim.getState();
    EXPECT_NEAR(state_after_reset_50(0), state_after_50(0), 0.001);
    
    // Time should be 50.0 seconds
    EXPECT_NEAR(sim.getTime(), 50.0, 1e-10);
}

// Test: Dynamic Retuning
TEST_F(SimulatorTest, DynamicRetuning) {
    Simulator::Config config = createSteadyStateConfig(TANK_NOMINAL_HEIGHT);
    Simulator sim(config);
    
    // Run for 50 steps at initial PID gains
    for (int i = 0; i < 50; ++i) {
        sim.step();
    }
    
    // Change to more aggressive gains (higher magnitude Kc, still negative for reverse-acting)
    PIDController::Gains aggressive_gains{
        -2.0,  // Kc doubled magnitude (more aggressive, still negative)
        10.0,  // tau_I unchanged
        0.0    // tau_D unchanged
    };
    sim.setControllerGains(0, aggressive_gains);
    
    // Change setpoint
    sim.setSetpoint(0, 3.0);
    
    // Run 200 steps with new gains
    for (int i = 0; i < 200; ++i) {
        sim.step();
    }
    
    Eigen::VectorXd state = sim.getState();
    
    // System should respond to new setpoint with new dynamics
    // Level should be tracking 3.0 m
    EXPECT_NEAR(state(0), 3.0, 0.15);  // Slightly more relaxed tolerance for dynamic retuning
    
    // The aggressive controller should bring it close to setpoint
    EXPECT_NEAR(sim.getError(0), 0.0, 0.15);
}

// Test: Time Advancement
TEST_F(SimulatorTest, TimeAdvancement) {
    Simulator::Config config = createSteadyStateConfig();
    Simulator sim(config);
    
    // Initial time should be 0.0
    EXPECT_EQ(sim.getTime(), 0.0);
    
    // After 1 step (dt=1.0), time should be 1.0
    sim.step();
    EXPECT_EQ(sim.getTime(), 1.0);
    
    // After 9 more steps (total 10), time should be 10.0
    for (int i = 0; i < 9; ++i) {
        sim.step();
    }
    EXPECT_EQ(sim.getTime(), 10.0);
}

// Test: Getter Methods
TEST_F(SimulatorTest, GetterMethods) {
    Simulator::Config config = createSteadyStateConfig();
    Simulator sim(config);
    
    // Run a few steps
    for (int i = 0; i < 5; ++i) {
        sim.step();
    }
    
    // getState() should return correct size vector
    Eigen::VectorXd state = sim.getState();
    EXPECT_EQ(state.size(), 1);
    EXPECT_GT(state(0), 0.0);  // Level should be positive
    EXPECT_LE(state(0), TANK_MAX_HEIGHT);  // Level should be below max
    
    // getInputs() should return correct size vector
    Eigen::VectorXd inputs = sim.getInputs();
    EXPECT_EQ(inputs.size(), 2);
    EXPECT_GT(inputs(0), 0.0);  // Inlet flow should be positive
    EXPECT_GE(inputs(1), 0.0);  // Valve position should be in [0, 1]
    EXPECT_LE(inputs(1), 1.0);
    
    // getSetpoint() should return the correct setpoint value
    double setpoint = sim.getSetpoint(0);
    EXPECT_EQ(setpoint, TANK_NOMINAL_HEIGHT);
    
    // getControllerOutput() should return value in valid range
    double ctrl_output = sim.getControllerOutput(0);
    EXPECT_GE(ctrl_output, 0.0);
    EXPECT_LE(ctrl_output, 1.0);
    
    // getError() should return setpoint minus measured value
    double error = sim.getError(0);
    double expected_error = setpoint - state(0);
    EXPECT_NEAR(error, expected_error, 1e-10);
    
    // getTime() should return positive value after steps
    double time = sim.getTime();
    EXPECT_GT(time, 0.0);
    EXPECT_EQ(time, 5.0);  // 5 steps * 1.0 s dt
}

// Test: Multiple Controllers (if applicable)
TEST_F(SimulatorTest, MultipleControllersIndependent) {
    // Create a configuration with two controllers
    Simulator::Config config;
    config.params = TankModel::Parameters{
        DEFAULT_TANK_AREA,
        DEFAULT_VALVE_COEFFICIENT,
        TANK_MAX_HEIGHT
    };
    
    config.initialState = Eigen::VectorXd(1);
    config.initialState << TANK_NOMINAL_HEIGHT;
    
    config.initialInputs = Eigen::VectorXd(2);
    config.initialInputs << TEST_INLET_FLOW,
                            TEST_VALVE_POSITION;
    
    config.dt = 1.0;
    
    // First controller on input 0 with setpoint 2.5 m (negative Kc for reverse-acting)
    Simulator::ControllerConfig ctrl1;
    ctrl1.gains = PIDController::Gains{-1.0, 10.0, 0.0};  // Negative Kc
    ctrl1.bias = 1.0;  // Bias for inlet flow
    ctrl1.minOutputLimit = 0.0;
    ctrl1.maxOutputLimit = 2.0;
    ctrl1.maxIntegralAccumulation = 10.0;
    ctrl1.measuredIndex = 0;
    ctrl1.outputIndex = 0;
    ctrl1.initialSetpoint = 2.5;
    config.controllerConfig.push_back(ctrl1);
    
    // Second controller on input 1 with different setpoint 2.0 m (negative Kc for reverse-acting)
    Simulator::ControllerConfig ctrl2;
    ctrl2.gains = PIDController::Gains{-0.5, 5.0, 0.0};  // Negative Kc
    ctrl2.bias = 0.5;
    ctrl2.minOutputLimit = 0.0;
    ctrl2.maxOutputLimit = 1.0;
    ctrl2.maxIntegralAccumulation = 10.0;
    ctrl2.measuredIndex = 0;
    ctrl2.outputIndex = 1;
    ctrl2.initialSetpoint = 2.0;  // Different setpoint
    config.controllerConfig.push_back(ctrl2);
    
    Simulator sim(config);
    
    // Run a few steps
    for (int i = 0; i < 10; ++i) {
        sim.step();
    }
    
    // Both controllers should have produced outputs
    double output0 = sim.getControllerOutput(0);
    double output1 = sim.getControllerOutput(1);
    
    // Both should be valid
    EXPECT_GE(output0, 0.0);
    EXPECT_LE(output0, 2.0);
    EXPECT_GE(output1, 0.0);
    EXPECT_LE(output1, 1.0);
    
    // Get errors - controllers have different setpoints
    double error0 = sim.getError(0);  // setpoint 2.5, measured 2.5 -> error ~0
    double error1 = sim.getError(1);  // setpoint 2.0, measured 2.5 -> error ~-0.5
    
    // Errors should be different because setpoints are different
    EXPECT_NE(error0, error1);
    
    // Controller 0 at steady state should have near-zero error
    EXPECT_NEAR(error0, 0.0, 0.1);
    
    // Controller 1 should have negative error (measured > setpoint)
    EXPECT_LT(error1, 0.0);
}
