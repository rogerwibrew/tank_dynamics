"""
Pytest configuration and fixtures for API tests.

This module provides shared fixtures for testing the FastAPI backend,
including mocked tank_sim module and FastAPI test client setup.
"""

import sys
from unittest.mock import MagicMock

import httpx
import pytest
from starlette.testclient import TestClient


# Mock Simulator class that will be used by all tests
class MockSimulator:
    def __init__(self, config):
        self.config = config
        self.state = [2.5]  # tank_level
        self.setpoint = [2.5]  # per controller
        self.inputs = [1.0, 0.5]  # inlet_flow, valve_position
        self.error = [0.0]  # per controller
        self.controller_output = [0.5]  # per controller
        self.time = 0.0
        self.step_count = 0

    def step(self):
        """Simulate one step forward."""
        self.time += 1.0
        self.step_count += 1
        # Simple simulation: inlet - outlet
        outlet = 0.15 * self.inputs[1] * (self.state[0] ** 0.5)
        net_flow = self.inputs[0] - outlet
        self.state[0] = max(0, self.state[0] + net_flow * 1.0)
        self.error[0] = self.setpoint[0] - self.state[0]

    def get_state(self):
        """Get tank level."""
        return self.state

    def get_setpoint(self, controller_idx):
        """Get controller setpoint."""
        return self.setpoint[controller_idx]

    def get_inputs(self):
        """Get input values."""
        return self.inputs

    def get_error(self, controller_idx):
        """Get control error."""
        return self.error[controller_idx]

    def get_controller_output(self, controller_idx):
        """Get controller output."""
        return self.controller_output[controller_idx]

    def get_time(self):
        """Get simulation time."""
        return self.time

    def set_setpoint(self, controller_idx, value):
        """Set controller setpoint."""
        self.setpoint[controller_idx] = value

    def set_controller_gains(self, controller_idx, gains):
        """Set PID gains."""
        pass

    def set_input(self, input_idx, value):
        """Set input value."""
        self.inputs[input_idx] = value

    def reset(self):
        """Reset to initial conditions."""
        self.state = [2.5]
        self.setpoint = [2.5]
        self.inputs = [1.0, 0.5]
        self.error = [0.0]
        self.controller_output = [0.5]
        self.time = 0.0
        self.step_count = 0


# Install mock BEFORE any imports - this runs at module import time
if "tank_sim" not in sys.modules:
    mock_module = MagicMock()

    # Mock SimulatorConfig
    mock_config = MagicMock()
    mock_config.model_params = MagicMock()
    mock_config.model_params.max_height = 5.0
    mock_config.model_params.area = 1.0
    mock_config.model_params.k_v = 0.15
    mock_config.controllers = [MagicMock()]
    mock_config.controllers[0].gains = MagicMock()
    mock_config.controllers[0].gains.Kc = 1.5
    mock_config.controllers[0].gains.tau_I = 100.0
    mock_config.controllers[0].gains.tau_D = 10.0
    mock_config.controllers[0].initial_setpoint = 2.5
    mock_config.initial_state = [2.5]
    mock_config.dt = 1.0

    mock_module.SimulatorConfig = MagicMock(return_value=mock_config)
    mock_module.create_default_config = MagicMock(return_value=mock_config)
    mock_module.Simulator = MockSimulator

    # Mock PIDGains
    def create_pid_gains(Kc, tau_I, tau_D):
        gains = MagicMock()
        gains.Kc = Kc
        gains.tau_I = tau_I
        gains.tau_D = tau_D
        return gains

    mock_module.PIDGains = create_pid_gains

    # Install mock in sys.modules
    sys.modules["tank_sim"] = mock_module


# Keep the fixture for compatibility but it's now just a reference
@pytest.fixture(scope="session", autouse=True)
def mock_tank_sim():
    """
    Mock the tank_sim C++ module to allow testing without compilation.

    The actual mock is installed at module import time above.
    This fixture just provides a reference for tests that need it.
    """
    yield sys.modules.get("tank_sim")


@pytest.fixture
def app():
    """
    Fixture that returns the FastAPI application instance.

    This creates a fresh app for each test to ensure isolation.
    """
    # Import after mock_tank_sim fixture ensures mock is in place
    from api.main import app as fastapi_app
    from api.simulation import SimulationManager

    # Reset the SimulationManager singleton for test isolation
    SimulationManager._instance = None

    return fastapi_app


@pytest.fixture
def client(app):
    """
    Fixture that returns a synchronous test client for testing.

    This uses Starlette's TestClient which works synchronously with FastAPI.
    For async tests, use this client with asyncio patterns.

    The TestClient automatically handles the lifespan context manager.
    """
    with TestClient(app) as test_client:
        yield test_client


@pytest.fixture
async def async_client(app):
    """
    Fixture that returns an async httpx client for WebSocket testing.

    This uses httpx.AsyncClient which can handle WebSocket connections.
    """
    async with httpx.AsyncClient(app=app, base_url="http://test") as client:
        yield client


@pytest.fixture
def simulation_state():
    """
    Fixture providing a typical simulation state dictionary.

    Tests can override specific fields as needed.
    """
    return {
        "time": 10.0,
        "tank_level": 2.5,
        "setpoint": 3.0,
        "inlet_flow": 1.0,
        "outlet_flow": 0.5,
        "valve_position": 0.5,
        "error": 0.5,
        "controller_output": 0.6,
    }


@pytest.fixture
def default_config():
    """
    Fixture providing the default simulation configuration.

    Matches what GET /api/config endpoint should return.
    """
    return {
        "tank_height": 5.0,
        "tank_area": 1.0,
        "valve_coefficient": 0.15,
        "initial_level": 2.5,
        "initial_setpoint": 2.5,
        "pid_gains": {
            "Kc": 1.5,
            "tau_I": 100.0,
            "tau_D": 10.0,
        },
        "timestep": 1.0,
        "history_capacity": 7200,
    }
