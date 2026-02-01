#include <gtest/gtest.h>
#include "../src/pid_controller.h"

using namespace tank_sim;

// Test: Proportional Only Response
TEST(PIDControllerTest, ProportionalOnlyResponse) {
    PIDController::Gains gains{1.0, 0.0, 0.0};
    PIDController pid(gains, 0.5, 0.0, 1.0, 10.0);

    // First test: error = 0.1, expected output = 0.5 + 1.0 * 0.1 = 0.6
    double output1 = pid.compute(0.1, 0.0, 1.0);
    EXPECT_NEAR(output1, 0.6, 0.001);

    // Reset and test again with error = 0.2, expected = 0.5 + 1.0 * 0.2 = 0.7
    pid.reset();
    double output2 = pid.compute(0.2, 0.0, 1.0);
    EXPECT_NEAR(output2, 0.7, 0.001);
}

// Test: Integral Accumulation Over Time
TEST(PIDControllerTest, IntegralAccumulationOverTime) {
    PIDController::Gains gains{1.0, 10.0, 0.0};
    PIDController pid(gains, 0.5, 0.0, 1.0, 10.0);

    // First call: p_term = 0.1, i_term = 0 (integral state is 0)
    // output = 0.5 + 1.0 * (0.1 + 0) = 0.6
    double output1 = pid.compute(0.1, 0.0, 1.0);
    EXPECT_NEAR(output1, 0.6, 0.001);

    // After first call, integral_state = 0.1 * 1.0 = 0.1
    // Second call: i_term = (1.0 / 10.0) * 0.1 = 0.01
    // output = 0.5 + 1.0 * (0.1 + 0.01) = 0.61
    double output2 = pid.compute(0.1, 0.0, 1.0);
    EXPECT_NEAR(output2, 0.61, 0.001);

    // After second call, integral_state = 0.1 + 0.1 * 1.0 = 0.2
    // Third call: i_term = (1.0 / 10.0) * 0.2 = 0.02
    // output = 0.5 + 1.0 * (0.1 + 0.02) = 0.62
    double output3 = pid.compute(0.1, 0.0, 1.0);
    EXPECT_NEAR(output3, 0.62, 0.001);
}

// Test: Derivative Response
TEST(PIDControllerTest, DerivativeResponse) {
    PIDController::Gains gains{1.0, 0.0, 5.0};
    PIDController pid(gains, 0.5, 0.0, 2.0, 10.0);

    // error = 0.0, error_dot = 0.1, dt = 1.0
    // p_term = 0.0, i_term = 0.0, d_term = 5.0 * 0.1 = 0.5
    // output = 0.5 + 1.0 * (0.0 + 0.0 + 0.5) = 1.0
    double output = pid.compute(0.0, 0.1, 1.0);
    EXPECT_NEAR(output, 1.0, 0.001);
}

// Test: Output Saturation at Upper Bound
TEST(PIDControllerTest, OutputSaturationUpperBound) {
    PIDController::Gains gains{1.0, 0.0, 0.0};
    PIDController pid(gains, 0.5, 0.0, 1.0, 10.0);

    // error = 1.0, expected raw output = 0.5 + 1.0 * 1.0 = 1.5
    // Should be clamped to 1.0 (max_output)
    double output = pid.compute(1.0, 0.0, 1.0);
    EXPECT_EQ(output, 1.0);
}

// Test: Output Saturation at Lower Bound
TEST(PIDControllerTest, OutputSaturationLowerBound) {
    PIDController::Gains gains{1.0, 0.0, 0.0};
    PIDController pid(gains, 0.5, 0.0, 1.0, 10.0);

    // error = -1.0, expected raw output = 0.5 - 1.0 * 1.0 = -0.5
    // Should be clamped to 0.0 (min_output)
    double output = pid.compute(-1.0, 0.0, 1.0);
    EXPECT_EQ(output, 0.0);
}

// Test: Anti-Windup During Saturation
TEST(PIDControllerTest, AntiWindupDuringSaturation) {
    // Controller with integral action
    PIDController::Gains gains{2.0, 1.0, 0.0};
    PIDController pid(gains, 0.5, 0.0, 1.0, 10.0);

    // Apply large positive error that will saturate output
    // Call compute multiple times to build up integral
    for (int i = 0; i < 5; ++i) {
        pid.compute(1.0, 0.0, 1.0);  // Large error causes saturation
    }

    // Check that integral state is clamped and didn't grow excessively
    double saturated_integral = pid.getIntegralState();
    EXPECT_LE(saturated_integral, 10.0);  // max_integral limit

    // Create a second controller with same gains but no saturation
    PIDController pid2(gains, 0.5, -1000.0, 1000.0, 10.0);
    for (int i = 0; i < 5; ++i) {
        pid2.compute(1.0, 0.0, 1.0);
    }
    double unsaturated_integral = pid2.getIntegralState();

    // The saturated controller should have accumulated much less integral
    EXPECT_LT(saturated_integral, unsaturated_integral);
}

// Test: Reset Clears Integral State
TEST(PIDControllerTest, ResetClearsIntegralState) {
    PIDController::Gains gains{1.0, 10.0, 0.0};
    PIDController pid(gains, 0.5, 0.0, 1.0, 10.0);

    // Build up integral over several calls
    pid.compute(0.1, 0.0, 1.0);
    pid.compute(0.1, 0.0, 1.0);
    pid.compute(0.1, 0.0, 1.0);

    // Verify integral is non-zero
    double integral_before = pid.getIntegralState();
    EXPECT_GT(integral_before, 0.0);

    // Reset the controller
    pid.reset();

    // Verify integral is now zero
    EXPECT_EQ(pid.getIntegralState(), 0.0);

    // Verify output behaves as if starting fresh
    // With zero integral, output should be 0.5 + 1.0 * 0.1 = 0.6
    double output = pid.compute(0.1, 0.0, 1.0);
    EXPECT_NEAR(output, 0.6, 0.001);
}

// Test: SetGains Updates Behavior
TEST(PIDControllerTest, SetGainsUpdatesBehavior) {
    PIDController::Gains gains1{1.0, 0.0, 0.0};
    PIDController pid(gains1, 0.5, 0.0, 1.0, 10.0);

    // First output with Kc = 1.0
    // error = 0.1, output = 0.5 + 1.0 * 0.1 = 0.6
    double output1 = pid.compute(0.1, 0.0, 1.0);
    EXPECT_NEAR(output1, 0.6, 0.001);

    // Update gains to Kc = 2.0
    PIDController::Gains gains2{2.0, 0.0, 0.0};
    pid.setGains(gains2);

    // Second output with Kc = 2.0
    // error = 0.1, output = 0.5 + 2.0 * 0.1 = 0.7
    double output2 = pid.compute(0.1, 0.0, 1.0);
    EXPECT_NEAR(output2, 0.7, 0.001);
}

// Test: Zero Error Produces Bias Output
TEST(PIDControllerTest, ZeroErrorProducesBiasOutput) {
    PIDController::Gains gains{1.5, 5.0, 2.0};  // Any gains
    PIDController pid(gains, 0.5, 0.0, 1.0, 10.0);

    // Ensure integral is zero (newly constructed)
    EXPECT_EQ(pid.getIntegralState(), 0.0);

    // error = 0.0, error_dot = 0.0
    // All terms should be zero, output = bias = 0.5
    double output = pid.compute(0.0, 0.0, 1.0);
    EXPECT_EQ(output, 0.5);
}

// Test: Combined PID Action
TEST(PIDControllerTest, CombinedPIDAction) {
    PIDController::Gains gains{1.0, 10.0, 2.0};
    PIDController pid(gains, 0.5, 0.0, 1.0, 10.0);

    // error = 0.1, error_dot = 0.05, dt = 1.0
    // p_term = 0.1
    // i_term (first call, integral_state = 0): (1.0 / 10.0) * 0 = 0.0
    // d_term = 2.0 * 0.05 = 0.1
    // output = 0.5 + 1.0 * (0.1 + 0.0 + 0.1) = 0.7
    double output = pid.compute(0.1, 0.05, 1.0);
    EXPECT_NEAR(output, 0.7, 0.001);

    // Second call with same inputs
    // integral_state is now 0.1 * 1.0 = 0.1
    // i_term = (1.0 / 10.0) * 0.1 = 0.01
    // output = 0.5 + 1.0 * (0.1 + 0.01 + 0.1) = 0.71
    double output2 = pid.compute(0.1, 0.05, 1.0);
    EXPECT_NEAR(output2, 0.71, 0.001);
}
