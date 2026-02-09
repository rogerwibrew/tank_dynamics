import asyncio
import logging
from collections import deque
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
        self.history: deque = deque(maxlen=7200)  # 2 hours at 1 Hz

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
            state = self.simulator.get_state()
            tank_level = state[0]
            setpoint = self.simulator.get_setpoint(0)  # 0 is controller index
            inlet_flow = self.simulator.get_inputs()[0]
            valve_position = self.simulator.get_inputs()[1]
            error = self.simulator.get_error(0)  # 0 is controller index
            controller_output = self.simulator.get_controller_output(0)
            time = self.simulator.get_time()

            # Calculate outlet flow using valve equation: q_out = k_v * valve_position * sqrt(tank_level)
            k_v = self.config.model_params.k_v
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
        """Reset simulation to initial conditions and clear history buffer."""
        if self.simulator is None or not self.initialized:
            logger.warning("reset called but simulator not initialized")
            return

        try:
            self.simulator.reset()
            self.history.clear()
            logger.info("Simulation reset to initial conditions and history cleared")
        except Exception as e:
            logger.error(f"Error resetting simulation: {e}")

    def set_setpoint(self, value: float):
        """Set the controller setpoint."""
        if self.simulator is None or not self.initialized:
            logger.warning("set_setpoint called but simulator not initialized")
            return

        try:
            self.simulator.set_setpoint(0, value)  # 0 is controller index
            logger.info(f"Setpoint set to {value}")
        except Exception as e:
            logger.error(f"Error setting setpoint: {e}")

    def set_pid_gains(self, gains: tank_sim.PIDGains):
        """Set PID controller gains."""
        if self.simulator is None or not self.initialized:
            logger.warning("set_pid_gains called but simulator not initialized")
            return

        try:
            self.simulator.set_controller_gains(0, gains)  # 0 is controller index
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
            self.simulator.set_input(0, value)  # 0 is inlet flow input index
            logger.info(f"Inlet flow set to {value}")
        except Exception as e:
            logger.error(f"Error setting inlet flow: {e}")

    def set_inlet_mode(
        self, mode: str, min_flow: float, max_flow: float, variance: float = 0.05
    ):
        """Set inlet flow mode (constant or brownian)."""
        if self.simulator is None or not self.initialized:
            logger.warning("set_inlet_mode called but simulator not initialized")
            return

        try:
            # Store mode parameters for Brownian implementation
            self.inlet_mode = mode
            self.inlet_mode_params = {
                "min": min_flow,
                "max": max_flow,
                "variance": variance,
            }
            logger.info(
                f"Inlet mode set to {mode} (min={min_flow}, max={max_flow}, variance={variance})"
            )
        except Exception as e:
            logger.error(f"Error setting inlet mode: {e}")

    def get_history(self, duration: int = 3600) -> list[dict[str, Any]]:
        """
        Get historical data points.

        Args:
            duration: Number of seconds of history to return (1-7200, default 3600)

        Returns:
            List of state snapshots in chronological order (oldest first)
        """
        if duration < 1 or duration > 7200:
            logger.warning(f"Invalid duration {duration}, clamping to valid range")
            duration = max(1, min(duration, 7200))

        num_entries = min(duration, len(self.history))
        if num_entries == 0:
            return []

        return list(self.history)[-num_entries:]

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

    async def simulation_loop(self):
        """
        Main simulation loop running at 1 Hz.

        Continuously:
        - Steps the simulation
        - Gets current state
        - Broadcasts state to all connected WebSocket clients
        - Stores state in history buffer
        """
        logger.info("Simulation loop started")
        try:
            while True:
                await asyncio.sleep(1.0)

                try:
                    # Advance simulation by one step
                    self.step()

                    # Get current state
                    state = self.get_state()

                    # Store in history buffer
                    self.history.append(state)

                    # Broadcast to all connected clients
                    message = {"type": "state", "data": state}
                    await self.broadcast(message)

                except Exception as e:
                    logger.error(f"Error in simulation loop iteration: {e}")
                    # Continue loop without crashing

        except asyncio.CancelledError:
            logger.info("Simulation loop cancelled")
            raise
        except Exception as e:
            logger.error(f"Fatal error in simulation loop: {e}")
            raise
