"""
Tank Dynamics Simulator
=======================

A real-time tank level control simulator with PID control.

This package provides Python bindings to a high-performance C++ simulation
engine that models:

- Tank material balance (ODE integration using GSL RK4)
- PID feedback control with anti-windup
- Valve dynamics and flow calculations

Basic usage::

    import tank_sim

    # Check version
    print(tank_sim.get_version())

    # Full API (available after Task 11):
    # config = tank_sim.create_default_config()
    # sim = tank_sim.Simulator(config)
    # sim.step()
    # print(sim.get_state())

"""

__version__ = "0.1.0"

# Import from the C++ extension module
import numpy as np

from ._tank_sim import (
    ControllerConfig,
    PIDGains,
    Simulator,
    SimulatorConfig,
    TankModelParameters,
    get_version,
)


def create_default_config():
    """Create a standard steady-state configuration for the tank simulator.

    This convenience function returns a pre-configured SimulatorConfig with
    typical values for a single tank with PID level control. Useful for
    getting started quickly without manually setting every parameter.

    The configuration represents:
    - Tank: 120 m² cross-section, 1.2649 m^2.5/s valve coefficient, 5 m max height
    - Initial state: 2.5 m level (50% of max)
    - Initial inputs: 1.0 m³/s inlet flow, 0.5 valve position
    - PID control: Reverse-acting (negative Kc for outlet valve control)
    - Tuning: Moderate response with anti-windup protection

    Returns:
        SimulatorConfig: Ready-to-use configuration at steady state.

    Example:
        >>> config = create_default_config()
        >>> sim = Simulator(config)
        >>> sim.step()
    """
    config = SimulatorConfig()

    # Tank physics parameters
    config.model_params = TankModelParameters()
    config.model_params.area = 120.0
    config.model_params.k_v = 1.2649
    config.model_params.max_height = 5.0

    # PID controller configuration
    controller = ControllerConfig()
    controller.gains = PIDGains()
    controller.gains.Kc = -1.0  # Reverse-acting for outlet valve
    controller.gains.tau_I = 10.0  # 10 second integral time
    controller.gains.tau_D = 1.0  # 1 second derivative time
    controller.bias = 0.5  # Nominal valve position at setpoint
    controller.min_output = 0.0  # Valve fully closed
    controller.max_output = 1.0  # Valve fully open
    controller.max_integral = 10.0  # Anti-windup limit
    controller.measured_index = 0  # Measure tank level (state 0)
    controller.output_index = 1  # Control valve position (input 1)
    controller.initial_setpoint = 2.5  # Target level: 2.5 m (50%)

    config.controllers = [controller]

    # Initial conditions at steady state
    # CRITICAL: These values satisfy the steady-state constraint where q_out = q_in
    # Outlet flow equation: q_out = k_v * valve_position * sqrt(level)
    # Verification: 1.0 = 1.2649 * 0.5 * sqrt(2.5) = 1.0 ✓
    # If modifying these parameters, ensure this equality still holds to maintain
    # steady state, otherwise the simulation will start with immediate transients.
    level = 2.5  # meters
    q_in = 1.0  # m³/s
    valve_position = 0.5  # 50% open

    config.initial_state = np.array([level])
    config.initial_inputs = np.array([q_in, valve_position])

    # Simulation timestep
    config.dt = 1.0  # 1 second per step

    return config


# Public API - functions and classes exported by this package
__all__ = [
    "get_version",
    "Simulator",
    "SimulatorConfig",
    "ControllerConfig",
    "TankModelParameters",
    "PIDGains",
    "create_default_config",
]
