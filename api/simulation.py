import logging
from typing import Any

import tank_sim

logger = logging.getLogger(__name__)


class SimulationManager:
    """
    Singleton manager for the tank simulator.
    Manages simulation state and provides interface for API to interact with the simulator.
    """

    _instance: "SimulationManager | None" = None

    def __new__(cls, config: tank_sim.SimulatorConfig):
        if cls._instance is None:
            cls._instance = super(SimulationManager, cls).__new__(cls)
        return cls._instance

    def __init__(self, config: tank_sim.SimulatorConfig):
        self.config: tank_sim.SimulatorConfig = config
        self.initialized: bool = False
        self.simulator: tank_sim.Simulator | None = None
        self.connections: set = set()

    def initialize(self):
        """Initialize the simulator with the configuration."""
        try:
            self.simulator = tank_sim.Simulator(self.config)
            self.initialized = True
            logger.info("SimulationManager initialized successfully")
        except Exception as e:
            logger.error(f"Failed to initialize simulator: {e}")
            raise

    def get_state(self) -> dict[str, Any]:
        """Get current simulation state snapshot."""
        if self.simulator is None or not self.initialized:
            logger.warning("get_state called but simulator not initialized")
            return {
                "time": 0.0,
                "tank_level": 0.0,
                "setpoint": 0.0,
                "inlet_flow": 0.0,
                "outlet_flow": 0.0,
                "valve_position": 0.0,
                "error": 0.0,
                "controller_output": 0.0,
            }

        try:
            state = self.simulator.getState()
            tank_level = state[0]
            setpoint = self.simulator.getSetpoint()
            inlet_flow = self.simulator.getInputs()[0]
            valve_position = self.simulator.getInputs()[1]
            error = self.simulator.getError()
            controller_output = self.simulator.getInputs()[
                1
            ]  # valve position is controller output
            time = self.simulator.getTime()

            # Calculate outlet flow using valve equation: q_out = k_v * valve_position * sqrt(tank_level)
            k_v = self.config.valve_coefficient
            outlet_flow = (
                k_v * valve_position * (tank_level**0.5) if tank_level > 0 else 0.0
            )

            return {
                "time": float(time),
                "tank_level": float(tank_level),
                "setpoint": float(setpoint),
                "inlet_flow": float(inlet_flow),
                "outlet_flow": float(outlet_flow),
                "valve_position": float(valve_position),
                "error": float(error),
                "controller_output": float(controller_output),
            }
        except Exception as e:
            logger.error(f"Error getting state: {e}")
            return {
                "time": 0.0,
                "tank_level": 0.0,
                "setpoint": 0.0,
                "inlet_flow": 0.0,
                "outlet_flow": 0.0,
                "valve_position": 0.0,
                "error": 0.0,
                "controller_output": 0.0,
            }

    def step(self):
        """Advance simulation by one time step."""
        if self.simulator is None or not self.initialized:
            logger.warning("step called but simulator not initialized")
            return

        try:
            self.simulator.step()
        except Exception as e:
            logger.error(f"Error during simulation step: {e}")

    def reset(self):
        """Reset simulation to initial conditions."""
        if self.simulator is None or not self.initialized:
            logger.warning("reset called but simulator not initialized")
            return

        try:
            self.simulator.reset()
            logger.info("Simulation reset to initial conditions")
        except Exception as e:
            logger.error(f"Error resetting simulation: {e}")

    def set_setpoint(self, value: float):
        """Set the controller setpoint."""
        if self.simulator is None or not self.initialized:
            logger.warning("set_setpoint called but simulator not initialized")
            return

        try:
            self.simulator.setSetpoint(0, value)  # 0 is controller index
            logger.info(f"Setpoint set to {value}")
        except Exception as e:
            logger.error(f"Error setting setpoint: {e}")

    def set_pid_gains(self, gains: tank_sim.PIDGains):
        """Set PID controller gains."""
        if self.simulator is None or not self.initialized:
            logger.warning("set_pid_gains called but simulator not initialized")
            return

        try:
            self.simulator.setControllerGains(0, gains)  # 0 is controller index
            logger.info(
                f"PID gains set: Kc={gains.Kc}, tau_I={gains.tau_I}, tau_D={gains.tau_D}"
            )
        except Exception as e:
            logger.error(f"Error setting PID gains: {e}")

    def set_inlet_flow(self, value: float):
        """Set inlet flow rate."""
        if self.simulator is None or not self.initialized:
            logger.warning("set_inlet_flow called but simulator not initialized")
            return

        try:
            self.simulator.setInput(0, value)  # 0 is inlet flow input index
            logger.info(f"Inlet flow set to {value}")
        except Exception as e:
            logger.error(f"Error setting inlet flow: {e}")

    def set_inlet_mode(self, mode: str, min_flow: float, max_flow: float):
        """Set inlet flow mode (constant or brownian)."""
        if self.simulator is None or not self.initialized:
            logger.warning("set_inlet_mode called but simulator not initialized")
            return

        try:
            # Store mode parameters for future Brownian implementation
            self.inlet_mode = mode
            self.inlet_min_flow = min_flow
            self.inlet_max_flow = max_flow
            logger.info(f"Inlet mode set to {mode}")
        except Exception as e:
            logger.error(f"Error setting inlet mode: {e}")

    def get_history(self, duration: int) -> list[dict[str, Any]]:
        """Get historical data points."""
        # For now, return empty list (will implement ring buffer in next task)
        return []

    def add_connection(self, websocket):
        """Add a WebSocket connection."""
        self.connections.add(websocket)
        logger.info(
            f"WebSocket connection added. Total connections: {len(self.connections)}"
        )

    def remove_connection(self, websocket):
        """Remove a WebSocket connection."""
        self.connections.discard(websocket)
        logger.info(
            f"WebSocket connection removed. Total connections: {len(self.connections)}"
        )

    async def broadcast(self, message: dict[str, Any]):
        """Broadcast message to all connected clients."""
        disconnected = []
        for connection in self.connections:
            try:
                await connection.send_json(message)
            except Exception as e:
                logger.warning(f"Error sending message to client: {e}")
                disconnected.append(connection)

        # Remove failed connections
        for connection in disconnected:
            self.remove_connection(connection)
