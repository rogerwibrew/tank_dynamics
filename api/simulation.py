# api/simulation.py

import tank_sim


class SimulationManager:
    _instance: "SimulationManager | None" = None

    def __new__(cls, config: tank_sim.SimulatorConfig):
        if cls._instance is None:
            cls._instance = super(SimulationManager, cls).__new__(cls)
        return cls._instance

    def __init__(self, config: tank_sim.SimulatorConfig):
        self.config: tank_sim.SimulatorConfig = config
        self.initialized: bool = False
        self.simulator: tank_sim.Simulator | None = None
        self.setpoint: float | None = None
        self.PID: tank_sim.PIDGains | None = None
        self.inlet_flow: float | None = None

    def initialize(self):
        self.simulator = tank_sim.Simulator(self.config)
        self.initialized = True

    def get_state(self):
        # Return dummy data matching StateSnapshot model structure
        return {
            "current_level": 0.0,
            "target_level": 10.0,
            "flow_rate": 0.5,
            "error": 0.0,
        }

    def step(self):
        pass

    def reset(self):
        if self.simulator is not None:
            self.simulator.reset()


# Example usage in main.py would be:
# simulation_manager = SimulationManager(config)
# await simulation_manager.initialize()
# state = simulation_manager.get_state()
