from typing import cast

from pydantic import BaseModel, Field, ValidationError, ValidationInfo, field_validator


class SimulationState(BaseModel):
    """
    Model representing a simulation state snapshot.
    """

    time: float = Field(..., description="Simulation time in seconds")
    tank_level: float = Field(..., ge=0.0, description="Current tank level in meters")
    setpoint: float = Field(..., description="Level setpoint in meters")
    inlet_flow: float = Field(
        ..., ge=0.0, description="Inlet flow rate in cubic meters per second"
    )
    outlet_flow: float = Field(
        ..., ge=0.0, description="Outlet flow rate in cubic meters per second"
    )
    valve_position: float = Field(
        ..., ge=0.0, le=1.0, description="Valve position from 0 to 1"
    )
    error: float = Field(..., description="Control error: setpoint minus level")
    controller_output: float = Field(
        ..., ge=0.0, le=1.0, description="PID controller output, 0 to 1"
    )


class SetpointCommand(BaseModel):
    """
    Model for setpoint change commands.
    """

    value: float = Field(..., ge=0.0, le=5.0, description="New setpoint in meters")


class PIDTuningCommand(BaseModel):
    """
    Model for PID tuning commands.
    """

    Kc: float = Field(
        ..., ge=0.0, description="Proportional gain, must be non-negative"
    )
    tau_I: float = Field(
        ...,
        ge=0.0,
        description="Integral time constant in seconds, 0 means no integral action",
    )
    tau_D: float = Field(
        ...,
        ge=0.0,
        description="Derivative time constant in seconds, 0 means no derivative action",
    )


class InletFlowCommand(BaseModel):
    """
    Model for inlet flow commands.
    """

    value: float = Field(
        ..., ge=0.0, le=2.0, description="New inlet flow in cubic meters per second"
    )


class InletModeCommand(BaseModel):
    """
    Model for inlet mode commands.
    """

    mode: str = Field(..., description="Inlet mode (either 'constant' or 'brownian')")
    min_flow: float = Field(
        0.8, ge=0.0, le=2.0, description="Minimum flow for Brownian mode, default 0.8"
    )
    max_flow: float = Field(
        1.2, ge=0.0, le=2.0, description="Maximum flow for Brownian mode, default 1.2"
    )

    @field_validator("mode")
    @classmethod
    def validate_mode(cls, v: str) -> str:
        if v not in ["constant", "brownian"]:
            raise ValueError("Mode must be either 'constant' or 'brownian'")
        return v

    @field_validator("max_flow")
    @classmethod
    def validate_min_max_flow(cls, v: float, info: ValidationInfo) -> float:
        min_flow = cast(float, info.data.get("min_flow", 0.8))
        if v <= min_flow:
            raise ValueError("max_flow must be greater than min_flow")
        return v


class ConfigResponse(BaseModel):
    """
    Model for configuration response.
    """

    tank_height: float = Field(..., description="Maximum tank height in meters")
    tank_area: float = Field(
        ..., ge=0.0, description="Cross-sectional area in square meters"
    )
    valve_coefficient: float = Field(..., ge=0.0, description="Valve k_v parameter")
    initial_level: float = Field(..., ge=0.0, description="Starting level in meters")
    initial_setpoint: float = Field(..., description="Starting setpoint in meters")
    pid_gains: PIDTuningCommand = Field(..., description="PID gains (Kc, tau_I, tau_D)")
    timestep: float = Field(..., gt=0.0, description="Simulation time step in seconds")


class HistoryQueryParams(BaseModel):
    """
    Model for history query parameters.
    """

    duration: int = Field(
        3600,
        ge=1,
        le=7200,
        description="Seconds of history to return (default 3600, max 7200)",
    )


# Example usage
try:
    state = SimulationState(
        time=10.0,
        tank_level=3.0,
        setpoint=4.0,
        inlet_flow=1.5,
        outlet_flow=0.5,
        valve_position=0.75,
        error=0.25,
        controller_output=0.6,
    )
    print(state)
except ValidationError as e:
    print(e)  # human-readable error message
    print(e.errors())  # structured error list
