"""Type stubs for the C++ extension module."""

import numpy as np
import numpy.typing as npt

class PIDGains:
    Kc: float
    tau_I: float
    tau_D: float

class ControllerConfig:
    gains: PIDGains
    bias: float
    min_output: float
    max_output: float
    max_integral: float
    measured_index: int
    output_index: int
    initial_setpoint: float

class TankModelParameters:
    area: float
    k_v: float
    max_height: float

class SimulatorConfig:
    model_params: TankModelParameters
    controllers: list[ControllerConfig]
    dt: float
    initial_state: npt.NDArray[np.float64]
    initial_inputs: npt.NDArray[np.float64]

class Simulator:
    def __init__(self, config: SimulatorConfig) -> None: ...
    def step(self) -> None: ...
    def reset(self) -> None: ...
    def get_state(self) -> npt.NDArray[np.float64]: ...
    def get_inputs(self) -> npt.NDArray[np.float64]: ...
    def get_time(self) -> float: ...
    def get_setpoint(self, index: int) -> float: ...
    def get_error(self, index: int) -> float: ...
    def get_controller_output(self, index: int) -> float: ...
    def set_setpoint(self, index: int, value: float) -> None: ...
    def set_input(self, index: int, value: float) -> None: ...
    def set_controller_gains(self, index: int, gains: PIDGains) -> None: ...

def get_version() -> str: ...
