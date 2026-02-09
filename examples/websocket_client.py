#!/usr/bin/env python3
"""
WebSocket Client Example - Tank Dynamics Simulator

This script demonstrates WebSocket usage with the Tank Dynamics API.
It connects to the real-time stream, receives state updates, and sends control commands.

Usage:
    python examples/websocket_client.py

Requirements:
    - websockets library (install with: pip install websockets)
    - API server running on localhost:8000
"""

import asyncio
import json
import logging
from datetime import datetime

import websockets

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s",
)
logger = logging.getLogger(__name__)


async def main():
    """Connect to WebSocket endpoint and demonstrate command/state interaction."""

    uri = "ws://localhost:8000/ws"
    logger.info(f"Connecting to {uri}...")

    try:
        async with websockets.connect(uri) as websocket:
            logger.info("Connected to server!")

            # Track received state updates
            state_count = 0
            start_time = datetime.now()

            # Create a task to send commands after delays
            async def send_commands():
                """Send control commands at specific times."""

                # Wait 5 seconds then change setpoint
                await asyncio.sleep(5)
                logger.info("Sending setpoint command: 3.5m")
                await websocket.send(json.dumps({"type": "setpoint", "value": 3.5}))

                # Wait another 5 seconds (10 total) and update PID gains
                await asyncio.sleep(5)
                logger.info("Sending PID tuning command")
                await websocket.send(
                    json.dumps({"type": "pid", "Kc": 1.5, "tau_I": 8.0, "tau_D": 2.0})
                )

                # Wait another 5 seconds (15 total) and switch to Brownian inlet mode
                await asyncio.sleep(5)
                logger.info("Switching to Brownian inlet mode")
                await websocket.send(
                    json.dumps(
                        {
                            "type": "inlet_mode",
                            "mode": "brownian",
                            "min": 0.8,
                            "max": 1.2,
                            "variance": 0.05,
                        }
                    )
                )

                # Wait another 5 seconds (20 total) and switch back to constant
                await asyncio.sleep(5)
                logger.info("Switching back to constant inlet mode")
                await websocket.send(
                    json.dumps(
                        {
                            "type": "inlet_mode",
                            "mode": "constant",
                            "min": 0.8,
                            "max": 1.2,
                        }
                    )
                )

            # Create a task to receive state updates
            async def receive_states():
                """Receive and display state updates from server."""
                nonlocal state_count

                try:
                    async for message in websocket:
                        data = json.loads(message)

                        # Handle state updates
                        if data["type"] == "state":
                            state_count += 1
                            state_data = data["data"]

                            # Display state nicely
                            elapsed = (datetime.now() - start_time).total_seconds()
                            logger.info(
                                f"[{state_count:3d}] Time: {state_data['time']:7.1f}s | "
                                f"Level: {state_data['tank_level']:5.2f}m | "
                                f"Setpoint: {state_data['setpoint']:5.2f}m | "
                                f"Error: {state_data['error']:6.2f}m | "
                                f"Valve: {state_data['valve_position']:5.2f} | "
                                f"Inlet: {state_data['inlet_flow']:5.2f} m³/s"
                            )

                        # Handle error messages
                        elif data["type"] == "error":
                            logger.error(f"Server error: {data['message']}")

                        # Stop after 30 updates (30 seconds at 1 Hz)
                        if state_count >= 30:
                            logger.info("Received 30 state updates, closing connection")
                            break

                except asyncio.CancelledError:
                    pass
                except Exception as e:
                    logger.error(f"Error receiving message: {e}")

            # Run both command sending and state receiving concurrently
            try:
                await asyncio.gather(send_commands(), receive_states())
            except asyncio.CancelledError:
                pass

            logger.info(f"Disconnected. Received {state_count} state updates.")

    except ConnectionRefusedError:
        logger.error(
            f"Could not connect to {uri}. "
            "Is the API server running? (python -m api.main)"
        )
    except Exception as e:
        logger.error(f"WebSocket error: {e}")


if __name__ == "__main__":
    """Run the WebSocket client."""
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Interrupted by user")
    except Exception as e:
        logger.error(f"Fatal error: {e}")
        exit(1)
