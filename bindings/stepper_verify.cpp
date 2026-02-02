#include "stepper.h"
#include <Eigen/Dense>
#include <cmath>
#include <iostream>
#include <iomanip>

using namespace tank_sim;

/**
 * Exponential decay ODE: dy/dt = -k*y
 * Analytical solution: y(t) = y0 * exp(-k*t)
 */
Eigen::VectorXd exponential_decay_derivative(double t, 
                                             const Eigen::VectorXd &state,
                                             const Eigen::VectorXd &input) {
  (void)t;     // Suppress unused parameter warning
  (void)input; // Suppress unused parameter warning
  
  double k = 1.0;
  Eigen::VectorXd deriv(1);
  deriv(0) = -k * state(0);
  return deriv;
}

int main() {
  std::cout << std::fixed << std::setprecision(10);
  std::cout << "========================================\n";
  std::cout << "Stepper (RK4) Verification Program\n";
  std::cout << "========================================\n\n";

  // Test parameters
  const double k = 1.0;
  const double y0 = 1.0;
  const double analytical_final = std::exp(-k * 1.0); // exp(-1.0) ≈ 0.3678794412

  std::cout << "Test: Exponential decay ODE dy/dt = -k*y\n";
  std::cout << "Parameters: k = " << k << ", y0 = " << y0 << "\n";
  std::cout << "Integration: t = 0 to t = 1\n";
  std::cout << "Analytical solution at t=1: y(1) = " << analytical_final << "\n\n";

  // ========== Test 1: dt = 0.1 ==========
  std::cout << "Test 1: dt = 0.1\n";
  std::cout << "-----------------------------------------\n";
  
  Stepper stepper1(1, 1);  // state_dim=1, input_dim=1
  Eigen::VectorXd state1(1);
  state1(0) = y0;
  
  double t1 = 0.0;
  const double dt1 = 0.1;
  const int steps1 = 10;
  
  Eigen::VectorXd dummy_input(1);
  dummy_input(0) = 0.0;
  
  for (int i = 0; i < steps1; ++i) {
    state1 = stepper1.step(t1, dt1, state1, dummy_input, exponential_decay_derivative);
    t1 += dt1;
  }
  
  double error1 = std::abs(state1(0) - analytical_final);
  std::cout << "Final state at t=1.0: " << state1(0) << "\n";
  std::cout << "Analytical value:     " << analytical_final << "\n";
  std::cout << "Absolute error:       " << error1 << "\n";
  std::cout << "Relative error:       " << (error1 / analytical_final) * 100 << "%\n\n";

  // ========== Test 2: dt = 0.05 ==========
  std::cout << "Test 2: dt = 0.05\n";
  std::cout << "-----------------------------------------\n";
  
  Stepper stepper2(1, 1);  // state_dim=1, input_dim=1
  Eigen::VectorXd state2(1);
  state2(0) = y0;
  
  double t2 = 0.0;
  const double dt2 = 0.05;
  const int steps2 = 20;
  
  for (int i = 0; i < steps2; ++i) {
    state2 = stepper2.step(t2, dt2, state2, dummy_input, exponential_decay_derivative);
    t2 += dt2;
  }
  
  double error2 = std::abs(state2(0) - analytical_final);
  std::cout << "Final state at t=1.0: " << state2(0) << "\n";
  std::cout << "Analytical value:     " << analytical_final << "\n";
  std::cout << "Absolute error:       " << error2 << "\n";
  std::cout << "Relative error:       " << (error2 / analytical_final) * 100 << "%\n\n";

  // ========== Order Verification ==========
  std::cout << "Order Verification (4th-order accuracy)\n";
  std::cout << "-----------------------------------------\n";
  
  double expected_ratio = std::pow(0.1 / 0.05, 4.0); // (dt1/dt2)^4 = 2^4 = 16
  double actual_ratio = error1 / error2;
  
  std::cout << "Expected error ratio (dt1/dt2)^4: " << expected_ratio << "\n";
  std::cout << "Actual error ratio:               " << actual_ratio << "\n";
  std::cout << "Relative difference:              " 
            << std::abs(actual_ratio - expected_ratio) / expected_ratio * 100 << "%\n\n";

  // ========== Summary ==========
  std::cout << "========================================\n";
  std::cout << "SUMMARY\n";
  std::cout << "========================================\n";
  
  bool error1_ok = error1 < 1e-5;
  bool error2_ok = error2 < 1e-7;
  bool order_ok = std::abs(actual_ratio - expected_ratio) / expected_ratio < 0.1;
  
  std::cout << "dt=0.1 error < 1e-5:      " << (error1_ok ? "✓ PASS" : "✗ FAIL") << "\n";
  std::cout << "dt=0.05 error < 1e-7:     " << (error2_ok ? "✓ PASS" : "✗ FAIL") << "\n";
  std::cout << "Order ratio within 10%:   " << (order_ok ? "✓ PASS" : "✗ FAIL") << "\n";

  if (error1_ok && error2_ok && order_ok) {
    std::cout << "\n✓ All verification tests PASSED\n";
    return 0;
  } else {
    std::cout << "\n✗ Some verification tests FAILED\n";
    return 1;
  }
}
