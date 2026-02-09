# Tank Dynamics API Reference

## Overview

The Tank Dynamics API provides a real-time simulation interface for a PID-controlled tank level system. The API uses FastAPI with both REST and WebSocket endpoints, allowing clients to monitor simulation state and send control commands.

**Base URL:** `http://localhost:8000` (default)

**Current API Version:** 0.1.0

**Authentication:** None (suitable for trusted networks only; see deployment guide for production security)

**CORS Configuration:** Enabled for `localhost:3000` and `localhost:5173` by default (configurable in production)

## REST Endpoints

All REST endpoints use JSON request/response format. Responses include appropriate HTTP status codes for error conditions.

### Health Check: `GET /api/health`

Simple health check endpoint useful for monitoring and deployment health probes.

**Request:**
```bash
curl http://localhost:8000/api/health
```

**Success Response (200 OK):**
```json
{"status": "ok"}
```

**Use Case:** Container orchestration, load balancers, and monitoring systems can use this to verify API availability.

---

### Get Current State: `GET /api/state`

Retrieve a snapshot of the current simulation state.

**Request:**
```bash
curl http://localhost:8000/api/state
```

**Success Response (200 OK):**
```json
{
  "time": 1234.5,
  "tank_level": 2.85,
  "setpoint": 3.0,
  "inlet_flow": 1.0,
  "outlet_flow": 0.95,
  "valve_position": 0.48,
  "error": 0.15,
  "controller_output": 0.48
}
```

**Response Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `time` | float | Simulation elapsed time in seconds |
| `tank_level` | float | Current tank level in meters (0 - 5.0 m) |
| `setpoint` | float | Target tank level in meters (0 - 5.0 m) |
| `inlet_flow` | float | Inlet volumetric flow rate in m³/s |
| `outlet_flow` | float | Outlet volumetric flow rate in m³/s (calculated) |
| `valve_position` | float | Outlet valve position (0.0 = closed, 1.0 = fully open) |
| `error` | float | Control error: setpoint minus level (meters) |
| `controller_output` | float | Raw PID controller output (0.0 - 1.0) |

**Error Responses:**

- `500 Internal Server Error` - Simulation not initialized or internal error

```json
{"error": "Simulation not initialized"}
```

---

### Get Configuration: `GET /api/config`

Retrieve the current simulation configuration, including tank parameters, PID gains, and system properties.

**Request:**
```bash
curl http://localhost:8000/api/config
```

**Success Response (200 OK):**
```json
{
  "tank_height": 5.0,
  "tank_area": 120.0,
  "valve_coefficient": 1.2649,
  "initial_level": 2.5,
  "initial_setpoint": 2.5,
  "pid_gains": {
    "Kc": 1.0,
    "tau_I": 10.0,
    "tau_D": 5.0
  },
  "timestep": 1.0,
  "history_capacity": 7200,
  "history_size": 2450
}
```

**Response Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `tank_height` | float | Maximum tank height in meters |
| `tank_area` | float | Tank cross-sectional area in m² |
| `valve_coefficient` | float | Valve flow coefficient k_v (m^2.5/s) |
| `initial_level` | float | Initial tank level at startup (m) |
| `initial_setpoint` | float | Initial setpoint at startup (m) |
| `pid_gains` | object | Current PID controller gains |
| `timestep` | float | Simulation time step (1.0 second) |
| `history_capacity` | int | Maximum history buffer size (7200 entries = 2 hours) |
| `history_size` | int | Current number of entries in history buffer |

---

### Set Setpoint: `POST /api/setpoint`

Change the tank level setpoint (target level for PID controller).

**Request:**
```bash
curl -X POST http://localhost:8000/api/setpoint \
  -H "Content-Type: application/json" \
  -d '{"value": 3.5}'
```

**Request Body:**
```json
{
  "value": 3.5
}
```

**Constraints:**
- `value`: 0.0 ≤ value ≤ 5.0 (meters)

**Success Response (200 OK):**
```json
{
  "message": "Setpoint updated",
  "value": 3.5
}
```

**Error Responses:**

- `422 Unprocessable Entity` - Invalid input value

```json
{
  "detail": [
    {
      "type": "greater_than_equal",
      "loc": ["body", "value"],
      "msg": "Input should be greater than or equal to 0",
      "input": -1.0
    }
  ]
}
```

**Use Case:** Operators set a new target level. The PID controller will adjust the outlet valve to reach and maintain this level.

---

### Update PID Gains: `POST /api/pid`

Dynamically tune the PID controller gains. Changes are applied immediately with bumpless transfer (no controller output jumps).

**Request:**
```bash
curl -X POST http://localhost:8000/api/pid \
  -H "Content-Type: application/json" \
  -d '{"Kc": 1.5, "tau_I": 8.0, "tau_D": 2.0}'
```

**Request Body:**
```json
{
  "Kc": 1.5,
  "tau_I": 8.0,
  "tau_D": 2.0
}
```

**Field Constraints:**

| Field | Constraint | Description |
|-------|-----------|-------------|
| `Kc` | Unrestricted | Proportional gain; can be negative for reverse-acting control |
| `tau_I` | ≥ 0.0 | Integral time in seconds; 0 disables integral action |
| `tau_D` | ≥ 0.0 | Derivative time in seconds; 0 disables derivative action |

**Success Response (200 OK):**
```json
{
  "message": "PID gains updated",
  "gains": {
    "Kc": 1.5,
    "tau_I": 8.0,
    "tau_D": 2.0
  }
}
```

**Control Theory:**

The PID controller output is calculated as:

```
error = setpoint - actual_level
output = Kc * (error + (1/tau_I) * ∫error + tau_D * d(error)/dt)
valve_position = saturate(output, 0, 1)
```

- **Kc (Proportional Gain):** Increases response speed. Larger values = more aggressive.
- **tau_I (Integral Time):** Smaller values = faster offset correction. Prevents steady-state error.
- **tau_D (Derivative Time):** Larger values = more damping. Reduces overshoot and oscillations.

---

### Set Inlet Flow: `POST /api/inlet_flow`

Manually set the inlet flow rate. Only applicable when in "constant" inlet mode.

**Request:**
```bash
curl -X POST http://localhost:8000/api/inlet_flow \
  -H "Content-Type: application/json" \
  -d '{"value": 1.2}'
```

**Request Body:**
```json
{
  "value": 1.2
}
```

**Constraints:**
- `value`: 0.0 ≤ value ≤ 2.0 (m³/s)

**Success Response (200 OK):**
```json
{
  "message": "Inlet flow updated",
  "value": 1.2
}
```

**Note:** In "brownian" inlet mode, this endpoint will succeed but the inlet flow will be overridden by Brownian motion on the next simulation step.

---

### Set Inlet Mode: `POST /api/inlet_mode`

Switch between constant and Brownian (random walk) inlet flow modes.

**Request:**
```bash
curl -X POST http://localhost:8000/api/inlet_mode \
  -H "Content-Type: application/json" \
  -d '{
    "mode": "brownian",
    "min": 0.8,
    "max": 1.2,
    "variance": 0.05
  }'
```

**Request Body:**
```json
{
  "mode": "brownian",
  "min": 0.8,
  "max": 1.2,
  "variance": 0.05
}
```

**Field Constraints:**

| Field | Constraint | Default | Description |
|-------|-----------|---------|-------------|
| `mode` | "constant" or "brownian" | - | Inlet flow mode |
| `min` | 0.0 ≤ min ≤ 2.0 | 0.8 | Minimum flow (m³/s) for Brownian mode |
| `max` | min < max ≤ 2.0 | 1.2 | Maximum flow (m³/s) for Brownian mode |
| `variance` | 0.0 ≤ variance ≤ 1.0 | 0.05 | Step variance for Brownian motion |

**Success Response (200 OK):**
```json
{
  "message": "Inlet mode updated",
  "mode": "brownian",
  "min": 0.8,
  "max": 1.2,
  "variance": 0.05
}
```

**Inlet Modes:**

- **constant:** Fixed inlet flow. Use `/api/inlet_flow` to adjust.
- **brownian:** Random walk inlet flow. Simulates process disturbances. Each step, flow = current_flow + N(0, variance), clamped to [min, max].

---

### Reset Simulation: `POST /api/reset`

Reset the simulation to initial steady-state conditions and clear the history buffer.

**Request:**
```bash
curl -X POST http://localhost:8000/api/reset
```

**Success Response (200 OK):**
```json
{
  "message": "Simulation reset successfully"
}
```

**Effects:**
- Tank level → initial level (2.5 m)
- Setpoint → initial setpoint (2.5 m)
- Inlet mode → "constant"
- Inlet flow → 1.0 m³/s
- Valve position → recalculated for steady state
- History buffer → cleared
- Simulation time → reset to 0

**Use Case:** After testing disturbance responses or tuning experiments, reset to start fresh.

---

### Get History: `GET /api/history?duration=3600`

Retrieve historical state data from the ring buffer. Data points are recorded every 1 second.

**Request:**
```bash
curl 'http://localhost:8000/api/history?duration=3600'
```

**Query Parameters:**

| Parameter | Type | Constraint | Default | Description |
|-----------|------|-----------|---------|-------------|
| `duration` | int | 1 ≤ duration ≤ 7200 | 3600 | Seconds of history to return |

**Success Response (200 OK):**
```json
[
  {
    "time": 0.0,
    "tank_level": 2.5,
    "setpoint": 2.5,
    "inlet_flow": 1.0,
    "outlet_flow": 1.0,
    "valve_position": 0.5,
    "error": 0.0,
    "controller_output": 0.5
  },
  {
    "time": 1.0,
    "tank_level": 2.502,
    "setpoint": 2.5,
    "inlet_flow": 1.0,
    "outlet_flow": 0.999,
    "valve_position": 0.501,
    "error": -0.002,
    "controller_output": 0.499
  },
  {
    "time": 2.0,
    "tank_level": 2.504,
    "setpoint": 2.5,
    "inlet_flow": 1.0,
    "outlet_flow": 0.998,
    "valve_position": 0.502,
    "error": -0.004,
    "controller_output": 0.498
  }
]
```

**Notes:**
- Data is in chronological order (oldest first)
- Ring buffer holds up to 7200 entries (~2 hours at 1 Hz)
- If fewer points than requested exist, all available points are returned
- Each entry matches the `SimulationState` model

**Common Query Examples:**

```bash
# Last 1 hour (3600 seconds)
curl 'http://localhost:8000/api/history?duration=3600'

# Last 5 minutes (300 seconds)
curl 'http://localhost:8000/api/history?duration=300'

# Full buffer (2 hours, 7200 seconds)
curl 'http://localhost:8000/api/history?duration=7200'
```

---

## WebSocket Endpoint

The WebSocket endpoint provides real-time bidirectional communication for continuous state updates and command input.

### Connection: `WS /ws`

**URL:** `ws://localhost:8000/ws` (or `wss://...` for TLS)

**Features:**
- Server broadcasts state updates every 1 second
- Client can send control commands at any time
- Connection remains open until explicitly closed
- Automatic error messages sent for malformed commands

### Server → Client Messages

#### State Update (every 1 second)

```json
{
  "type": "state",
  "data": {
    "time": 1234.5,
    "tank_level": 2.85,
    "setpoint": 3.0,
    "inlet_flow": 1.0,
    "outlet_flow": 0.95,
    "valve_position": 0.48,
    "error": 0.15,
    "controller_output": 0.48
  }
}
```

#### Error Message

```json
{
  "type": "error",
  "message": "Invalid JSON format"
}
```

**Error Messages:**
- "Invalid JSON format" - Client sent non-JSON data
- "Missing 'value' field" - Command lacks required field
- "Unknown message type: ..." - Unrecognized command type
- "Invalid message format: ..." - Value parsing failed
- "Error processing command: ..." - Command execution failed

### Client → Server Commands

#### Setpoint Command

```json
{
  "type": "setpoint",
  "value": 3.5
}
```

**Constraints:** 0.0 ≤ value ≤ 5.0 (meters)

#### PID Gains Command

```json
{
  "type": "pid",
  "Kc": 1.5,
  "tau_I": 8.0,
  "tau_D": 2.0
}
```

**Constraints:**
- `Kc`: Unrestricted (any float)
- `tau_I`: ≥ 0.0
- `tau_D`: ≥ 0.0

#### Inlet Flow Command

```json
{
  "type": "inlet_flow",
  "value": 1.2
}
```

**Constraints:** 0.0 ≤ value ≤ 2.0 (m³/s)

#### Inlet Mode Command

```json
{
  "type": "inlet_mode",
  "mode": "brownian",
  "min": 0.8,
  "max": 1.2,
  "variance": 0.05
}
```

**Constraints:**
- `mode`: "constant" or "brownian"
- `min`: 0.0 ≤ min ≤ 2.0
- `max`: min < max ≤ 2.0
- `variance`: 0.0 ≤ variance ≤ 1.0 (optional, default 0.05)

### Connection Lifecycle Example

```
Client                          Server
  |-------- CONNECT -------->|  (WebSocket handshake)
  |<----- STATE UPDATE -----| ({"type": "state", ...})
  |<----- STATE UPDATE -----| ({"type": "state", ...})
  |------- SETPOINT ------->| ({"type": "setpoint", "value": 3.5})
  |<----- STATE UPDATE -----| ({"type": "state", ...})
  |------- PID GAINS ------->| ({"type": "pid", ...})
  |<----- STATE UPDATE -----| ({"type": "state", ...})
  |------ DISCONNECT ------>|
```

### Python WebSocket Example

```python
import asyncio
import json
import websockets

async def connect():
    uri = "ws://localhost:8000/ws"
    async with websockets.connect(uri) as websocket:
        # Start a task to receive messages
        async def receive_messages():
            async for message in websocket:
                data = json.loads(message)
                print(f"Received: {data}")
        
        # Start a task to send commands
        async def send_commands():
            await asyncio.sleep(5)  # Wait 5 seconds
            await websocket.send(json.dumps({
                "type": "setpoint",
                "value": 3.5
            }))
        
        # Run both tasks concurrently
        await asyncio.gather(
            receive_messages(),
            send_commands()
        )

asyncio.run(connect())
```

### JavaScript/Browser Example

```javascript
const ws = new WebSocket('ws://localhost:8000/ws');

ws.onopen = () => {
  console.log('Connected');
};

ws.onmessage = (event) => {
  const message = JSON.parse(event.data);
  if (message.type === 'state') {
    console.log(`Level: ${message.data.tank_level.toFixed(2)}m`);
  }
};

ws.onerror = (error) => {
  console.error('WebSocket error:', error);
};

// Send a command
setTimeout(() => {
  ws.send(JSON.stringify({
    type: 'setpoint',
    value: 3.5
  }));
}, 5000);
```

---

## Data Models

### SimulationState

Complete snapshot of the simulation at a point in time.

```json
{
  "time": 1234.5,
  "tank_level": 2.85,
  "setpoint": 3.0,
  "inlet_flow": 1.0,
  "outlet_flow": 0.95,
  "valve_position": 0.48,
  "error": 0.15,
  "controller_output": 0.48
}
```

**Fields:**

| Field | Type | Range | Description |
|-------|------|-------|-------------|
| `time` | float | ≥ 0 | Elapsed simulation time (seconds) |
| `tank_level` | float | 0 - 5.0 | Tank liquid level (meters) |
| `setpoint` | float | 0 - 5.0 | Controller target level (meters) |
| `inlet_flow` | float | 0 - 2.0 | Inlet volumetric flow (m³/s) |
| `outlet_flow` | float | 0 - 2.0 | Outlet volumetric flow (m³/s) |
| `valve_position` | float | 0 - 1.0 | Outlet valve position (0=closed, 1=open) |
| `error` | float | -5.0 - 5.0 | Control error (setpoint - level) |
| `controller_output` | float | 0 - 1.0 | Raw PID output |

### SetpointCommand

```json
{
  "value": 3.5
}
```

### PIDTuningCommand

```json
{
  "Kc": 1.5,
  "tau_I": 8.0,
  "tau_D": 2.0
}
```

### InletFlowCommand

```json
{
  "value": 1.2
}
```

### InletModeCommand

```json
{
  "mode": "brownian",
  "min": 0.8,
  "max": 1.2,
  "variance": 0.05
}
```

### ConfigResponse

```json
{
  "tank_height": 5.0,
  "tank_area": 120.0,
  "valve_coefficient": 1.2649,
  "initial_level": 2.5,
  "initial_setpoint": 2.5,
  "pid_gains": {
    "Kc": 1.0,
    "tau_I": 10.0,
    "tau_D": 5.0
  },
  "timestep": 1.0,
  "history_capacity": 7200,
  "history_size": 2450
}
```

---

## Error Handling

### HTTP Error Responses

All REST endpoints follow standard HTTP status codes:

| Status | Meaning | Example |
|--------|---------|---------|
| 200 | Success | Command executed successfully |
| 422 | Unprocessable Entity | Invalid input values (validation error) |
| 500 | Internal Server Error | Simulation not initialized or runtime error |

### Validation Error Format

When a request contains invalid values (e.g., setpoint > 5.0), the API returns:

```json
{
  "detail": [
    {
      "type": "less_than_equal",
      "loc": ["body", "value"],
      "msg": "Input should be less than or equal to 5",
      "input": 6.0
    }
  ]
}
```

### WebSocket Error Handling

Errors on WebSocket are sent as JSON messages with type "error":

```json
{
  "type": "error",
  "message": "Invalid JSON format"
}
```

The connection is NOT closed on error; the client can continue sending valid commands.

---

## Rate Limits

**Current Status:** No rate limiting is implemented.

The API accepts requests at any rate. For production deployments with multiple clients, consider implementing rate limiting at the reverse proxy level (e.g., nginx) to prevent resource exhaustion.

---

## API Conventions

### Timestamps

All times are elapsed seconds from simulation start, not wall-clock times.

### Units

- **Length:** Meters (m)
- **Flow:** Cubic meters per second (m³/s)
- **Time:** Seconds (s)
- **Area:** Square meters (m²)

### Floating-Point Precision

All numeric values are IEEE 754 double precision. JSON serialization may round to 15-16 significant digits.

### JSON Keys

Command and response JSON uses snake_case for REST endpoints and PascalCase for PID gains (matching C++ naming):

- REST: `tank_level`, `inlet_flow`, `valve_position`
- PID: `Kc`, `tau_I`, `tau_D`

---

## Swagger/OpenAPI Documentation

Interactive API documentation is available at:

```
http://localhost:8000/docs
```

This provides a live interface to test all endpoints directly from the browser.

Alternative (ReDoc) documentation:

```
http://localhost:8000/redoc
```

---

## Common Usage Patterns

### Monitoring Real-Time Process

Use WebSocket endpoint:

```python
import asyncio, json, websockets

async def monitor():
    async with websockets.connect('ws://localhost:8000/ws') as ws:
        for _ in range(60):  # Monitor for 1 minute
            msg = json.loads(await ws.recv())
            if msg['type'] == 'state':
                level = msg['data']['tank_level']
                setpoint = msg['data']['setpoint']
                print(f"Level: {level:.2f}m, Setpoint: {setpoint:.2f}m")

asyncio.run(monitor())
```

### Retrieving Test Data for Analysis

Use REST history endpoint:

```python
import requests

response = requests.get(
    'http://localhost:8000/api/history',
    params={'duration': 600}  # Last 10 minutes
)
history = response.json()
print(f"Retrieved {len(history)} data points")
```

### Automated Setpoint Stepping

Use REST endpoints:

```python
import requests, time

for setpoint in [2.0, 2.5, 3.0, 3.5, 4.0]:
    requests.post(
        'http://localhost:8000/api/setpoint',
        json={'value': setpoint}
    )
    time.sleep(30)  # Wait for controller to settle
    history = requests.get('http://localhost:8000/api/history?duration=30').json()
    print(f"Setpoint {setpoint}: avg level = {sum(h['tank_level'] for h in history)/len(history):.2f}")
```

---

## Troubleshooting

### Connection Refused

**Problem:** `Connection refused` when connecting to localhost:8000

**Solution:** Verify the API server is running:

```bash
python -m api.main
```

### CORS Errors

**Problem:** Browser shows CORS error when accessing from a different origin

**Solution:** Update CORS allowed origins in `api/main.py` (see Deployment guide).

### WebSocket Connection Fails

**Problem:** WebSocket connection fails or drops frequently

**Solution:**
1. Check reverse proxy configuration (nginx) has proper WebSocket headers
2. Verify firewall allows WebSocket connections
3. Check API server logs: `journalctl -u tank-sim-api -f`

### Invalid Command Errors

**Problem:** Receiving "Invalid message format" or "Unknown message type"

**Solution:** Verify command JSON matches specification exactly, including field names and capitalization.
