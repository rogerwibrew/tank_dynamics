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
  
  // Initialize previous errors to zero (at steady state, error should be zero)
  previousErrors.resize(controllers.size(), 0.0);
}

void Simulator::step() {
  // Step 1: Integrate the model forward
  // Create a lambda that wraps TankModel's derivatives method to match
  // Stepper's DerivativeFunc signature: (double t, VectorXd state, VectorXd input) -> VectorXd
  auto derivative_func = [this](double t, const Eigen::VectorXd& state,
                                const Eigen::VectorXd& input) -> Eigen::VectorXd {
    return model.derivatives(state, input);
  };

  // Call Stepper's step method to integrate one time step
  // Uses RK4 integration with:
  // - Current time
  // - Time step dt
  // - Current state vector
  // - Current input vector (from PREVIOUS timestep)
  // - Derivative function
  state = stepper.step(time, dt, state, inputs, derivative_func);

  // Step 2: Advance simulation time
  time += dt;

  // Step 3: Update all controllers for NEXT step
  // For each controller, read measured value, calculate error, and compute output
  for (size_t i = 0; i < controllers.size(); ++i) {
    // Read the measured variable from current state using measured_index
    int measured_index = controllerConfig[i].measuredIndex;
    double measured_value = state(measured_index);

    // Calculate error as setpoint minus measured value
    double error = setpoints[i] - measured_value;

    // Calculate error derivative using backward finite difference
    // error_dot = (error - previous_error) / dt
    // This introduces a one-step delay but is standard for discrete-time PID
    double error_dot = (error - previousErrors[i]) / dt;

    // Call controller's compute method with error, error_dot, and dt
    double output = controllers[i].compute(error, error_dot, dt);

    // Write the controller output to the inputs vector at output_index
    int output_index = controllerConfig[i].outputIndex;
    inputs(output_index) = output;
    
    // Store current error for next derivative calculation
    previousErrors[i] = error;
  }
}

double Simulator::getTime() const {
  return time;
}

Eigen::VectorXd Simulator::getState() const {
  return state;
}

Eigen::VectorXd Simulator::getInputs() const {
  return inputs;
}

double Simulator::getSetpoint(int index) const {
  if (index < 0 || static_cast<size_t>(index) >= setpoints.size()) {
    throw std::out_of_range("Setpoint index " + std::to_string(index) +
                            " out of bounds for " + std::to_string(setpoints.size()) +
                            " controller(s)");
  }
  return setpoints[index];
}

double Simulator::getControllerOutput(int index) const {
  if (index < 0 || static_cast<size_t>(index) >= controllers.size()) {
    throw std::out_of_range("Controller index " + std::to_string(index) +
                            " out of bounds for " + std::to_string(controllers.size()) +
                            " controller(s)");
  }
  // Get the controller's output from the inputs vector
  int output_index = controllerConfig[index].outputIndex;
  return inputs(output_index);
}

double Simulator::getError(int index) const {
  if (index < 0 || static_cast<size_t>(index) >= controllers.size()) {
    throw std::out_of_range("Controller index " + std::to_string(index) +
                            " out of bounds for " + std::to_string(controllers.size()) +
                            " controller(s)");
  }
  // Calculate error: setpoint - measured_value
  int measured_index = controllerConfig[index].measuredIndex;
  double measured_value = state(measured_index);
  double setpoint = setpoints[index];
  return setpoint - measured_value;
}

void Simulator::setInput(int index, double value) {
  if (index < 0 || static_cast<size_t>(index) >= inputs.size()) {
    throw std::out_of_range("Input index " + std::to_string(index) +
                            " out of bounds for input vector of size " +
                            std::to_string(inputs.size()));
  }
  inputs(index) = value;
}

void Simulator::setSetpoint(int index, double value) {
  if (index < 0 || static_cast<size_t>(index) >= setpoints.size()) {
    throw std::out_of_range("Setpoint index " + std::to_string(index) +
                            " out of bounds for " + std::to_string(setpoints.size()) +
                            " controller(s)");
  }
  setpoints[index] = value;
}

void Simulator::setControllerGains(
    int index, const tank_sim::PIDController::Gains &gains) {
  if (index < 0 || static_cast<size_t>(index) >= controllers.size()) {
    throw std::out_of_range("Controller index " + std::to_string(index) +
                            " out of bounds for " + std::to_string(controllers.size()) +
                            " controller(s)");
  }
  controllers[index].setGains(gains);
}

void Simulator::reset() {
  // Reset simulation to initial conditions
  time = 0.0;
  state = initialState;
  inputs = initialInputs;
  
  // Reset all controller integral states
  for (auto& controller : controllers) {
    controller.reset();
  }
  
  // Reset setpoints to initial values
  for (size_t i = 0; i < controllers.size(); ++i) {
    setpoints[i] = controllerConfig[i].initialSetpoint;
  }
  
  // Reset previous errors to zero (at steady state)
  std::fill(previousErrors.begin(), previousErrors.end(), 0.0);
}

int Simulator::getControllerCount() const {
  return static_cast<int>(controllers.size());
}

} // namespace tank_sim
