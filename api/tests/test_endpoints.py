"""
REST API endpoint tests for the tank dynamics simulator.

Tests all HTTP endpoints including GET, POST, and error cases.
Uses mocked tank_sim to avoid C++ compilation dependency.
"""

import pytest


def test_health_endpoint(client):
    """Verify GET /health returns status 200 and contains 'status' field."""
    response = client.get("/api/health")
    assert response.status_code == 200
    data = response.json()
    assert "status" in data
    assert data["status"] in ["ok", "healthy"]


def test_get_config(client):
    """Verify GET /api/config returns configuration with all required fields."""
    response = client.get("/api/config")
    assert response.status_code == 200
    data = response.json()

    # Check all required fields
    assert "tank_height" in data
    assert "tank_area" in data
    assert "valve_coefficient" in data
    assert "initial_level" in data
    assert "initial_setpoint" in data
    assert "pid_gains" in data
    assert "timestep" in data
    assert "history_capacity" in data
    assert "history_size" in data

    # Verify PID gains structure
    assert "Kc" in data["pid_gains"]
    assert "tau_I" in data["pid_gains"]
    assert "tau_D" in data["pid_gains"]

    # Verify numeric values are reasonable
    assert data["tank_height"] > 0
    assert data["tank_area"] > 0
    assert data["valve_coefficient"] > 0
    assert data["timestep"] > 0
    assert data["history_capacity"] > 0


def test_get_history_default(client):
    """Verify GET /api/history without parameters returns recent history data."""
    response = client.get("/api/history")
    assert response.status_code == 200
    data = response.json()

    # Should be a list
    assert isinstance(data, list)

    # If there are entries, verify structure
    if len(data) > 0:
        entry = data[0]
        assert "time" in entry
        assert "tank_level" in entry
        assert "setpoint" in entry


def test_get_history_with_duration(client):
    """Verify GET /api/history?duration=60 returns history filtered to requested duration."""
    response = client.get("/api/history?duration=60")
    assert response.status_code == 200
    data = response.json()

    # Should be a list
    assert isinstance(data, list)


def test_get_history_validation(client):
    """Verify invalid duration values return appropriate validation errors."""
    # Test negative duration
    response = client.get("/api/history?duration=-1")
    assert response.status_code == 422

    # Test zero duration
    response = client.get("/api/history?duration=0")
    assert response.status_code == 422

    # Test non-numeric duration
    response = client.get("/api/history?duration=abc")
    assert response.status_code == 422

    # Test duration exceeding max
    response = client.get("/api/history?duration=10000")
    assert response.status_code == 422


def test_post_setpoint_valid(client):
    """Verify POST /api/setpoint with valid value succeeds."""
    payload = {"value": 3.5}
    response = client.post("/api/setpoint", json=payload)
    assert response.status_code == 200
    data = response.json()
    assert "message" in data
    assert data.get("value") == 3.5


def test_post_setpoint_out_of_range(client):
    """Verify POST /api/setpoint with out-of-range value returns validation error."""
    # Test above max (5.0)
    response = client.post("/api/setpoint", json={"value": 6.0})
    assert response.status_code == 422

    # Test below min (0.0)
    response = client.post("/api/setpoint", json={"value": -1.0})
    assert response.status_code == 422


def test_post_inlet_flow_valid(client):
    """Verify POST /api/inlet_flow with valid positive flow value succeeds."""
    payload = {"value": 0.8}
    response = client.post("/api/inlet_flow", json=payload)
    assert response.status_code == 200
    data = response.json()
    assert "message" in data
    assert data.get("value") == 0.8


def test_post_inlet_flow_negative(client):
    """Verify POST /api/inlet_flow with negative value returns validation error."""
    response = client.post("/api/inlet_flow", json={"value": -0.5})
    assert response.status_code == 422


def test_post_pid_gains_valid(client):
    """Verify POST /api/pid with valid PID parameters succeeds."""
    payload = {"Kc": 2.0, "tau_I": 120.0, "tau_D": 15.0}
    response = client.post("/api/pid", json=payload)
    assert response.status_code == 200
    data = response.json()
    assert "message" in data
    assert data["gains"]["Kc"] == 2.0
    assert data["gains"]["tau_I"] == 120.0
    assert data["gains"]["tau_D"] == 15.0


def test_post_pid_gains_invalid(client):
    """Verify POST /api/pid with invalid values returns validation error."""
    # Test negative tau_I
    response = client.post("/api/pid", json={"Kc": 1.5, "tau_I": -1.0, "tau_D": 10.0})
    assert response.status_code == 422

    # Test negative tau_D
    response = client.post("/api/pid", json={"Kc": 1.5, "tau_I": 100.0, "tau_D": -5.0})
    assert response.status_code == 422


def test_post_reset(client):
    """Verify POST /api/reset succeeds and returns confirmation."""
    response = client.post("/api/reset")
    assert response.status_code == 200
    data = response.json()
    assert "message" in data

    # After reset, verify we can still get config
    config_response = client.get("/api/config")
    assert config_response.status_code == 200


def test_404_unknown_endpoint(client):
    """Verify requests to non-existent endpoints return 404."""
    response = client.get("/api/nonexistent")
    assert response.status_code == 404


def test_json_parse_error(client):
    """Verify POST requests with malformed JSON return error."""
    # Send invalid JSON
    response = client.post(
        "/api/setpoint",
        content="{invalid json}",
        headers={"content-type": "application/json"},
    )
    assert response.status_code in [400, 422]


def test_inlet_mode_constant(client):
    """Verify POST /api/inlet_mode can set constant mode."""
    payload = {"mode": "constant"}
    response = client.post("/api/inlet_mode", json=payload)
    assert response.status_code == 200
    data = response.json()
    assert data["mode"] == "constant"


def test_inlet_mode_brownian(client):
    """Verify POST /api/inlet_mode can set brownian mode with parameters."""
    payload = {"mode": "brownian", "min": 0.8, "max": 1.2, "variance": 0.05}
    response = client.post("/api/inlet_mode", json=payload)
    assert response.status_code == 200
    data = response.json()
    assert data["mode"] == "brownian"
    assert data["min"] == 0.8
    assert data["max"] == 1.2
    assert data["variance"] == 0.05


def test_post_setpoint_missing_value(client):
    """Verify POST /api/setpoint without value returns validation error."""
    response = client.post("/api/setpoint", json={})
    assert response.status_code == 422


def test_post_inlet_flow_missing_value(client):
    """Verify POST /api/inlet_flow without value returns validation error."""
    response = client.post("/api/inlet_flow", json={})
    assert response.status_code == 422


def test_post_pid_gains_missing_fields(client):
    """Verify POST /api/pid with missing fields returns validation error."""
    response = client.post("/api/pid", json={"Kc": 1.5})
    assert response.status_code == 422
