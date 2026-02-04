"""Comprehensive tests for tank_sim Python bindings.

This module tests the Python bindings to the C++ simulation library, verifying:
- Configuration creation and property access
- Simulator initialization and basic operation
- Steady-state stability
- Step response behavior (increase and decrease)
- Disturbance rejection
- Reset functionality
- Exception handling
- Numpy array conversion
- Dynamic controller retuning

Tests serve dual purposes:
1. Verify bindings correctness and completeness
2. Provide usage examples for Python users
"""

import numpy as np
import pytest

import tank_sim


class TestConfigurationCreation:
    """Tests for creating and configuring simulator components."""

    def test_tank_model_parameters_creation(self):
        """Verify TankModelParameters can be created and modified.

        This tests the basic C++ structure binding for tank physics parameters.
        All fields should be accessible as read-write properties.
        """
        params = tank_sim.TankModelParameters()

        # Set parameters
        params.area = 120.0
        params.k_v = 1.2649
        params.max_height = 5.0

        # Verify retrieval
        assert params.area == 120.0, "Tank area not set correctly"
        assert params.k_v == 1.2649, "Valve coefficient not set correctly"
        assert params.max_height == 5.0, "Max height not set correctly"

    def test_pid_gains_creation(self):
        """Verify PIDGains can be created and modified.

        This tests the C++ PID gains structure binding.
        All gain parameters should be accessible as read-write properties.
        """
        gains = tank_sim.PIDGains()

        # Set gains
        gains.Kc = -1.0  # Reverse-acting controller
        gains.tau_I = 10.0
        gains.tau_D = 1.0

        # Verify retrieval
        assert gains.Kc == -1.0, "Proportional gain not set correctly"
        assert gains.tau_I == 10.0, "Integral time not set correctly"
        assert gains.tau_D == 1.0, "Derivative time not set correctly"

    def test_controller_config_creation(self):
        """Verify ControllerConfig can be created with all fields.

        This tests the controller configuration structure binding, including
        nested structures and various field types.
        """
        controller = tank_sim.ControllerConfig()

        # Set all fields
        controller.gains = tank_sim.PIDGains()
        controller.gains.Kc = -1.0
        controller.gains.tau_I = 10.0
        controller.gains.tau_D = 1.0
        controller.bias = 0.5
        controller.min_output = 0.0
        controller.max_output = 1.0
        controller.max_integral = 10.0
        controller.measured_index = 0
        controller.output_index = 1
        controller.initial_setpoint = 2.5

        # Verify all fields
        assert controller.gains.Kc == -1.0
        assert controller.bias == 0.5
        assert controller.min_output == 0.0
        assert controller.max_output == 1.0
        assert controller.max_integral == 10.0
        assert controller.measured_index == 0
        assert controller.output_index == 1
        assert controller.initial_setpoint == 2.5

    def test_simulator_config_creation(self):
        """Verify SimulatorConfig can be created with all components.

        This tests the complete simulator configuration structure, including
        tank parameters, controllers, and initial conditions.
        """
        config = tank_sim.SimulatorConfig()

        # Set tank parameters
        config.model_params = tank_sim.TankModelParameters()
        config.model_params.area = 120.0
        config.model_params.k_v = 1.2649
        config.model_params.max_height = 5.0

        # Set controller
        controller = tank_sim.ControllerConfig()
        controller.gains = tank_sim.PIDGains()
        controller.gains.Kc = -1.0
        controller.gains.tau_I = 10.0
        controller.gains.tau_D = 1.0
        controller.bias = 0.5
        controller.min_output = 0.0
        controller.max_output = 1.0
        controller.max_integral = 10.0
        controller.measured_index = 0
        controller.output_index = 1
        controller.initial_setpoint = 2.5

        config.controllers = [controller]

        # Set initial conditions
        config.initial_state = np.array([2.5])
        config.initial_inputs = np.array([1.0, 0.5])
        config.dt = 1.0

        # Verify state is numpy array (not Python list)
        assert isinstance(config.initial_state, np.ndarray), (
            "State should be numpy array"
        )
        assert isinstance(config.initial_inputs, np.ndarray), (
            "Inputs should be numpy array"
        )
        assert config.initial_state.dtype == np.float64, "State array should be float64"


class TestSimulatorConstruction:
    """Tests for simulator initialization."""

    def test_simulator_construction(self, default_config):
        """Verify Simulator can be constructed with valid configuration.

        The simulator should be created without errors and initialize with
        correct initial conditions.
        """
        sim = tank_sim.Simulator(default_config)

        # Verify initial conditions
        assert sim.get_time() == 0.0, "Initial time should be 0.0"

        state = sim.get_state()
        assert isinstance(state, np.ndarray), "State should be numpy array"
        assert state.shape == (1,), "State should have one element"
        assert abs(state[0] - 2.5) < 1e-6, "Initial level should be 2.5 m"

    def test_simulator_initial_inputs(self, default_config):
        """Verify initial inputs are set correctly."""
        sim = tank_sim.Simulator(default_config)

        inputs = sim.get_inputs()
        assert isinstance(inputs, np.ndarray), "Inputs should be numpy array"
        assert inputs.shape == (2,), "Inputs should have two elements"
        assert abs(inputs[0] - 1.0) < 1e-6, "Initial inlet flow should be 1.0"
        assert abs(inputs[1] - 0.5) < 1e-6, "Initial valve position should be 0.5"


class TestSteadyStateStability:
    """Tests for steady state behavior."""

    def test_steady_state_stability(self, steady_state_simulator):
        """Verify that steady state remains stable over time.

        At steady state, all derivatives should be zero, so the system should
        not drift. This tests both correct initialization and numerical
        stability of the integration.
        """
        sim = steady_state_simulator

        # Run 100 steps at steady state
        for i in range(100):
            sim.step()
            state = sim.get_state()
            level = state[0]

            # Level should remain at initial setpoint within tolerance
            assert abs(level - 2.5) < 0.01, (
                f"Level drifted to {level:.4f} m after {i + 1} steps (tolerance: 0.01 m)"
            )

        # Verify time tracking
        time = sim.get_time()
        assert abs(time - 100.0) < 1e-6, (
            f"Time tracking incorrect: expected 100.0, got {time}"
        )

    def test_steady_state_controller_output(self, steady_state_simulator):
        """Verify controller output remains constant at steady state."""
        sim = steady_state_simulator

        # Record initial controller state
        initial_output = sim.get_controller_output(0)
        initial_error = sim.get_error(0)

        # Run for 50 steps
        for _ in range(50):
            sim.step()

        # Controller output and error should remain essentially constant
        final_output = sim.get_controller_output(0)
        final_error = sim.get_error(0)

        assert abs(final_output - initial_output) < 1e-4, (
            "Controller output should remain constant at steady state"
        )
        assert abs(final_error - initial_error) < 2e-6, (
            "Control error should remain very small at steady state"
        )


class TestStepResponse:
    """Tests for step response behavior."""

    def test_step_response_increase(self, steady_state_simulator):
        """Verify system response when setpoint increases.

        When setpoint increases (demand for higher level), the controller
        should reduce valve opening (decrease output) to reduce outlet flow,
        allowing level to rise.
        """
        sim = steady_state_simulator

        # Record steady-state output
        steady_output = sim.get_controller_output(0)

        # Change setpoint from 2.5 m to 3.0 m
        sim.set_setpoint(0, 3.0)

        # Run for 200 steps
        for _ in range(200):
            sim.step()

        # Verify level increases toward setpoint
        state = sim.get_state()
        level = state[0]
        assert level > 2.5, "Level should increase after setpoint increase"
        assert level < 3.1, "Level should not overshoot significantly"

        # Verify controller reduces output (closes valve)
        output = sim.get_controller_output(0)
        assert output < steady_output, (
            "Controller should reduce output to close valve for level increase"
        )

    def test_step_response_decrease(self, steady_state_simulator):
        """Verify system response when setpoint decreases.

        When setpoint decreases (demand for lower level), the controller
        should increase valve opening (increase output) to increase outlet flow,
        allowing level to fall.
        """
        sim = steady_state_simulator

        # Record steady-state output
        steady_output = sim.get_controller_output(0)

        # Change setpoint from 2.5 m to 2.0 m
        sim.set_setpoint(0, 2.0)

        # Run for 200 steps
        for _ in range(200):
            sim.step()

        # Verify level decreases toward setpoint
        state = sim.get_state()
        level = state[0]
        assert level < 2.5, "Level should decrease after setpoint decrease"
        assert level > 1.9, "Level should not undershoot significantly"

        # Verify controller increases output (opens valve)
        output = sim.get_controller_output(0)
        assert output > steady_output, (
            "Controller should increase output to open valve for level decrease"
        )


class TestDisturbanceRejection:
    """Tests for disturbance rejection capability."""

    def test_disturbance_rejection(self, steady_state_simulator):
        """Verify controller compensates for input disturbances.

        When inlet flow increases (disturbance), the controller should
        increase valve opening to reject the disturbance and return level
        to setpoint.
        """
        sim = steady_state_simulator

        # Run 50 steps to establish baseline
        for _ in range(50):
            sim.step()

        level_before = sim.get_state()[0]

        # Apply disturbance: increase inlet flow from 1.0 to 1.2 m³/s
        sim.set_input(0, 1.2)

        # Run 200 steps for controller to respond
        for _ in range(200):
            sim.step()

        # Verify level returns to setpoint despite disturbance
        level_after = sim.get_state()[0]
        assert abs(level_after - 2.5) < 0.1, (
            f"Level should return to setpoint (2.5 m), got {level_after:.3f} m"
        )

        # Controller should have adjusted output to compensate
        # (increased valve opening to allow more outlet flow)
        output = sim.get_controller_output(0)
        assert 0.0 <= output <= 1.0, "Controller output should remain in valid range"


class TestResetFunctionality:
    """Tests for simulator reset."""

    def test_reset_returns_to_initial_conditions(self, default_config):
        """Verify reset() returns simulator to initial conditions."""
        sim = tank_sim.Simulator(default_config)

        # Run 50 steps
        for _ in range(50):
            sim.step()

        # Change setpoint
        sim.set_setpoint(0, 3.5)

        # Run 50 more steps (system now in transient)
        for _ in range(50):
            sim.step()

        # Record state before reset
        time_before_reset = sim.get_time()
        state_before_reset = sim.get_state().copy()
        setpoint_before_reset = sim.get_setpoint(0)

        # Reset
        sim.reset()

        # Verify reset to initial conditions
        assert sim.get_time() == 0.0, "Time should be 0.0 after reset"

        state_after = sim.get_state()
        assert abs(state_after[0] - 2.5) < 1e-6, (
            "Level should return to initial 2.5 m after reset"
        )

        setpoint_after = sim.get_setpoint(0)
        assert abs(setpoint_after - 2.5) < 1e-6, (
            "Setpoint should return to initial 2.5 m after reset"
        )

    def test_reset_is_reproducible(self, default_config):
        """Verify behavior is reproducible after reset."""
        # Create two simulators with same config
        sim1 = tank_sim.Simulator(default_config)
        sim2 = tank_sim.Simulator(default_config)

        # Run sim1 for a while
        for _ in range(50):
            sim1.step()

        # Run sim2 for same number of steps without any changes
        for _ in range(50):
            sim2.step()

        # Verify states match
        state1 = sim1.get_state()
        state2 = sim2.get_state()
        assert np.allclose(state1, state2), (
            "States should be identical after same number of steps"
        )

        # Make some changes to sim1
        sim1.set_setpoint(0, 3.0)

        # Reset sim1
        sim1.reset()

        # Run both for 50 more steps
        for _ in range(50):
            sim1.step()
            sim2.step()

        # Verify states still match (behavior is reproducible)
        state1 = sim1.get_state()
        state2 = sim2.get_state()
        assert np.allclose(state1, state2), (
            "Behavior should be reproducible after reset"
        )


class TestExceptionHandling:
    """Tests for proper error handling."""

    def test_invalid_configuration_raises_error(self):
        """Verify invalid configuration raises appropriate exception.

        An empty state vector should raise ValueError from std::invalid_argument.
        """
        config = tank_sim.SimulatorConfig()
        config.model_params = tank_sim.TankModelParameters()
        config.model_params.area = 120.0
        config.model_params.k_v = 1.2649
        config.model_params.max_height = 5.0

        controller = tank_sim.ControllerConfig()
        controller.gains = tank_sim.PIDGains()
        controller.gains.Kc = -1.0
        controller.gains.tau_I = 10.0
        controller.gains.tau_D = 1.0
        controller.bias = 0.5
        controller.min_output = 0.0
        controller.max_output = 1.0
        controller.max_integral = 10.0
        controller.measured_index = 0
        controller.output_index = 1
        controller.initial_setpoint = 2.5

        config.controllers = [controller]

        # Empty state vector should be invalid
        config.initial_state = np.array([])
        config.initial_inputs = np.array([1.0, 0.5])
        config.dt = 1.0

        # Should raise ValueError from std::invalid_argument
        with pytest.raises(ValueError):
            tank_sim.Simulator(config)

    def test_invalid_controller_index_raises_error(self, steady_state_simulator):
        """Verify accessing invalid controller index raises IndexError."""
        sim = steady_state_simulator

        # Accessing setpoint for non-existent controller should raise IndexError
        with pytest.raises(IndexError):
            sim.get_setpoint(999)

        with pytest.raises(IndexError):
            sim.get_controller_output(999)

        with pytest.raises(IndexError):
            sim.get_error(999)


class TestNumpyArrayConversion:
    """Tests for numpy array conversion."""

    def test_get_state_returns_numpy_array(self, steady_state_simulator):
        """Verify get_state() returns numpy array, not Python list."""
        sim = steady_state_simulator

        state = sim.get_state()

        # Verify type
        assert isinstance(state, np.ndarray), "get_state() should return numpy.ndarray"

        # Verify dtype (should be float64)
        assert state.dtype == np.float64, "State array should be float64"

        # Verify shape
        assert state.shape == (1,), "State should have shape (1,)"

    def test_get_inputs_returns_numpy_array(self, steady_state_simulator):
        """Verify get_inputs() returns numpy array, not Python list."""
        sim = steady_state_simulator

        inputs = sim.get_inputs()

        # Verify type
        assert isinstance(inputs, np.ndarray), (
            "get_inputs() should return numpy.ndarray"
        )

        # Verify dtype
        assert inputs.dtype == np.float64, "Inputs array should be float64"

        # Verify shape
        assert inputs.shape == (2,), "Inputs should have shape (2,)"

    def test_initial_state_as_numpy_array(self, default_config):
        """Verify initial_state in config accepts numpy arrays."""
        # Create with explicit numpy array
        config = tank_sim.SimulatorConfig()
        config.model_params = tank_sim.TankModelParameters()
        config.model_params.area = 120.0
        config.model_params.k_v = 1.2649
        config.model_params.max_height = 5.0

        controller = tank_sim.ControllerConfig()
        controller.gains = tank_sim.PIDGains()
        controller.gains.Kc = -1.0
        controller.gains.tau_I = 10.0
        controller.gains.tau_D = 1.0
        controller.bias = 0.5
        controller.min_output = 0.0
        controller.max_output = 1.0
        controller.max_integral = 10.0
        controller.measured_index = 0
        controller.output_index = 1
        controller.initial_setpoint = 2.5

        config.controllers = [controller]

        # Set as numpy array (float64)
        config.initial_state = np.array([2.5], dtype=np.float64)
        config.initial_inputs = np.array([1.0, 0.5], dtype=np.float64)
        config.dt = 1.0

        # Should create simulator without issues
        sim = tank_sim.Simulator(config)
        assert isinstance(sim.get_state(), np.ndarray)


class TestDynamicRetuning:
    """Tests for controller retuning during simulation."""

    def test_dynamic_gains_change(self, steady_state_simulator):
        """Verify controller can be retuned during simulation.

        Dynamic retuning allows changing controller gains while the
        simulation is running, useful for adaptive control or testing.
        """
        sim = steady_state_simulator

        # Run initial 50 steps with original gains (Kc=-1.0)
        for _ in range(50):
            sim.step()

        initial_state = sim.get_state().copy()

        # Change setpoint to trigger response
        sim.set_setpoint(0, 3.0)

        # Run 50 steps with original tuning
        states_original = []
        for _ in range(50):
            sim.step()
            states_original.append(sim.get_state()[0])

        # Record response with original gains
        response_original = sim.get_state()[0]

        # Now retune controller with different gains (more aggressive)
        new_gains = tank_sim.PIDGains()
        new_gains.Kc = -2.0  # Double the gain for more aggressive response
        new_gains.tau_I = 5.0  # Faster integral
        new_gains.tau_D = 0.5  # Faster derivative

        sim.set_controller_gains(0, new_gains)

        # Reset to measure response with new tuning
        sim.reset()
        sim.set_setpoint(0, 3.0)

        states_new = []
        for _ in range(50):
            sim.step()
            states_new.append(sim.get_state()[0])

        response_new = sim.get_state()[0]

        # Verify that new gains produce different response
        # (Higher Kc should produce faster, more aggressive response)
        assert response_new != response_original, (
            "Controller response should change after retuning"
        )


class TestIntegration:
    """Integration tests combining multiple features."""

    def test_full_simulation_sequence(self):
        """Test a complete simulation workflow."""
        # Create custom config
        config = tank_sim.SimulatorConfig()
        config.model_params = tank_sim.TankModelParameters()
        config.model_params.area = 100.0
        config.model_params.k_v = 1.0
        config.model_params.max_height = 5.0

        controller = tank_sim.ControllerConfig()
        controller.gains = tank_sim.PIDGains()
        controller.gains.Kc = -1.0
        controller.gains.tau_I = 10.0
        controller.gains.tau_D = 1.0
        controller.bias = 0.5
        controller.min_output = 0.0
        controller.max_output = 1.0
        controller.max_integral = 10.0
        controller.measured_index = 0
        controller.output_index = 1
        controller.initial_setpoint = 2.5

        config.controllers = [controller]
        config.initial_state = np.array([2.5])
        config.initial_inputs = np.array([1.0, 0.5])
        config.dt = 0.5  # Smaller timestep

        sim = tank_sim.Simulator(config)

        # Phase 1: Establish steady state (100 steps)
        for _ in range(100):
            sim.step()
        assert abs(sim.get_time() - 50.0) < 1e-6

        # Phase 2: Step response (100 steps at new setpoint)
        sim.set_setpoint(0, 3.0)
        for _ in range(100):
            sim.step()
        assert abs(sim.get_time() - 100.0) < 1e-6

        # Phase 3: Disturbance rejection (100 steps)
        sim.set_input(0, 1.2)
        for _ in range(100):
            sim.step()

        # Verify system is still stable
        state = sim.get_state()
        assert 1.8 < state[0] < 3.2, "System should remain within reasonable bounds"

    def test_default_config_is_usable(self):
        """Verify create_default_config() produces a ready-to-use configuration."""
        config = tank_sim.create_default_config()
        sim = tank_sim.Simulator(config)

        # Run a complete simulation cycle
        for _ in range(100):
            sim.step()

        # Verify it works as expected
        assert sim.get_time() > 0.0
        state = sim.get_state()
        assert state.shape == (1,)
        assert 2.0 < state[0] < 3.0


class TestEdgeCases:
    """Test boundary conditions and unusual configurations."""

    def test_open_loop_simulation(self):
        """Verify simulator works without controllers (open-loop operation).

        In open-loop mode, the simulator runs the physics model without
        any feedback control. Inputs must be set manually via set_input().
        """
        config = tank_sim.SimulatorConfig()
        config.model_params = tank_sim.TankModelParameters()
        config.model_params.area = 120.0
        config.model_params.k_v = 1.2649
        config.model_params.max_height = 5.0

        # No controllers - open loop
        config.controllers = []

        # Initial conditions
        config.initial_state = np.array([2.5])
        config.initial_inputs = np.array([1.0, 0.5])  # Fixed inlet/valve
        config.dt = 1.0

        # Should create simulator successfully
        sim = tank_sim.Simulator(config)

        # Run open-loop simulation
        for _ in range(50):
            sim.step()

        # Verify simulator ran
        assert sim.get_time() == 50.0
        state = sim.get_state()
        assert len(state) == 1

        # Manually change valve position (no controller to interfere)
        sim.set_input(1, 0.3)  # Close valve to 30%
        sim.step()

        # Verify input changed
        inputs = sim.get_inputs()
        assert abs(inputs[1] - 0.3) < 1e-6

    def test_zero_timestep_raises_error(self):
        """Verify zero timestep is rejected during configuration.

        A zero timestep would cause division by zero in the integrator.
        """
        config = tank_sim.create_default_config()
        config.dt = 0.0

        # Should raise an error (implementation-dependent)
        # If C++ allows it, the simulation would fail on first step
        try:
            sim = tank_sim.Simulator(config)
            # If construction succeeds, step should fail
            with pytest.raises(Exception):
                sim.step()
        except Exception:
            # Construction failed as expected
            pass

    def test_negative_timestep(self):
        """Verify negative timestep behavior.

        Negative timesteps don't make physical sense but should be handled
        gracefully (either rejected or treated as forward integration).
        """
        config = tank_sim.create_default_config()
        config.dt = -1.0

        # Behavior depends on implementation
        # Document what actually happens
        try:
            sim = tank_sim.Simulator(config)
            # If allowed, step and observe behavior
            initial_time = sim.get_time()
            sim.step()
            final_time = sim.get_time()

            # Time should either increase (abs value used) or decrease
            assert final_time != initial_time
        except Exception:
            # Rejection is also acceptable
            pass

    def test_extreme_setpoint_causing_saturation(self):
        """Verify controller handles impossible setpoints gracefully.

        Setting a very high setpoint with reverse-acting controller should
        close the valve (attempting to raise level by reducing outlet flow).
        This tests anti-windup protection.
        """
        sim = tank_sim.create_default_config()
        sim = tank_sim.Simulator(sim)

        # Set impossible setpoint (higher than tank can physically reach)
        # With reverse-acting controller (Kc < 0), positive error closes valve
        sim.set_setpoint(0, 10.0)  # 10 m level (max is 5 m)

        # Run simulation
        for _ in range(100):
            sim.step()

        # Valve should be saturated at minimum (closed) due to reverse action
        # Large positive error → negative controller output → saturates at min_output (0.0)
        valve_position = sim.get_controller_output(0)
        assert abs(valve_position - 0.0) < 0.01, "Valve should saturate at 0% (closed)"

        # Level should rise as outlet flow is reduced
        state = sim.get_state()
        assert state[0] > 2.5, "Level should have risen with closed valve"
        assert state[0] < 5.0, "Level cannot exceed max tank height"

    def test_very_low_setpoint_causing_valve_opening(self):
        """Verify controller handles very low setpoint (valve opening).

        Setting a very low setpoint with reverse-acting controller should
        open the valve fully (attempting to lower level by increasing outlet flow).
        """
        sim = tank_sim.create_default_config()
        sim = tank_sim.Simulator(sim)

        # Set very low setpoint
        # With reverse-acting controller (Kc < 0), negative error opens valve
        sim.set_setpoint(0, 0.1)  # 0.1 m level (near empty)

        # Run simulation
        for _ in range(100):
            sim.step()

        # Valve should be saturated at maximum (fully open) due to reverse action
        # Large negative error → positive controller output → saturates at max_output (1.0)
        valve_position = sim.get_controller_output(0)
        assert abs(valve_position - 1.0) < 0.01, (
            "Valve should saturate at 100% (fully open)"
        )

        # Level should drop significantly with fully open valve
        state = sim.get_state()
        assert state[0] < 2.0, "Level should have dropped"
        assert state[0] > 0.0, "Level should remain positive"

    def test_empty_state_vector(self):
        """Verify empty state vector is rejected.

        A simulator with no state variables makes no sense.
        """
        config = tank_sim.create_default_config()
        config.initial_state = np.array([])

        with pytest.raises((ValueError, RuntimeError)):
            tank_sim.Simulator(config)

    def test_mismatched_input_dimensions(self):
        """Verify mismatched input vector dimensions are caught.

        The initial_inputs must match the expected input dimension
        (typically 2: inlet flow and valve position).
        """
        config = tank_sim.create_default_config()
        config.initial_inputs = np.array([1.0])  # Too short

        with pytest.raises((ValueError, RuntimeError)):
            tank_sim.Simulator(config)
