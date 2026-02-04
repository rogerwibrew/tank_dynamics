/**
 * @file bindings.cpp
 * @brief Python bindings for the Tank Dynamics Simulator using pybind11
 *
 * This file creates the pybind11 module that exposes the C++ simulation
 * library to Python. The module provides a high-performance tank level
 * control simulator with PID feedback control.
 *
 * The module is named _tank_sim (with underscore) to indicate it's the
 * internal C++ extension module. The public Python API is provided by
 * the tank_sim package which imports from this module.
 */

#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/stl.h>

#include "simulator.h"
#include "tank_model.h"
#include "pid_controller.h"
#include "stepper.h"

namespace py = pybind11;

/**
 * @brief Returns the version string for the tank_sim module.
 * @return Version string in semantic versioning format.
 */
std::string get_version() {
    return "0.1.0";
}

/**
 * @brief pybind11 module definition
 *
 * PYBIND11_MODULE macro creates the Python extension module.
 * The first argument (_tank_sim) becomes the module name.
 * The second argument (m) is the module object used to add bindings.
 */
PYBIND11_MODULE(_tank_sim, m) {
    m.doc() = R"pbdoc(
        Tank Dynamics Simulator - C++ Extension Module
        ===============================================

        This is the internal C++ extension module for the tank_sim package.
        It provides Python bindings to a high-performance simulation engine
        for tank level control with PID feedback.

        Features:
        - Real-time tank material balance (ODE integration using GSL RK4)
        - PID feedback control with anti-windup protection
        - Valve dynamics and flow calculations
        - Step-by-step simulation with configurable time steps

        This module is not intended for direct import. Use the tank_sim
        package instead:

            import tank_sim
            print(tank_sim.get_version())

        Full class bindings (Simulator, SimulatorConfig, etc.) will be
        added in subsequent development tasks.
    )pbdoc";

    // Module version function
    m.def("get_version", &get_version, R"pbdoc(
        Get the version of the tank_sim module.

        Returns:
            str: Version string in semantic versioning format (e.g., "0.1.0").
    )pbdoc");

    // ========================================================================
    // TankModel::Parameters binding
    // ========================================================================
    py::class_<tank_sim::TankModel::Parameters>(m, "TankModelParameters", R"pbdoc(
        Configuration parameters for the tank physics model.

        This class represents the physical characteristics of the tank system.
        All parameters are read-write and can be modified at any time.

        Attributes:
            area (float): Cross-sectional area of the tank in m².
                         Must be positive. Larger area means slower level changes.
            k_v (float): Valve discharge coefficient in m^2.5/s.
                        Determines the relationship between valve position and outlet flow.
                        For the standard tank, k_v = 1.2649.
            max_height (float): Maximum tank height in meters.
                               Physical limit of the tank. Typically 5.0 m.

        Example:
            >>> params = TankModelParameters()
            >>> params.area = 120.0
            >>> params.k_v = 1.2649
            >>> params.max_height = 5.0
    )pbdoc")
        .def(py::init<>())
        .def_readwrite("area", &tank_sim::TankModel::Parameters::area,
                      "Cross-sectional area (m²)")
        .def_readwrite("k_v", &tank_sim::TankModel::Parameters::k_v,
                      "Valve discharge coefficient (m^2.5/s)")
        .def_readwrite("max_height", &tank_sim::TankModel::Parameters::max_height,
                      "Maximum tank height (m)");

    // ========================================================================
    // PIDController::Gains binding
    // ========================================================================
    py::class_<tank_sim::PIDController::Gains>(m, "PIDGains", R"pbdoc(
        PID controller gain parameters.

        This class contains the three tuning parameters for a PID controller:
        proportional, integral, and derivative gains. All parameters are read-write
        and can be modified at any time for dynamic retuning.

        Attributes:
            Kc (float): Proportional gain (dimensionless).
                       Controls the immediate response to error. Negative values
                       implement reverse-acting control (e.g., for outlet valve).
                       Typical range: -5.0 to +5.0 depending on system dynamics.
            tau_I (float): Integral time constant in seconds.
                          Controls long-term error correction. Larger values mean
                          slower integral action. Set to 0 to disable integral action.
                          Typical range: 5.0 to 50.0 seconds.
            tau_D (float): Derivative time constant in seconds.
                          Provides predictive damping based on error rate of change.
                          Set to 0 to disable derivative action.
                          Typical range: 0.5 to 5.0 seconds.

        Example:
            >>> gains = PIDGains()
            >>> gains.Kc = -1.0      # Reverse-acting
            >>> gains.tau_I = 10.0   # 10 second integral
            >>> gains.tau_D = 1.0    # 1 second derivative
    )pbdoc")
        .def(py::init<>())
        .def_readwrite("Kc", &tank_sim::PIDController::Gains::Kc,
                      "Proportional gain (dimensionless)")
        .def_readwrite("tau_I", &tank_sim::PIDController::Gains::tau_I,
                      "Integral time constant (seconds)")
        .def_readwrite("tau_D", &tank_sim::PIDController::Gains::tau_D,
                      "Derivative time constant (seconds)");

    // ========================================================================
    // Simulator::ControllerConfig binding
    // ========================================================================
    py::class_<tank_sim::Simulator::ControllerConfig>(m, "ControllerConfig", R"pbdoc(
        Configuration for a single feedback controller.

        This class bundles all settings needed to define one PID controller
        that regulates a measurement toward a setpoint by adjusting an output.
        All fields are read-write and can be modified at any time.

        Attributes:
            gains (PIDGains): PID controller tuning parameters (Kc, tau_I, tau_D).
            bias (float): Output value when error is zero (unitless, 0-1 range).
                         For a valve, this is the "nominal" position at steady state.
                         Typical value: 0.5 (middle of range).
            min_output (float): Minimum output saturation limit (e.g., 0.0 for valve closed).
            max_output (float): Maximum output saturation limit (e.g., 1.0 for valve fully open).
            max_integral (float): Maximum magnitude of integral accumulator clamping.
                                 Prevents integral windup. Typical value: 10.0.
            measured_index (int): Index of the state variable being measured (usually 0 for level).
            output_index (int): Index of the input variable being adjusted (usually 1 for valve).
            initial_setpoint (float): Initial target value for the controlled variable.
                                    For a tank level controller, typically 2.5 m.

        Example:
            >>> config = ControllerConfig()
            >>> config.gains = PIDGains()
            >>> config.gains.Kc = -1.0
            >>> config.gains.tau_I = 10.0
            >>> config.gains.tau_D = 1.0
            >>> config.bias = 0.5
            >>> config.min_output = 0.0
            >>> config.max_output = 1.0
            >>> config.max_integral = 10.0
            >>> config.measured_index = 0
            >>> config.output_index = 1
            >>> config.initial_setpoint = 2.5
    )pbdoc")
        .def(py::init<>())
        .def_readwrite("gains", &tank_sim::Simulator::ControllerConfig::gains,
                      "PID controller gains")
        .def_readwrite("bias", &tank_sim::Simulator::ControllerConfig::bias,
                      "Output bias (setpoint zero value)")
        .def_readwrite("min_output", &tank_sim::Simulator::ControllerConfig::minOutputLimit,
                      "Minimum output saturation limit")
        .def_readwrite("max_output", &tank_sim::Simulator::ControllerConfig::maxOutputLimit,
                      "Maximum output saturation limit")
        .def_readwrite("max_integral", &tank_sim::Simulator::ControllerConfig::maxIntegralAccumulation,
                      "Maximum integral accumulator magnitude")
        .def_readwrite("measured_index", &tank_sim::Simulator::ControllerConfig::measuredIndex,
                      "Index of measured state variable")
        .def_readwrite("output_index", &tank_sim::Simulator::ControllerConfig::outputIndex,
                      "Index of output/input variable")
        .def_readwrite("initial_setpoint", &tank_sim::Simulator::ControllerConfig::initialSetpoint,
                      "Initial controller setpoint");

    // ========================================================================
    // Simulator::Config binding
    // ========================================================================
    py::class_<tank_sim::Simulator::Config>(m, "SimulatorConfig", R"pbdoc(
        Complete configuration for the Simulator.

        This class bundles all settings needed to initialize a Simulator:
        tank physics parameters, controller configurations, initial state,
        initial inputs, and timestep. All fields are read-write.

        Attributes:
            model_params (TankModelParameters): Physical parameters of the tank
                                              (area, k_v, max_height).
            controllers (list[ControllerConfig]): List of controller configurations.
                                                 For a single tank, usually one controller.
                                                 Can be empty if running open-loop.
            initial_state (numpy.ndarray): Initial state vector. For a single tank,
                                          this is a 1D array with one element: [level_m].
                                          Example: np.array([2.5]) for 2.5 meters.
            initial_inputs (numpy.ndarray): Initial input vector [q_in, valve_position].
                                           q_in is inlet flow (m³/s), typically 1.0.
                                           valve_position (0-1), typically 0.5.
            dt (float): Simulation timestep in seconds. Typical value: 1.0.

        Example:
            >>> config = SimulatorConfig()
            >>> config.model_params = TankModelParameters()
            >>> config.model_params.area = 120.0
            >>> config.model_params.k_v = 1.2649
            >>> config.model_params.max_height = 5.0
            >>> config.controllers = [ControllerConfig()]
            >>> config.initial_state = np.array([2.5])
            >>> config.initial_inputs = np.array([1.0, 0.5])
            >>> config.dt = 1.0
    )pbdoc")
        .def(py::init<>())
        .def_readwrite("model_params", &tank_sim::Simulator::Config::params,
                      "Tank physics parameters")
        .def_readwrite("controllers", &tank_sim::Simulator::Config::controllerConfig,
                      "List of controller configurations")
        .def_property("initial_state",
                      [](const tank_sim::Simulator::Config& self) -> Eigen::VectorXd {
                          return self.initialState;
                      },
                      [](tank_sim::Simulator::Config& self, const Eigen::Ref<const Eigen::VectorXd>& val) {
                          self.initialState = val;
                      },
                      "Initial state vector (as numpy array)")
        .def_property("initial_inputs",
                      [](const tank_sim::Simulator::Config& self) -> Eigen::VectorXd {
                          return self.initialInputs;
                      },
                      [](tank_sim::Simulator::Config& self, const Eigen::Ref<const Eigen::VectorXd>& val) {
                          self.initialInputs = val;
                      },
                      "Initial inputs vector (as numpy array)")
        .def_readwrite("dt", &tank_sim::Simulator::Config::dt,
                      "Simulation timestep (seconds)");

    // ========================================================================
    // Simulator class binding
    // ========================================================================
    py::class_<tank_sim::Simulator>(m, "Simulator", R"pbdoc(
        Real-time tank dynamics simulator with feedback control.

        The Simulator orchestrates the entire control loop: reading measurements,
        computing control outputs from configured PID controllers, advancing
        the tank physics model by one timestep using ODE integration, and
        updating states and inputs. All computation happens at 1 Hz in real time.

        The simulator maintains internal state (level, time, controller integral
        accumulators) and logs historical data for WebSocket streaming.

        Example:
            >>> config = tank_sim.create_default_config()
            >>> sim = tank_sim.Simulator(config)
            >>> for i in range(100):
            ...     sim.step()
            ...     print(f"t={sim.get_time()}, level={sim.get_state()[0]:.2f}")

        All getter methods are read-only and do not modify simulator state.
        Setter methods (set_input, set_setpoint, set_controller_gains) modify
        state and affect subsequent simulation steps.
    )pbdoc")
        .def(py::init<const tank_sim::Simulator::Config&>(), py::arg("config"),
             R"pbdoc(
                Initialize a Simulator with the given configuration.

                Args:
                    config (SimulatorConfig): Complete simulator configuration.

                Raises:
                    ValueError: If configuration is invalid (e.g., empty state vector).

                Note:
                    The simulator copies the configuration, so modifying the
                    original config object does not affect the simulator.
             )pbdoc")

        // Core simulation method
        .def("step", &tank_sim::Simulator::step, R"pbdoc(
            Advance the simulation by one timestep.

            This method:
            1. Computes control outputs from all configured PID controllers
            2. Updates controller integral accumulators
            3. Integrates the tank physics ODE by dt seconds
            4. Updates state, inputs, and time

            Should be called at regular intervals (e.g., every 1 second).

            Returns:
                None
        )pbdoc")

        // State getters (all const, non-modifying)
        .def("get_time", &tank_sim::Simulator::getTime, R"pbdoc(
            Get the current simulation time in seconds.

            Returns:
                float: Elapsed time since initialization.
        )pbdoc")

        .def("get_state", &tank_sim::Simulator::getState, R"pbdoc(
            Get the current state vector as a numpy array.

            For a single tank, this is a 1D array with one element: [level_m].

            Returns:
                numpy.ndarray: Current state vector (float64, 1D array).

            Note:
                The returned array is a copy. Modifying it does not affect the
                simulator. Use set_state() or reset() to change state.
        )pbdoc")

        .def("get_inputs", &tank_sim::Simulator::getInputs, R"pbdoc(
            Get the current input vector as a numpy array.

            This is a 1D array [q_in, valve_position] where:
            - q_in is inlet flow rate (m³/s)
            - valve_position is valve opening (0-1)

            Returns:
                numpy.ndarray: Current inputs vector (float64, 1D array).

            Note:
                Use set_input() to modify individual inputs.
        )pbdoc")

        .def("get_setpoint", 
             [](const tank_sim::Simulator& self, int index) {
                 if (index < 0 || index >= self.getControllerCount()) {
                     throw py::index_error(
                         "Controller index " + std::to_string(index) + 
                         " out of range (have " + std::to_string(self.getControllerCount()) + 
                         " controller" + (self.getControllerCount() == 1 ? "" : "s") + ")"
                     );
                 }
                 return self.getSetpoint(index);
             },
             py::arg("index"), R"pbdoc(
            Get the setpoint for a specific controller.

            Args:
                index (int): Controller index (0-based).

            Returns:
                float: Target setpoint value.

            Raises:
                IndexError: If index is out of range.

            Example:
                >>> sim = tank_sim.Simulator(config)
                >>> setpoint = sim.get_setpoint(0)  # First controller
        )pbdoc")

        .def("get_controller_output",
             [](const tank_sim::Simulator& self, int index) {
                 if (index < 0 || index >= self.getControllerCount()) {
                     throw py::index_error(
                         "Controller index " + std::to_string(index) + 
                         " out of range (have " + std::to_string(self.getControllerCount()) + 
                         " controller" + (self.getControllerCount() == 1 ? "" : "s") + ")"
                     );
                 }
                 return self.getControllerOutput(index);
             },
             py::arg("index"), R"pbdoc(
            Get the control output from a specific controller.

            This is the value that would be written to the controlled input
            (e.g., valve position) after clamping to [min_output, max_output].

            Args:
                index (int): Controller index (0-based).

            Returns:
                float: Control output value (clamped to limits).

            Raises:
                IndexError: If index is out of range.

            Example:
                >>> output = sim.get_controller_output(0)  # Valve position
        )pbdoc")

        .def("get_error",
             [](const tank_sim::Simulator& self, int index) {
                 if (index < 0 || index >= self.getControllerCount()) {
                     throw py::index_error(
                         "Controller index " + std::to_string(index) + 
                         " out of range (have " + std::to_string(self.getControllerCount()) + 
                         " controller" + (self.getControllerCount() == 1 ? "" : "s") + ")"
                     );
                 }
                 return self.getError(index);
             },
             py::arg("index"), R"pbdoc(
            Get the current control error for a specific controller.

            Error = setpoint - measured_value. Positive means below setpoint.

            Args:
                index (int): Controller index (0-based).

            Returns:
                float: Current control error.

            Raises:
                IndexError: If index is out of range.

            Example:
                >>> error = sim.get_error(0)  # How far from setpoint?
        )pbdoc")

        // Setters (modify simulator state for next step)
        .def("set_input", &tank_sim::Simulator::setInput,
             py::arg("index"), py::arg("value"), R"pbdoc(
            Set a specific input value.

            Use this to change inlet flow or other disturbances during simulation.

            Args:
                index (int): Input index (0-based). For the standard tank:
                            0 = inlet flow (q_in), 1 = valve position
                value (float): New input value.

            Raises:
                IndexError: If index is out of range.

            Example:
                >>> sim.set_input(0, 1.2)  # Change inlet flow to 1.2 m³/s
        )pbdoc")

        .def("set_setpoint",
             [](tank_sim::Simulator& self, int index, double value) {
                 if (index < 0 || index >= self.getControllerCount()) {
                     throw py::index_error(
                         "Controller index " + std::to_string(index) + 
                         " out of range (have " + std::to_string(self.getControllerCount()) + 
                         " controller" + (self.getControllerCount() == 1 ? "" : "s") + ")"
                     );
                 }
                 self.setSetpoint(index, value);
             },
             py::arg("index"), py::arg("value"), R"pbdoc(
            Change the setpoint for a specific controller.

            This triggers a new control response. The controller will adjust
            outputs to move the measured variable toward the new setpoint.

            Args:
                index (int): Controller index (0-based).
                value (float): New setpoint value.

            Raises:
                IndexError: If index is out of range.

            Example:
                >>> sim.set_setpoint(0, 3.5)  # New level target: 3.5 meters
        )pbdoc")

        .def("set_controller_gains",
             [](tank_sim::Simulator& self, int index, const tank_sim::PIDController::Gains& gains) {
                 if (index < 0 || index >= self.getControllerCount()) {
                     throw py::index_error(
                         "Controller index " + std::to_string(index) + 
                         " out of range (have " + std::to_string(self.getControllerCount()) + 
                         " controller" + (self.getControllerCount() == 1 ? "" : "s") + ")"
                     );
                 }
                 self.setControllerGains(index, gains);
             },
             py::arg("index"), py::arg("gains"), R"pbdoc(
            Dynamically retune a controller's PID gains.

            This allows runtime tuning without resetting the controller or
            resetting integral accumulator (bumpless transfer).

            Args:
                index (int): Controller index (0-based).
                gains (PIDGains): New PID gain parameters (Kc, tau_I, tau_D).

            Raises:
                IndexError: If index is out of range.

            Example:
                >>> new_gains = tank_sim.PIDGains()
                >>> new_gains.Kc = -2.0
                >>> new_gains.tau_I = 5.0
                >>> new_gains.tau_D = 0.5
                >>> sim.set_controller_gains(0, new_gains)
        )pbdoc")

        .def("reset", &tank_sim::Simulator::reset, R"pbdoc(
            Reset the simulator to initial conditions.

            This resets:
            - Time to 0.0
            - State to initial_state
            - Inputs to initial_inputs
            - All controller integral accumulators to zero
            - All setpoints to initial setpoints

            After reset(), the simulator can be run again from the beginning
            and will produce identical results (assuming same inputs/setpoints).

            Returns:
                None

            Example:
                >>> sim.step()  # Run one step
                >>> sim.reset()  # Back to beginning
                >>> sim.step()  # Produces identical result
        )pbdoc");
}
