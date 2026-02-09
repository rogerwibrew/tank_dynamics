import logging
from contextlib import asynccontextmanager

from fastapi import FastAPI, Query, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse

import tank_sim

from .models import (
    ConfigResponse,
    InletFlowCommand,
    InletModeCommand,
    PIDTuningCommand,
    SetpointCommand,
    SimulationState,
)
from .simulation import SimulationManager

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
)
logger = logging.getLogger(__name__)

# Global simulation manager instance
simulation_manager: SimulationManager | None = None


@asynccontextmanager
async def lifespan(app: FastAPI):
    """
    Manages application startup and shutdown.
    """
    # Startup
    global simulation_manager
    try:
        config = tank_sim.create_default_config()
        simulation_manager = SimulationManager(config)
        simulation_manager.initialize()
        logger.info("Application started successfully")
    except Exception as e:
        logger.error(f"Failed to initialize simulation manager: {e}")
        raise

    yield

    # Shutdown
    logger.info("Application shutting down")


# Create FastAPI application
app = FastAPI(
    title="Tank Dynamics Simulator API",
    description="Real-time tank level control simulation with PID control",
    version="0.1.0",
    lifespan=lifespan,
)

# Enable CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=[
        "http://localhost:3000",  # Next.js dev server
        "http://localhost:5173",  # Vite dev server
    ],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


# REST Endpoints
@app.get("/api/health")
async def health_check():
    """Health check endpoint for monitoring."""
    return {"status": "ok"}


@app.get("/api/config", response_model=ConfigResponse)
async def get_config():
    """Get current simulation configuration."""
    try:
        if simulation_manager is None or not simulation_manager.initialized:
            return JSONResponse(
                status_code=500, content={"error": "Simulation not initialized"}
            )

        config = simulation_manager.config
        return {
            "tank_height": config.tank_height,
            "tank_area": config.tank_area,
            "valve_coefficient": config.valve_coefficient,
            "initial_level": config.initial_level,
            "initial_setpoint": config.initial_setpoint,
            "pid_gains": {
                "Kc": config.pid_gains.Kc,
                "tau_I": config.pid_gains.tau_I,
                "tau_D": config.pid_gains.tau_D,
            },
            "timestep": config.timestep,
        }
    except Exception as e:
        logger.error(f"Error getting config: {e}")
        return JSONResponse(status_code=500, content={"error": str(e)})


@app.post("/api/reset")
async def reset_simulation():
    """Reset simulation to initial steady state."""
    try:
        if simulation_manager is None or not simulation_manager.initialized:
            return JSONResponse(
                status_code=500, content={"error": "Simulation not initialized"}
            )

        simulation_manager.reset()
        logger.info("Simulation reset")
        return {"message": "Simulation reset successfully"}
    except Exception as e:
        logger.error(f"Error resetting simulation: {e}")
        return JSONResponse(status_code=500, content={"error": str(e)})


@app.post("/api/setpoint")
async def set_setpoint(command: SetpointCommand):
    """Update the simulation setpoint."""
    try:
        if simulation_manager is None or not simulation_manager.initialized:
            return JSONResponse(
                status_code=500, content={"error": "Simulation not initialized"}
            )

        simulation_manager.set_setpoint(command.value)
        logger.info(f"Setpoint changed to {command.value}")
        return {"message": "Setpoint updated", "value": command.value}
    except Exception as e:
        logger.error(f"Error setting setpoint: {e}")
        return JSONResponse(status_code=500, content={"error": str(e)})


@app.post("/api/pid")
async def set_pid_gains(command: PIDTuningCommand):
    """Update PID controller gains."""
    try:
        if simulation_manager is None or not simulation_manager.initialized:
            return JSONResponse(
                status_code=500, content={"error": "Simulation not initialized"}
            )

        gains = tank_sim.PIDGains(
            Kc=command.Kc, tau_I=command.tau_I, tau_D=command.tau_D
        )
        simulation_manager.set_pid_gains(gains)
        logger.info(
            f"PID gains updated: Kc={command.Kc}, tau_I={command.tau_I}, tau_D={command.tau_D}"
        )
        return {
            "message": "PID gains updated",
            "gains": {"Kc": command.Kc, "tau_I": command.tau_I, "tau_D": command.tau_D},
        }
    except Exception as e:
        logger.error(f"Error setting PID gains: {e}")
        return JSONResponse(status_code=500, content={"error": str(e)})


@app.post("/api/inlet_flow")
async def set_inlet_flow(command: InletFlowCommand):
    """Update inlet flow rate."""
    try:
        if simulation_manager is None or not simulation_manager.initialized:
            return JSONResponse(
                status_code=500, content={"error": "Simulation not initialized"}
            )

        simulation_manager.set_inlet_flow(command.value)
        logger.info(f"Inlet flow changed to {command.value}")
        return {"message": "Inlet flow updated", "value": command.value}
    except Exception as e:
        logger.error(f"Error setting inlet flow: {e}")
        return JSONResponse(status_code=500, content={"error": str(e)})


@app.post("/api/inlet_mode")
async def set_inlet_mode(command: InletModeCommand):
    """Switch inlet between constant and Brownian modes."""
    try:
        if simulation_manager is None or not simulation_manager.initialized:
            return JSONResponse(
                status_code=500, content={"error": "Simulation not initialized"}
            )

        simulation_manager.set_inlet_mode(
            command.mode, command.min_flow, command.max_flow
        )
        logger.info(f"Inlet mode changed to {command.mode}")
        return {
            "message": "Inlet mode updated",
            "mode": command.mode,
            "min_flow": command.min_flow,
            "max_flow": command.max_flow,
        }
    except Exception as e:
        logger.error(f"Error setting inlet mode: {e}")
        return JSONResponse(status_code=500, content={"error": str(e)})


@app.get("/api/history")
async def get_history(duration: int = Query(3600, ge=1, le=7200)):
    """Get historical data points."""
    try:
        if simulation_manager is None or not simulation_manager.initialized:
            return JSONResponse(
                status_code=500, content={"error": "Simulation not initialized"}
            )

        # For now, return empty list (will implement ring buffer in next task)
        history = simulation_manager.get_history(duration)
        return history
    except Exception as e:
        logger.error(f"Error getting history: {e}")
        return JSONResponse(status_code=500, content={"error": str(e)})


# WebSocket endpoint
@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    """WebSocket endpoint for real-time state broadcasting."""
    await websocket.accept()
    logger.info("Client connected to WebSocket")

    try:
        simulation_manager.add_connection(websocket)

        while True:
            # Receive messages from client
            data = await websocket.receive_text()
            logger.debug(f"Received message: {data}")

            # Echo back the message (for testing)
            await websocket.send_text(f"Echo: {data}")

    except WebSocketDisconnect:
        logger.info("Client disconnected from WebSocket")
        if simulation_manager is not None:
            simulation_manager.remove_connection(websocket)
    except Exception as e:
        logger.error(f"WebSocket error: {e}")
        if simulation_manager is not None:
            simulation_manager.remove_connection(websocket)
