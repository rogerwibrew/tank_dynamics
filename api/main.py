import asyncio
import json
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

# Track the background simulation loop task
simulation_task: asyncio.Task | None = None


@asynccontextmanager
async def lifespan(app: FastAPI):
    """
    Manages application startup and shutdown.
    """
    # Startup
    global simulation_manager, simulation_task
    try:
        config = tank_sim.create_default_config()
        simulation_manager = SimulationManager(config)
        simulation_manager.initialize()
        logger.info("Application started successfully")

        # Start the simulation loop as a background task
        simulation_task = asyncio.create_task(simulation_manager.simulation_loop())
        logger.info("Simulation loop started")

    except Exception as e:
        logger.error(f"Failed to initialize simulation manager: {e}")
        raise

    yield

    # Shutdown
    if simulation_task is not None:
        simulation_task.cancel()
        try:
            await simulation_task
        except asyncio.CancelledError:
            pass
        logger.info("Simulation loop stopped")

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
        model_params = config.model_params
        controller = config.controllers[0]
        gains = controller.gains

        return {
            "tank_height": model_params.max_height,
            "tank_area": model_params.area,
            "valve_coefficient": model_params.k_v,
            "initial_level": config.initial_state[0],
            "initial_setpoint": controller.initial_setpoint,
            "pid_gains": {
                "Kc": gains.Kc,
                "tau_I": gains.tau_I,
                "tau_D": gains.tau_D,
            },
            "timestep": config.dt,
            "history_capacity": 7200,
            "history_size": len(simulation_manager.history),
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

        history = simulation_manager.get_history(duration)
        return history
    except Exception as e:
        logger.error(f"Error getting history: {e}")
        return JSONResponse(status_code=500, content={"error": str(e)})


# WebSocket endpoint
@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    """
    WebSocket endpoint for real-time state broadcasting and command handling.

    Sends:
    - State updates: {"type": "state", "data": {...}}
    - Error messages: {"type": "error", "message": "..."}

    Receives:
    - {"type": "setpoint", "value": <float>}
    - {"type": "pid", "Kc": <float>, "tau_I": <float>, "tau_D": <float>}
    - {"type": "inlet_flow", "value": <float>}
    - {"type": "inlet_mode", "mode": <str>, "min_flow": <float>, "max_flow": <float>}
    """
    await websocket.accept()
    logger.info("Client connected to WebSocket")

    try:
        if simulation_manager is None or not simulation_manager.initialized:
            await websocket.send_json(
                {"type": "error", "message": "Simulation not initialized"}
            )
            await websocket.close()
            return

        simulation_manager.add_connection(websocket)

        while True:
            # Receive JSON messages from client
            try:
                data = await websocket.receive_text()
                message = json.loads(data)
            except json.JSONDecodeError:
                await websocket.send_json(
                    {"type": "error", "message": "Invalid JSON format"}
                )
                continue
            except Exception as e:
                logger.error(f"Error receiving message: {e}")
                break

            # Route message based on type
            try:
                msg_type = message.get("type")

                if msg_type == "setpoint":
                    value = message.get("value")
                    if value is None:
                        await websocket.send_json(
                            {"type": "error", "message": "Missing 'value' field"}
                        )
                    else:
                        simulation_manager.set_setpoint(float(value))
                        logger.info(f"Setpoint command: {value}")

                elif msg_type == "pid":
                    kc = message.get("Kc")
                    tau_i = message.get("tau_I")
                    tau_d = message.get("tau_D")
                    if any(x is None for x in [kc, tau_i, tau_d]):
                        await websocket.send_json(
                            {
                                "type": "error",
                                "message": "Missing PID gain fields (Kc, tau_I, tau_D)",
                            }
                        )
                    else:
                        gains = tank_sim.PIDGains(
                            Kc=float(kc), tau_I=float(tau_i), tau_D=float(tau_d)
                        )
                        simulation_manager.set_pid_gains(gains)
                        logger.info(
                            f"PID command: Kc={kc}, tau_I={tau_i}, tau_D={tau_d}"
                        )

                elif msg_type == "inlet_flow":
                    value = message.get("value")
                    if value is None:
                        await websocket.send_json(
                            {"type": "error", "message": "Missing 'value' field"}
                        )
                    else:
                        simulation_manager.set_inlet_flow(float(value))
                        logger.info(f"Inlet flow command: {value}")

                elif msg_type == "inlet_mode":
                    mode = message.get("mode")
                    min_flow = message.get("min")
                    max_flow = message.get("max")
                    if any(x is None for x in [mode, min_flow, max_flow]):
                        await websocket.send_json(
                            {
                                "type": "error",
                                "message": "Missing inlet mode fields (mode, min, max)",
                            }
                        )
                    else:
                        simulation_manager.set_inlet_mode(
                            str(mode), float(min_flow), float(max_flow)
                        )
                        logger.info(f"Inlet mode command: {mode}")

                else:
                    await websocket.send_json(
                        {
                            "type": "error",
                            "message": f"Unknown message type: {msg_type}",
                        }
                    )

            except (ValueError, TypeError) as e:
                logger.error(f"Error parsing message values: {e}")
                await websocket.send_json(
                    {"type": "error", "message": f"Invalid message format: {e}"}
                )
            except Exception as e:
                logger.error(f"Error processing message: {e}")
                await websocket.send_json(
                    {"type": "error", "message": f"Error processing command: {e}"}
                )

    except WebSocketDisconnect:
        logger.info("Client disconnected from WebSocket")
        if simulation_manager is not None:
            simulation_manager.remove_connection(websocket)
    except Exception as e:
        logger.error(f"WebSocket error: {e}")
        if simulation_manager is not None:
            simulation_manager.remove_connection(websocket)
