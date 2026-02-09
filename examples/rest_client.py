#!/usr/bin/env python3
"""
REST API Client Example - Tank Dynamics Simulator

This script demonstrates REST API usage with the Tank Dynamics API.
It shows how to call each endpoint and handle responses.

Usage:
    python examples/rest_client.py

Requirements:
    - requests library (install with: pip install requests)
    - API server running on localhost:8000
"""

import json
import logging
import time

import requests

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s - %(levelname)s - %(message)s",
)
logger = logging.getLogger(__name__)

# API base URL
API_URL = "http://localhost:8000"


def print_json(data, title=""):
    """Pretty-print JSON data."""
    if title:
        logger.info(f"\n{title}")
    print(json.dumps(data, indent=2))


def check_health():
    """Check API health status."""
    logger.info("1. Health Check")
    try:
        response = requests.get(f"{API_URL}/api/health")
        response.raise_for_status()
        logger.info(f"Status: {response.json()['status']}")
        return True
    except requests.exceptions.RequestException as e:
        logger.error(f"Health check failed: {e}")
        return False


def get_config():
    """Retrieve simulation configuration."""
    logger.info("\n2. Get Configuration")
    try:
        response = requests.get(f"{API_URL}/api/config")
        response.raise_for_status()
        config = response.json()

        logger.info("Configuration retrieved:")
        logger.info(f"  Tank height: {config['tank_height']} m")
        logger.info(f"  Tank area: {config['tank_area']} m²")
        logger.info(f"  Valve k_v: {config['valve_coefficient']}")
        logger.info(f"  Initial level: {config['initial_level']} m")
        logger.info(f"  Initial setpoint: {config['initial_setpoint']} m")
        logger.info(f"  PID Gains:")
        logger.info(f"    Kc: {config['pid_gains']['Kc']}")
        logger.info(f"    tau_I: {config['pid_gains']['tau_I']} s")
        logger.info(f"    tau_D: {config['pid_gains']['tau_D']} s")
        logger.info(f"  History capacity: {config['history_capacity']} entries")
        logger.info(f"  Current history size: {config['history_size']} entries")

        return config
    except requests.exceptions.RequestException as e:
        logger.error(f"Failed to get config: {e}")
        return None


def get_state():
    """Retrieve current simulation state."""
    logger.info("\n3. Get Current State")
    try:
        response = requests.get(f"{API_URL}/api/state")
        response.raise_for_status()
        state = response.json()

        logger.info("Current simulation state:")
        logger.info(f"  Time: {state['time']:.1f} s")
        logger.info(f"  Tank level: {state['tank_level']:.3f} m")
        logger.info(f"  Setpoint: {state['setpoint']:.3f} m")
        logger.info(f"  Error: {state['error']:.3f} m")
        logger.info(f"  Inlet flow: {state['inlet_flow']:.3f} m³/s")
        logger.info(f"  Outlet flow: {state['outlet_flow']:.3f} m³/s")
        logger.info(f"  Valve position: {state['valve_position']:.3f}")
        logger.info(f"  Controller output: {state['controller_output']:.3f}")

        return state
    except requests.exceptions.RequestException as e:
        logger.error(f"Failed to get state: {e}")
        return None


def set_setpoint(value):
    """Change the controller setpoint."""
    logger.info(f"\n4. Set Setpoint to {value} m")
    try:
        response = requests.post(f"{API_URL}/api/setpoint", json={"value": value})
        response.raise_for_status()
        logger.info(f"Response: {response.json()['message']}")
        return True
    except requests.exceptions.RequestException as e:
        logger.error(f"Failed to set setpoint: {e}")
        return False


def set_pid_gains(Kc, tau_I, tau_D):
    """Update PID controller gains."""
    logger.info(f"\n5. Update PID Gains")
    logger.info(f"  Kc: {Kc}, tau_I: {tau_I}, tau_D: {tau_D}")
    try:
        response = requests.post(
            f"{API_URL}/api/pid", json={"Kc": Kc, "tau_I": tau_I, "tau_D": tau_D}
        )
        response.raise_for_status()
        result = response.json()
        logger.info(f"Response: {result['message']}")
        return True
    except requests.exceptions.RequestException as e:
        logger.error(f"Failed to update PID gains: {e}")
        return False


def set_inlet_flow(value):
    """Set inlet flow rate."""
    logger.info(f"\n6. Set Inlet Flow to {value} m³/s")
    try:
        response = requests.post(f"{API_URL}/api/inlet_flow", json={"value": value})
        response.raise_for_status()
        logger.info(f"Response: {response.json()['message']}")
        return True
    except requests.exceptions.RequestException as e:
        logger.error(f"Failed to set inlet flow: {e}")
        return False


def set_inlet_mode(mode, min_flow=0.8, max_flow=1.2, variance=0.05):
    """Switch inlet mode between constant and brownian."""
    logger.info(f"\n7. Set Inlet Mode to '{mode}'")
    if mode == "brownian":
        logger.info(
            f"  Min flow: {min_flow}, Max flow: {max_flow}, Variance: {variance}"
        )
    try:
        response = requests.post(
            f"{API_URL}/api/inlet_mode",
            json={"mode": mode, "min": min_flow, "max": max_flow, "variance": variance},
        )
        response.raise_for_status()
        result = response.json()
        logger.info(f"Response: {result['message']}")
        return True
    except requests.exceptions.RequestException as e:
        logger.error(f"Failed to set inlet mode: {e}")
        return False


def get_history(duration=300):
    """Retrieve historical data points."""
    logger.info(f"\n8. Get History (last {duration} seconds)")
    try:
        response = requests.get(f"{API_URL}/api/history", params={"duration": duration})
        response.raise_for_status()
        history = response.json()

        logger.info(f"Retrieved {len(history)} data points:")

        if history:
            # Show first, middle, and last points
            sample_indices = [0, len(history) // 2, -1]
            for i in sample_indices:
                h = history[i]
                logger.info(
                    f"  [{h['time']:7.1f}s] Level: {h['tank_level']:5.2f}m, "
                    f"Setpoint: {h['setpoint']:5.2f}m, Error: {h['error']:6.2f}m"
                )

            # Calculate statistics
            levels = [h["tank_level"] for h in history]
            avg_level = sum(levels) / len(levels)
            min_level = min(levels)
            max_level = max(levels)

            logger.info(f"Level statistics:")
            logger.info(f"  Average: {avg_level:.3f} m")
            logger.info(f"  Min: {min_level:.3f} m")
            logger.info(f"  Max: {max_level:.3f} m")
            logger.info(f"  Range: {max_level - min_level:.3f} m")

        return history
    except requests.exceptions.RequestException as e:
        logger.error(f"Failed to get history: {e}")
        return None


def reset_simulation():
    """Reset simulation to initial conditions."""
    logger.info("\n9. Reset Simulation")
    try:
        response = requests.post(f"{API_URL}/api/reset")
        response.raise_for_status()
        logger.info(f"Response: {response.json()['message']}")
        return True
    except requests.exceptions.RequestException as e:
        logger.error(f"Failed to reset: {e}")
        return False


def main():
    """Run all API examples."""
    logger.info("=" * 70)
    logger.info("Tank Dynamics Simulator - REST API Client Example")
    logger.info("=" * 70)

    # Check health first
    if not check_health():
        logger.error("API server is not available. Is it running?")
        logger.error("Start it with: python -m api.main")
        return

    # Get initial config
    config = get_config()
    if not config:
        return

    # Get initial state
    get_state()

    # Demonstrate control operations
    logger.info("\n" + "=" * 70)
    logger.info("Demonstrating Control Operations")
    logger.info("=" * 70)

    # Set a new setpoint
    if set_setpoint(3.5):
        logger.info("Waiting 10 seconds for controller to respond...")
        time.sleep(10)
        get_state()

    # Update PID gains
    if set_pid_gains(Kc=1.5, tau_I=8.0, tau_D=2.0):
        logger.info("Waiting 5 seconds...")
        time.sleep(5)
        get_state()

    # Change inlet flow
    if set_inlet_flow(1.2):
        logger.info("Waiting 5 seconds...")
        time.sleep(5)
        get_state()

    # Switch to Brownian inlet mode
    if set_inlet_mode("brownian", min_flow=0.8, max_flow=1.2, variance=0.05):
        logger.info("Waiting 10 seconds with Brownian disturbance...")
        time.sleep(10)
        get_state()

    # Switch back to constant
    if set_inlet_mode("constant"):
        logger.info("Waiting 5 seconds...")
        time.sleep(5)
        get_state()

    # Get history from last 5 minutes
    get_history(duration=300)

    # Reset for next test
    logger.info("\n" + "=" * 70)
    logger.info("Resetting Simulation")
    logger.info("=" * 70)
    reset_simulation()
    logger.info("Waiting 2 seconds after reset...")
    time.sleep(2)
    get_state()

    logger.info("\n" + "=" * 70)
    logger.info("API Examples Complete")
    logger.info("=" * 70)


if __name__ == "__main__":
    """Run REST API examples."""
    try:
        main()
    except KeyboardInterrupt:
        logger.info("\nInterrupted by user")
    except Exception as e:
        logger.error(f"Fatal error: {e}")
        exit(1)
