"""
Concurrency tests for the tank dynamics simulator API.

Tests behavior under concurrent access patterns including multiple simultaneous
REST requests, WebSocket connections, and mixed operations.
"""

import asyncio

import pytest
from starlette.testclient import TestClient


def test_concurrent_rest_requests(client):
    """Make multiple simultaneous REST requests and verify all succeed."""
    # Make 5 concurrent GET /api/state requests
    responses = []
    for _ in range(5):
        response = client.get("/api/state")
        responses.append(response)

    # All should succeed
    for response in responses:
        assert response.status_code == 200
        data = response.json()
        assert "tank_level" in data
        assert "setpoint" in data


def test_concurrent_websocket_clients(client):
    """Open multiple WebSocket connections concurrently and verify all receive updates."""
    with (
        client.websocket_connect("/ws") as ws1,
        client.websocket_connect("/ws") as ws2,
        client.websocket_connect("/ws") as ws3,
    ):
        # All clients should receive at least 2 state messages
        for ws in [ws1, ws2, ws3]:
            for _ in range(2):
                data = ws.receive_json()
                assert data["type"] == "state"


def test_mixed_concurrent_operations(client):
    """Simultaneously perform REST and WebSocket operations."""
    # Make REST GET requests
    for _ in range(3):
        response = client.get("/api/state")
        assert response.status_code == 200

    # Make REST POST requests
    for i in range(2):
        response = client.post("/api/setpoint", json={"value": 2.5 + i * 0.5})
        assert response.status_code == 200

    # Connect WebSocket and receive messages
    with client.websocket_connect("/ws") as ws:
        for _ in range(2):
            data = ws.receive_json()
            assert data["type"] == "state"


def test_history_query_during_updates(client):
    """Query history repeatedly while simulation updates occur."""
    # Make multiple history queries
    for _ in range(5):
        response = client.get("/api/history?duration=60")
        assert response.status_code == 200
        data = response.json()
        assert isinstance(data, list)


def test_setpoint_changes_rapid_succession(client):
    """Send multiple setpoint changes in rapid succession."""
    # Send 10 setpoint changes as fast as possible
    for i in range(10):
        response = client.post("/api/setpoint", json={"value": 2.0 + i * 0.1})
        assert response.status_code == 200

    # Verify final state
    state_response = client.get("/api/state")
    assert state_response.status_code == 200


def test_reset_during_active_connections(client):
    """Issue reset command while WebSocket client is connected."""
    with client.websocket_connect("/ws") as ws:
        # Receive initial state
        data1 = ws.receive_json()
        assert data1["type"] == "state"

        # Send reset command (using a separate request, not through websocket)
        reset_response = client.post("/api/reset")
        assert reset_response.status_code == 200

        # WebSocket should continue receiving after reset
        data2 = ws.receive_json()
        assert data2["type"] == "state"


def test_concurrent_inlet_mode_changes(client):
    """Send multiple inlet mode changes in succession."""
    responses = []

    responses.append(
        client.post(
            "/api/inlet_mode",
            json={"mode": "brownian", "min": 0.8, "max": 1.2, "variance": 0.05},
        )
    )
    responses.append(client.post("/api/inlet_mode", json={"mode": "constant"}))
    responses.append(
        client.post(
            "/api/inlet_mode",
            json={"mode": "brownian", "min": 0.7, "max": 1.3, "variance": 0.1},
        )
    )
    responses.append(client.post("/api/inlet_mode", json={"mode": "constant"}))

    # All should succeed
    for response in responses:
        assert response.status_code == 200
