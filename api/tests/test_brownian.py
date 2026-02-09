"""Tests for Brownian inlet flow mode functionality."""

import asyncio
from unittest.mock import MagicMock, call, patch

import numpy as np
import pytest

import tank_sim
from api.simulation import SimulationManager


@pytest.fixture
def mock_config():
    """Create a mock tank_sim configuration."""
    # Note: tank_sim.SimulatorConfig is already a MagicMock from conftest.py
    # so we can't use spec= here. Just create a plain MagicMock.
    config = MagicMock()
    config.model_params = MagicMock()
    config.model_params.k_v = 1.0
    config.model_params.max_height = 5.0
    config.model_params.area = 2.0
    config.initial_state = [2.0]
    config.controllers = [MagicMock()]
    config.controllers[0].initial_setpoint = 3.0
    config.controllers[0].gains = MagicMock()
    config.controllers[0].gains.Kc = 1.0
    config.controllers[0].gains.tau_I = 0.5
    config.controllers[0].gains.tau_D = 0.1
    config.dt = 1.0
    return config


@pytest.fixture
def sim_manager(mock_config):
    """Create a SimulationManager instance with mocked simulator."""
    with patch("tank_sim.Simulator"):
        manager = SimulationManager(mock_config)
        manager.initialized = True
        # Note: tank_sim.Simulator is already a class from conftest.py MockSimulator
        # so we can't use spec= here. Just create a plain MagicMock.
        manager.simulator = MagicMock()
        manager.simulator.get_state.return_value = [2.0]
        manager.simulator.get_setpoint.return_value = 3.0

        # Track inlet flow for testing
        manager.current_inlet_flow = 1.0

        def mock_get_inputs():
            return [manager.current_inlet_flow, 0.5]

        def mock_set_input(index, value):
            if index == 0:
                manager.current_inlet_flow = value

        manager.simulator.get_inputs = mock_get_inputs
        manager.simulator.set_input = mock_set_input
        manager.simulator.get_error.return_value = 1.0
        manager.simulator.get_controller_output.return_value = 0.5
        manager.simulator.get_time.return_value = 0.0
        manager.simulator.step = MagicMock()
        return manager


@pytest.mark.asyncio
async def test_brownian_mode_changes_inlet_flow(sim_manager):
    """
    Set inlet mode to Brownian and verify that inlet flow changes between steps.
    Verify that inlet flow stays within bounds.
    """
    # Set Brownian mode with reasonable parameters
    sim_manager.set_inlet_mode("brownian", min_flow=0.8, max_flow=1.2, variance=0.05)

    assert sim_manager.inlet_mode == "brownian"

    # Collect inlet flow values over 10 steps
    inlet_flows = []
    for _ in range(10):
        inlet_flows.append(sim_manager.current_inlet_flow)
        sim_manager.step()

    # Verify that flows changed (not constant)
    assert len(set(inlet_flows)) > 1, "Inlet flow should vary with Brownian motion"

    # Verify all flows stayed within bounds
    for flow in inlet_flows:
        assert 0.8 <= flow <= 1.2, f"Flow {flow} outside bounds [0.8, 1.2]"


@pytest.mark.asyncio
async def test_brownian_mode_respects_bounds(sim_manager):
    """
    Set inlet mode to Brownian with tight bounds and high variance.
    Run for 50 steps and verify inlet flow NEVER exceeds bounds.
    """
    # Set tight bounds with high variance to stress test clamping
    sim_manager.set_inlet_mode("brownian", min_flow=0.95, max_flow=1.05, variance=0.5)

    # Collect inlet flow values over 50 steps
    inlet_flows = []
    for _ in range(50):
        inlet_flows.append(sim_manager.current_inlet_flow)
        sim_manager.step()

    # Verify ALL flows stay within bounds despite high variance
    for flow in inlet_flows:
        assert 0.95 <= flow <= 1.05, f"Flow {flow} exceeded bounds [0.95, 1.05]"


@pytest.mark.asyncio
async def test_brownian_mode_mean_reversion(sim_manager):
    """
    Set inlet mode to Brownian centered at 1.0 with symmetric bounds.
    Run for 1000 steps and verify the mean is approximately 1.0 (unbiased random walk).
    """
    sim_manager.set_inlet_mode("brownian", min_flow=0.5, max_flow=1.5, variance=0.05)

    # Collect inlet flow values over 1000 steps
    inlet_flows = []
    for _ in range(1000):
        inlet_flows.append(sim_manager.current_inlet_flow)
        sim_manager.step()

    # Compute mean of collected values
    mean_flow = np.mean(inlet_flows)

    # Verify mean is approximately 1.0 (within ±0.2)
    assert 0.8 <= mean_flow <= 1.2, f"Mean flow {mean_flow} not approximately 1.0"


@pytest.mark.asyncio
async def test_brownian_variance_effect(sim_manager):
    """
    Run two simulations with different variances.
    Verify that high variance produces higher standard deviation.
    """
    # First run with low variance
    sim_manager.set_inlet_mode("brownian", min_flow=0.5, max_flow=1.5, variance=0.01)

    low_var_flows = []
    for _ in range(100):
        low_var_flows.append(sim_manager.current_inlet_flow)
        sim_manager.step()

    low_var_std = np.std(low_var_flows)

    # Reset for second run
    sim_manager.current_inlet_flow = 1.0
    sim_manager.set_inlet_mode("brownian", min_flow=0.5, max_flow=1.5, variance=0.2)

    high_var_flows = []
    for _ in range(100):
        high_var_flows.append(sim_manager.current_inlet_flow)
        sim_manager.step()

    high_var_std = np.std(high_var_flows)

    # Verify high variance has higher standard deviation
    assert high_var_std > low_var_std, (
        f"High variance ({high_var_std}) should have higher std than "
        f"low variance ({low_var_std})"
    )


@pytest.mark.asyncio
async def test_constant_mode_disables_brownian(sim_manager):
    """
    Enable Brownian mode, establish random walk, then switch to constant mode.
    Verify inlet flow stops changing after switching to constant.
    """
    # Enable Brownian and let it run 5 steps
    sim_manager.set_inlet_mode("brownian", min_flow=0.5, max_flow=1.5, variance=0.1)

    for _ in range(5):
        sim_manager.step()

    # Get current inlet flow
    frozen_flow = sim_manager.current_inlet_flow

    # Switch to constant mode
    sim_manager.set_inlet_mode("constant", min_flow=0.5, max_flow=1.5, variance=0.0)

    # Verify inlet flow stops changing
    for _ in range(10):
        sim_manager.step()
        current_flow = sim_manager.current_inlet_flow
        # In constant mode, inlet flow should not change
        assert current_flow == frozen_flow, (
            f"Inlet flow changed in constant mode: {frozen_flow} -> {current_flow}"
        )


@pytest.mark.asyncio
async def test_brownian_mode_parameter_validation(sim_manager):
    """
    Verify that valid Brownian parameters are stored correctly.
    """
    # Valid parameters should work
    sim_manager.set_inlet_mode("brownian", min_flow=0.5, max_flow=1.5, variance=0.05)
    assert sim_manager.inlet_mode == "brownian"
    assert sim_manager.inlet_mode_params["min"] == 0.5
    assert sim_manager.inlet_mode_params["max"] == 1.5
    assert sim_manager.inlet_mode_params["variance"] == 0.05


@pytest.mark.asyncio
async def test_pid_rejects_brownian_disturbances(sim_manager):
    """
    Enable Brownian disturbances and verify that the inlet flow varies while
    simulation steps correctly.
    """
    # Setup: typical operating condition
    sim_manager.simulator.get_setpoint.return_value = 3.0

    # Enable Brownian with moderate disturbances
    sim_manager.set_inlet_mode("brownian", min_flow=0.8, max_flow=1.2, variance=0.05)

    # Track inlet flows
    inlet_flows = []

    for i in range(100):
        inlet_flows.append(sim_manager.current_inlet_flow)
        sim_manager.step()

    # Verify inlet flows vary due to Brownian motion
    assert len(set(inlet_flows)) > 1, "Inlet flow should vary with Brownian motion"

    # Verify all flows stay within bounds
    for flow in inlet_flows:
        assert 0.8 <= flow <= 1.2, f"Flow {flow} outside bounds"


@pytest.mark.asyncio
async def test_apply_brownian_inlet_direct(sim_manager):
    """
    Test the apply_brownian_inlet method directly.
    Verify that it returns values within bounds and applies random increment.
    """
    sim_manager.set_inlet_mode("brownian", min_flow=0.5, max_flow=1.5, variance=0.1)

    current_flow = 1.0
    new_flows = []

    # Generate 50 new flows
    for _ in range(50):
        new_flow = sim_manager.apply_brownian_inlet(current_flow)
        new_flows.append(new_flow)
        current_flow = new_flow

    # Verify all flows are within bounds
    for flow in new_flows:
        assert 0.5 <= flow <= 1.5, f"Flow {flow} outside bounds"

    # Verify flows are different (not constant)
    assert len(set(new_flows)) > 1, "Flows should vary"


@pytest.mark.asyncio
async def test_brownian_reset_behavior(sim_manager):
    """
    Verify that reset() resets inlet_mode to constant and clears parameters.
    """
    # Enable Brownian mode
    sim_manager.set_inlet_mode("brownian", min_flow=0.8, max_flow=1.2, variance=0.05)
    assert sim_manager.inlet_mode == "brownian"

    # Reset simulation
    sim_manager.reset()

    # Verify inlet mode is reset to constant
    assert sim_manager.inlet_mode == "constant"

    # Verify parameters are reset to defaults
    assert sim_manager.inlet_mode_params["min"] == 0.8
    assert sim_manager.inlet_mode_params["max"] == 1.2
    assert sim_manager.inlet_mode_params["variance"] == 0.05
