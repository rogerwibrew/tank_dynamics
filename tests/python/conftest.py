"""Pytest configuration and fixtures for tank_sim tests."""

import numpy as np
import pytest

import tank_sim


@pytest.fixture
def default_config():
    """Standard steady-state configuration for testing.

    Returns a SimulatorConfig pre-configured with typical tank and controller
    parameters at steady state. This ensures consistent test conditions across
    all test functions.

    Returns:
        tank_sim.SimulatorConfig: Configuration with parameters:
            - Tank: area=120.0, k_v=1.2649, max_height=5.0
            - Initial state: [2.5] (2.5 m level)
            - Initial inputs: [1.0, 0.5] (inlet flow, valve position)
            - PID gains: Kc=-1.0, tau_I=10.0, tau_D=1.0
            - dt=1.0 second per step
    """
    return tank_sim.create_default_config()


@pytest.fixture
def steady_state_simulator(default_config):
    """Simulator instance initialized at steady state.

    Creates a fresh Simulator instance with the default configuration.
    This fixture is function-scoped, so each test gets its own simulator
    instance with clean initial conditions.

    Args:
        default_config: The default_config fixture providing SimulatorConfig.

    Returns:
        tank_sim.Simulator: Initialized simulator ready for testing.
    """
    return tank_sim.Simulator(default_config)
