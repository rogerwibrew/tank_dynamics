#include <gtest/gtest.h>
#include "../src/pid_controller.h"
#include "../src/constants.h"

using namespace tank_sim;
using namespace tank_sim::constants;

// Test: Proportional Only Response
TEST(PIDControllerTest, ProportionalOnlyResponse) {
    PIDController::Gains gains{DEFAULT_PID_PROPORTIONAL_GAIN, 0.0, 0.0};
    PIDController pid(gains, DEFAULT_PID_BIAS, DEFAULT_PID_MIN_OUTPUT, DEFAULT_PID_MAX_OUTPUT, DEFAULT_PID_MAX_INTEGRAL);

    // First test: error = TEST_ERROR_VALUE, expected output = bias + Kc * error
    double output1 = pid.compute(TEST_ERROR_VALUE, 0.0, TEST_DT);
    EXPECT_NEAR(output1, DEFAULT_PID_BIAS + DEFAULT_PID_PROPORTIONAL_GAIN * TEST_ERROR_VALUE, CONTROL_OUTPUT_TOLERANCE);

    // Reset and test again with error = 0.2
    pid.reset();
    double output2 = pid.compute(0.2, 0.0, TEST_DT);
    EXPECT_NEAR(output2, DEFAULT_PID_BIAS + DEFAULT_PID_PROPORTIONAL_GAIN * 0.2, CONTROL_OUTPUT_TOLERANCE);
}

// Test: Integral Accumulation Over Time
TEST(PIDControllerTest, IntegralAccumulationOverTime) {
    PIDController::Gains gains{DEFAULT_PID_PROPORTIONAL_GAIN, DEFAULT_PID_INTEGRAL_TIME, 0.0};
    PIDController pid(gains, DEFAULT_PID_BIAS, DEFAULT_PID_MIN_OUTPUT, DEFAULT_PID_MAX_OUTPUT, DEFAULT_PID_MAX_INTEGRAL);

    // First call: p_term = TEST_ERROR_VALUE, i_term = 0 (integral state is 0)
    double output1 = pid.compute(TEST_ERROR_VALUE, 0.0, TEST_DT);
    EXPECT_NEAR(output1, DEFAULT_PID_BIAS + DEFAULT_PID_PROPORTIONAL_GAIN * TEST_ERROR_VALUE, CONTROL_OUTPUT_TOLERANCE);

    // After first call, integral accumulates
    // Second call: integral term grows
    double output2 = pid.compute(TEST_ERROR_VALUE, 0.0, TEST_DT);
    EXPECT_GT(output2, output1);

    // After second call, integral continues to accumulate
    // Third call: integral term grows further
    double output3 = pid.compute(TEST_ERROR_VALUE, 0.0, TEST_DT);
    EXPECT_GT(output3, output2);
}

// Test: Derivative Response
TEST(PIDControllerTest, DerivativeResponse) {
    PIDController::Gains gains{DEFAULT_PID_PROPORTIONAL_GAIN, 0.0, DEFAULT_PID_DERIVATIVE_TIME};
    PIDController pid(gains, DEFAULT_PID_BIAS, DEFAULT_PID_MIN_OUTPUT, 2.0, DEFAULT_PID_MAX_INTEGRAL);

    // error = 0.0, error_dot = 0.05 (derivative of error), dt = TEST_DT
    // p_term = 0.0, i_term = 0.0, d_term = tau_D * error_dot
    // output = bias + Kc * (0.0 + 0.0 + tau_D * error_dot)
    double output = pid.compute(0.0, 0.05, TEST_DT);
    EXPECT_GT(output, DEFAULT_PID_BIAS);  // Derivative action should increase output
}

// Test: Output Saturation at Upper Bound
TEST(PIDControllerTest, OutputSaturationUpperBound) {
    PIDController::Gains gains{DEFAULT_PID_PROPORTIONAL_GAIN, 0.0, 0.0};
    PIDController pid(gains, DEFAULT_PID_BIAS, DEFAULT_PID_MIN_OUTPUT, DEFAULT_PID_MAX_OUTPUT, DEFAULT_PID_MAX_INTEGRAL);

    // Large error that will cause saturation
    double output = pid.compute(1.0, 0.0, TEST_DT);
    EXPECT_EQ(output, DEFAULT_PID_MAX_OUTPUT);  // Should be clamped to max
}

// Test: Output Saturation at Lower Bound
TEST(PIDControllerTest, OutputSaturationLowerBound) {
    PIDController::Gains gains{DEFAULT_PID_PROPORTIONAL_GAIN, 0.0, 0.0};
    PIDController pid(gains, DEFAULT_PID_BIAS, DEFAULT_PID_MIN_OUTPUT, DEFAULT_PID_MAX_OUTPUT, DEFAULT_PID_MAX_INTEGRAL);

    // Large negative error that will cause saturation
    double output = pid.compute(-1.0, 0.0, TEST_DT);
    EXPECT_EQ(output, DEFAULT_PID_MIN_OUTPUT);  // Should be clamped to min
}

// Test: Anti-Windup During Saturation
TEST(PIDControllerTest, AntiWindupDuringSaturation) {
    // Controller with integral action
    PIDController::Gains gains{2.0, DEFAULT_PID_INTEGRAL_TIME, 0.0};
    PIDController pid(gains, DEFAULT_PID_BIAS, DEFAULT_PID_MIN_OUTPUT, DEFAULT_PID_MAX_OUTPUT, DEFAULT_PID_MAX_INTEGRAL);

    // Apply large positive error that will saturate output
    // Call compute multiple times to build up integral
    for (int i = 0; i < 5; ++i) {
        pid.compute(1.0, 0.0, TEST_DT);  // Large error causes saturation
    }

    // Check that integral state is clamped and didn't grow excessively
    double saturated_integral = pid.getIntegralState();
    EXPECT_LE(saturated_integral, DEFAULT_PID_MAX_INTEGRAL);

    // Create a second controller with same gains but no saturation
    PIDController pid2(gains, DEFAULT_PID_BIAS, -1000.0, 1000.0, DEFAULT_PID_MAX_INTEGRAL);
    for (int i = 0; i < 5; ++i) {
        pid2.compute(1.0, 0.0, TEST_DT);
    }
    double unsaturated_integral = pid2.getIntegralState();

    // The saturated controller should have accumulated much less integral
    EXPECT_LT(saturated_integral, unsaturated_integral);
}

// Test: Reset Clears Integral State
TEST(PIDControllerTest, ResetClearsIntegralState) {
    PIDController::Gains gains{DEFAULT_PID_PROPORTIONAL_GAIN, DEFAULT_PID_INTEGRAL_TIME, 0.0};
    PIDController pid(gains, DEFAULT_PID_BIAS, DEFAULT_PID_MIN_OUTPUT, DEFAULT_PID_MAX_OUTPUT, DEFAULT_PID_MAX_INTEGRAL);

    // Build up integral over several calls
    pid.compute(TEST_ERROR_VALUE, 0.0, TEST_DT);
    pid.compute(TEST_ERROR_VALUE, 0.0, TEST_DT);
    pid.compute(TEST_ERROR_VALUE, 0.0, TEST_DT);

    // Verify integral is non-zero
    double integral_before = pid.getIntegralState();
    EXPECT_GT(integral_before, 0.0);

    // Reset the controller
    pid.reset();

    // Verify integral is now zero
    EXPECT_EQ(pid.getIntegralState(), 0.0);

    // Verify output behaves as if starting fresh (zero integral, zero derivative)
    double output = pid.compute(TEST_ERROR_VALUE, 0.0, TEST_DT);
    EXPECT_NEAR(output, DEFAULT_PID_BIAS + DEFAULT_PID_PROPORTIONAL_GAIN * TEST_ERROR_VALUE, CONTROL_OUTPUT_TOLERANCE);
}

// Test: SetGains Updates Behavior
TEST(PIDControllerTest, SetGainsUpdatesBehavior) {
    PIDController::Gains gains1{DEFAULT_PID_PROPORTIONAL_GAIN, 0.0, 0.0};
    PIDController pid(gains1, DEFAULT_PID_BIAS, DEFAULT_PID_MIN_OUTPUT, DEFAULT_PID_MAX_OUTPUT, DEFAULT_PID_MAX_INTEGRAL);

    // First output with default Kc
    double output1 = pid.compute(TEST_ERROR_VALUE, 0.0, TEST_DT);
    EXPECT_NEAR(output1, DEFAULT_PID_BIAS + DEFAULT_PID_PROPORTIONAL_GAIN * TEST_ERROR_VALUE, CONTROL_OUTPUT_TOLERANCE);

    // Update gains to higher proportional gain
    PIDController::Gains gains2{2.0 * DEFAULT_PID_PROPORTIONAL_GAIN, 0.0, 0.0};
    pid.setGains(gains2);

    // Second output with higher Kc should produce larger response
    double output2 = pid.compute(TEST_ERROR_VALUE, 0.0, TEST_DT);
    EXPECT_GT(output2, output1);
}

// Test: Zero Error Produces Bias Output
TEST(PIDControllerTest, ZeroErrorProducesBiasOutput) {
    PIDController::Gains gains{1.5, DEFAULT_PID_INTEGRAL_TIME, DEFAULT_PID_DERIVATIVE_TIME};
    PIDController pid(gains, DEFAULT_PID_BIAS, DEFAULT_PID_MIN_OUTPUT, DEFAULT_PID_MAX_OUTPUT, DEFAULT_PID_MAX_INTEGRAL);

    // Ensure integral is zero (newly constructed)
    EXPECT_EQ(pid.getIntegralState(), 0.0);

    // error = 0.0, error_dot = 0.0
    // All PID terms should be zero, output = bias
    double output = pid.compute(0.0, 0.0, TEST_DT);
    EXPECT_EQ(output, DEFAULT_PID_BIAS);
}

// Test: Combined PID Action
TEST(PIDControllerTest, CombinedPIDAction) {
    PIDController::Gains gains{DEFAULT_PID_PROPORTIONAL_GAIN, DEFAULT_PID_INTEGRAL_TIME, DEFAULT_PID_DERIVATIVE_TIME};
    PIDController pid(gains, DEFAULT_PID_BIAS, DEFAULT_PID_MIN_OUTPUT, DEFAULT_PID_MAX_OUTPUT, DEFAULT_PID_MAX_INTEGRAL);

    // First call with error and error derivative
    double output1 = pid.compute(TEST_ERROR_VALUE, 0.05, TEST_DT);
    // Should have all three terms: proportional, integral (small), and derivative
    EXPECT_GT(output1, DEFAULT_PID_BIAS);

    // Second call with same inputs
    // Integral should accumulate further
    double output2 = pid.compute(TEST_ERROR_VALUE, 0.05, TEST_DT);
    // Should increase due to additional integral accumulation
    EXPECT_GT(output2, output1);
}
