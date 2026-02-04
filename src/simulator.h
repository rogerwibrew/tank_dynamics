#ifndef TANK_SIMULATOR_H
#define TANK_SIMULATOR_H

#include "pid_controller.h" // Include the PID controller header
#include "stepper.h"
#include "tank_model.h"
#include <Eigen/src/Core/Matrix.h>
#include <vector>

namespace tank_sim {

class Simulator {
public:
  struct ControllerConfig {
    tank_sim::PIDController::Gains gains; // Use the existing Gains struct
    double bias;
    double minOutputLimit;
    double maxOutputLimit;
    double maxIntegralAccumulation;
    int measuredIndex;
    int outputIndex;
    double initialSetpoint;
  };

  struct Config {
    tank_sim::TankModel::Parameters params;
    std::vector<ControllerConfig> controllerConfig;
    Eigen::VectorXd initialState;
    Eigen::VectorXd initialInputs;
    double dt;
  };

  // Constructor
  Simulator(const Config &config);

  void step();

  // State getters (const methods - do not modify simulator state)
  double getTime() const;
  Eigen::VectorXd getState() const;
  Eigen::VectorXd getInputs() const;
  double getSetpoint(int index) const;
  double getControllerOutput(int index) const;
  double getError(int index) const;
  int getControllerCount() const;

  // Operator control methods
  void setInput(int index, double value);
  void setSetpoint(int index, double value);
  void setControllerGains(int index, const tank_sim::PIDController::Gains &gains);

  // Utility method
  void reset();

  private:

  TankModel model;
  Stepper stepper;
  std::vector<PIDController> controllers;
  double time;
  Eigen::VectorXd state;
  Eigen::VectorXd inputs;
  Eigen::VectorXd initialState;
  Eigen::VectorXd initialInputs;
  double dt;
  std::vector<double> setpoints;
  std::vector<double> previousErrors;  // For error derivative calculation
  std::vector<ControllerConfig> controllerConfig;
};

} // namespace tank_sim

#endif // TANK_SIMULATOR_H
