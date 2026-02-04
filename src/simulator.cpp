#include "simulator.h"
#include "constants.h"

namespace tank_sim {

Simulator::Simulator(const Config &config)
    : model(config.params),
      stepper(config.initialState.size(), config.initialInputs.size()),
      time(0.0), state(config.initialState), inputs(config.initialInputs),
      initialState(config.initialState), initialInputs(config.initialInputs),
      dt(config.dt), setpoints(), controllers(),
      controllerConfig(
          config.controllerConfig) { // Validation 1: Check state and input
                                     // dimensions match TankModel expectations


  if (state.size() != constants::TANK_STATE_SIZE) {
    throw std::invalid_argument("Initial state size " +
                                std::to_string(state.size()) +
                                " does not match TankModel expectation of " +
                                std::to_string(constants::TANK_STATE_SIZE));
  }

  if (inputs.size() != constants::TANK_INPUT_SIZE) {
    throw std::invalid_argument("Initial inputs size " +
                                std::to_string(inputs.size()) +
                                " does not match TankModel expectation of " +
                                std::to_string(constants::TANK_INPUT_SIZE));
  }

  // Validation 2: Check dt is positive and reasonable
  if (dt <= 0.0 || dt < constants::MIN_DT || dt > constants::MAX_DT) {
    throw std::invalid_argument(
        "dt must be positive and between " + std::to_string(constants::MIN_DT) +
        " and " + std::to_string(constants::MAX_DT) + " seconds");
  }

  // Validation 3: Check controller indices are in bounds
  for (size_t i = 0; i < config.controllerConfig.size(); ++i) {
    const auto &ctrl = config.controllerConfig[i];

    if (ctrl.measuredIndex < 0 ||
        static_cast<size_t>(ctrl.measuredIndex) >= state.size()) {
      throw std::invalid_argument(
          "Controller " + std::to_string(i) + " measured_index " +
          std::to_string(ctrl.measuredIndex) + " is out of bounds for state " +
          "vector of size " + std::to_string(state.size()));
    }

    if (ctrl.outputIndex < 0 ||
        static_cast<size_t>(ctrl.outputIndex) >= inputs.size()) {
      throw std::invalid_argument(
          "Controller " + std::to_string(i) + " output_index " +
          std::to_string(ctrl.outputIndex) + " is out of bounds for input " +
          "vector of size " + std::to_string(inputs.size()));
    }
  }

  // Validation 4: Create controllers
  for (const auto &ctrl_config : config.controllerConfig) {
    controllers.emplace_back(
        ctrl_config.gains, ctrl_config.bias, ctrl_config.minOutputLimit,
        ctrl_config.maxOutputLimit, ctrl_config.maxIntegralAccumulation);
  }

  // Validation 5: Initialize setpoints from config
  setpoints.resize(controllers.size());
  for (size_t i = 0; i < controllers.size(); ++i) {
    setpoints[i] = controllerConfig[i].initialSetpoint;
  }
}

void Simulator::step() {
  // step implementation
}

double Simulator::getTime() {
  // getTime implementation
  return 0.0;
}

Eigen::VectorXd Simulator::getState() {
  // getState implementation
  return Eigen::VectorXd();
}

Eigen::VectorXd Simulator::getInputs() {
  // getInputs implementation
  return Eigen::VectorXd();
}

double Simulator::getSetpoint(int index) {
  // getSetpoint implementation
  return 0.0;
}

double Simulator::getControllerOutput(int index) {
  // getControllerOutput implementation
  return 0.0;
}

double Simulator::getError(int index) {
  // getError implementation
  return 0.0;
}

void Simulator::setInput(int index, double value) {
  // setInput implementation
}

void Simulator::setSetpoint(int index, double value) {
  // setSetpoint implementation
}

void Simulator::setControllerGains(
    int index, const tank_sim::PIDController::Gains &gains) {
  // setControllerGains implementation
}

void Simulator::reset() {
  // reset implementation
}

} // namespace tank_sim
