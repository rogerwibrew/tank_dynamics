"""
WebSocket protocol tests for the tank dynamics simulator.

Tests WebSocket connection lifecycle, message handling, and command processing.
"""

import asyncio

import httpx
import pytest
from starlette.testclient import TestClient


def test_websocket_connection(client):
    """Verify that a client can connect to /ws endpoint successfully."""
    with client.websocket_connect("/ws") as ws:
        # Connection successful if we reach here
        assert ws is not None


def test_websocket_receives_state_updates(client):
    """Connect to WebSocket and verify state update messages arrive periodically."""
    with client.websocket_connect("/ws") as ws:
        # Receive first state message
        data1 = ws.receive_json()
        assert data1["type"] == "state"
        assert "data" in data1
        assert "tank_level" in data1["data"]
        assert "setpoint" in data1["data"]
        assert "inlet_flow" in data1["data"]

        # Receive second state message to verify continuous streaming
        data2 = ws.receive_json()
        assert data2["type"] == "state"
        assert "data" in data2


def test_websocket_setpoint_command(client):
    """Connect to WebSocket, send a setpoint command, and verify acceptance."""
    with client.websocket_connect("/ws") as ws:
        # Receive initial state
        ws.receive_json()

        # Send setpoint command
        command = {"type": "setpoint", "value": 3.0}
        ws.send_json(command)

        # Should not receive error
        data = ws.receive_json()
        # If we get a state message, the command was accepted
        assert data["type"] in ["state", "error"]
        if data["type"] == "error":
            pytest.fail("Setpoint command was rejected")


def test_websocket_pid_command(client):
    """Send a PID gains command via WebSocket and verify acceptance."""
    with client.websocket_connect("/ws") as ws:
        # Receive initial state
        ws.receive_json()

        # Send PID command
        command = {"type": "pid", "Kc": 2.0, "tau_I": 120.0, "tau_D": 15.0}
        ws.send_json(command)

        # Should not receive error immediately
        data = ws.receive_json()
        assert data["type"] in ["state", "error"]
        if data["type"] == "error":
            pytest.fail("PID command was rejected")


def test_websocket_inlet_flow_command(client):
    """Send an inlet flow command via WebSocket and verify acceptance."""
    with client.websocket_connect("/ws") as ws:
        # Receive initial state
        ws.receive_json()

        # Send inlet flow command
        command = {"type": "inlet_flow", "value": 0.9}
        ws.send_json(command)

        # Should not receive error
        data = ws.receive_json()
        assert data["type"] in ["state", "error"]
        if data["type"] == "error":
            pytest.fail("Inlet flow command was rejected")


def test_websocket_invalid_json(client):
    """Send malformed JSON over WebSocket and verify error handling."""
    with client.websocket_connect("/ws") as ws:
        # Receive initial state
        ws.receive_json()

        # Send invalid JSON
        ws.send_text("{invalid json")

        # Should receive error message, not disconnect
        data = ws.receive_json()
        assert data["type"] == "error"
        assert "message" in data


def test_websocket_missing_fields(client):
    """Send a command with missing required fields and verify error."""
    with client.websocket_connect("/ws") as ws:
        # Receive initial state
        ws.receive_json()

        # Send setpoint command without value
        command = {"type": "setpoint"}
        ws.send_json(command)

        # Should receive error message
        data = ws.receive_json()
        assert data["type"] == "error"
        assert "message" in data


def test_websocket_invalid_command_type(client):
    """Send a message with unknown type and verify error response."""
    with client.websocket_connect("/ws") as ws:
        # Receive initial state
        ws.receive_json()

        # Send unknown command type
        command = {"type": "unknown_command", "value": 123}
        ws.send_json(command)

        # Should receive error message
        data = ws.receive_json()
        assert data["type"] == "error"
        assert "Unknown" in data.get("message", "")


def test_websocket_multiple_clients(client):
    """Open multiple WebSocket connections and verify all receive updates."""
    # Note: TestClient lifespan is shared, so we can create multiple connections
    with (
        client.websocket_connect("/ws") as ws1,
        client.websocket_connect("/ws") as ws2,
        client.websocket_connect("/ws") as ws3,
    ):
        # All clients should receive state updates
        for ws in [ws1, ws2, ws3]:
            data = ws.receive_json()
            assert data["type"] == "state"

        # Close one connection and verify others still receive updates
        ws1.close()

        # ws2 and ws3 should still receive updates
        for ws in [ws2, ws3]:
            data = ws.receive_json()
            assert data["type"] == "state"


def test_websocket_inlet_mode_command(client):
    """Send an inlet mode command via WebSocket and verify acceptance."""
    with client.websocket_connect("/ws") as ws:
        # Receive initial state
        ws.receive_json()

        # Send inlet_mode command for brownian
        command = {
            "type": "inlet_mode",
            "mode": "brownian",
            "min": 0.8,
            "max": 1.2,
            "variance": 0.05,
        }
        ws.send_json(command)

        # Should not receive error
        data = ws.receive_json()
        assert data["type"] in ["state", "error"]
        if data["type"] == "error":
            pytest.fail("Inlet mode command was rejected")
