# FastAPI Backend - Tank Dynamics Simulator

Real-time tank level control simulation API built with FastAPI and WebSockets.

## Quick Start

### Prerequisites

- Python 3.10+
- C++ library built (see [main project README](../README.md#building-the-c-core))

### Installation

```bash
# From project root
pip install -e .                          # Install Python bindings
pip install -r api/requirements.txt       # Install API dependencies
```

Or with `uv` (faster):

```bash
uv pip install -e .
uv sync --extra dev
```

### Running the Server

**Development mode** (with auto-reload):

```bash
uvicorn api.main:app --reload --host 0.0.0.0 --port 8000
```

**Production mode** (single worker, no reload):

```bash
uvicorn api.main:app --host 0.0.0.0 --port 8000 --workers 1
```

The server will start on `http://localhost:8000`.

### Access the API

- **Swagger UI (Interactive Docs):** http://localhost:8000/docs
- **ReDoc (Alternative Docs):** http://localhost:8000/redoc
- **Health Check:** http://localhost:8000/api/health

## API Overview

The API provides both **REST** and **WebSocket** interfaces:

### REST Endpoints

- `GET /api/health` - Health check
- `GET /api/state` - Get current simulation state
- `GET /api/config` - Get configuration parameters
- `GET /api/history?duration=3600` - Get historical data (up to 2 hours)
- `POST /api/setpoint` - Change setpoint
- `POST /api/pid` - Update PID gains
- `POST /api/inlet_flow` - Set inlet flow rate
- `POST /api/inlet_mode` - Switch inlet mode (constant/brownian)
- `POST /api/reset` - Reset simulation

### WebSocket Endpoint

- `WS /ws` - Real-time bidirectional state updates and command interface

## Documentation

For complete API reference, see **[API_REFERENCE.md](../docs/API_REFERENCE.md)**

For deployment in production, see **[DEPLOYMENT.md](../docs/DEPLOYMENT.md)**

## Project Structure

```
api/
├── README.md              # This file
├── main.py               # FastAPI application and endpoints
├── models.py             # Pydantic data models
├── simulation.py         # Simulation manager and loop
├── requirements.txt      # Python dependencies
└── tests/
    ├── conftest.py       # pytest fixtures and configuration
    ├── test_endpoints.py # REST endpoint tests
    ├── test_websocket.py # WebSocket tests
    ├── test_concurrent.py # Concurrency tests
    └── test_brownian.py  # Brownian inlet mode tests
```

## Running Tests

```bash
# Run all tests
pytest api/tests/ -v

# Run specific test file
pytest api/tests/test_endpoints.py -v

# Run with coverage
pytest api/tests/ --cov=api --cov-report=term-missing

# Run single test
pytest api/tests/test_endpoints.py::test_health_check -v
```

### Test Coverage

The API test suite covers:

- **REST endpoints:** All 9 REST endpoints with valid/invalid inputs
- **WebSocket:** Connection, message handling, command routing, disconnection
- **Concurrency:** Multiple simultaneous WebSocket connections
- **Brownian Mode:** Stochastic inlet flow simulation
- **Error Handling:** Invalid JSON, missing fields, validation errors
- **Integration:** Full request/response cycles with actual simulation

Expected coverage: 85%+ of API code (excluding logging calls).

## Development Tips

### Auto-Reload Development Server

The `--reload` flag watches for file changes and restarts the server:

```bash
uvicorn api.main:app --reload
```

Any changes to `api/main.py`, `api/models.py`, or `api/simulation.py` trigger a restart.

### Debugging

Enable DEBUG logging:

```python
# In api/main.py, change logging level:
logging.basicConfig(level=logging.DEBUG)
```

Or via environment:

```bash
LOG_LEVEL=DEBUG uvicorn api.main:app --reload
```

View debug output:

```bash
uvicorn api.main:app --reload --log-level debug
```

### Testing Endpoints Manually

Using `curl`:

```bash
# Health check
curl http://localhost:8000/api/health

# Get state
curl http://localhost:8000/api/state

# Get config
curl http://localhost:8000/api/config

# Set setpoint
curl -X POST http://localhost:8000/api/setpoint \
  -H "Content-Type: application/json" \
  -d '{"value": 3.5}'

# Get history (last 5 minutes)
curl 'http://localhost:8000/api/history?duration=300'
```

Using Python `requests`:

```python
import requests

# Get state
resp = requests.get('http://localhost:8000/api/state')
print(resp.json())

# Set setpoint
resp = requests.post(
    'http://localhost:8000/api/setpoint',
    json={'value': 3.5}
)
print(resp.json())
```

### Testing WebSocket Manually

Using `websocat` (install with `cargo install websocat` or package manager):

```bash
websocat ws://localhost:8000/ws

# Then type JSON commands:
{"type": "setpoint", "value": 3.5}
{"type": "pid", "Kc": 1.5, "tau_I": 8.0, "tau_D": 2.0}
```

Or using Python:

```python
import asyncio, json, websockets

async def test():
    async with websockets.connect('ws://localhost:8000/ws') as ws:
        # Receive 5 state updates
        for _ in range(5):
            msg = await ws.recv()
            data = json.loads(msg)
            print(f"Level: {data['data']['tank_level']:.2f}m")
        
        # Send a command
        await ws.send(json.dumps({
            'type': 'setpoint',
            'value': 3.5
        }))
        
        # Receive more updates
        for _ in range(5):
            msg = await ws.recv()
            data = json.loads(msg)
            print(f"Level: {data['data']['tank_level']:.2f}m")

asyncio.run(test())
```

## Examples

Complete working examples are provided in the `examples/` directory:

- **`websocket_client.py`** - Python WebSocket client
- **`websocket_client.html`** - JavaScript/Browser WebSocket client
- **`rest_client.py`** - Python REST API client

Run them:

```bash
# Python WebSocket client
python examples/websocket_client.py

# Python REST client
python examples/rest_client.py

# HTML client (open in browser)
open examples/websocket_client.html
```

## Key Design Decisions

### Single Worker

The API always runs with `--workers 1` because:
- Simulation state is maintained in memory (singleton)
- Multiple workers would create separate simulator instances
- WebSocket broadcasts would only reach clients on the same worker
- Result: state divergence between clients

### 1 Hz Simulation Tick

State updates broadcast at 1 Hz:
- Matches simulation physics (1-second time step)
- Sufficient for tank-level process dynamics
- Reduces network bandwidth vs. higher frequencies
- Clients receive 60 updates per minute

### Ring Buffer History

Up to 7200 historical data points (~2 hours at 1 Hz):
- Fixed-size deque for memory efficiency
- Oldest data automatically discarded when full
- Suitable for trend visualization and analysis
- Larger buffers increase memory usage (minor impact)

## Architecture

```
FastAPI Application
├── WebSocket Endpoint (/ws)
│   └── Broadcasts state updates every 1 second
│   └── Routes incoming commands
├── REST Endpoints
│   ├── Health & Config (/api/health, /api/config)
│   ├── Control (/api/setpoint, /api/pid, /api/inlet_flow, /api/inlet_mode)
│   ├── Queries (/api/state, /api/history)
│   └── Management (/api/reset)
└── Simulation Loop
    ├── Reads state from C++ simulator (via pybind11)
    ├── Steps simulation forward 1 second
    ├── Applies inlet flow commands (constant or Brownian)
    └── Stores state in history buffer
```

## Troubleshooting

### Import Errors

**`ModuleNotFoundError: No module named 'tank_sim'`**

The C++ library hasn't been built or isn't installed:

```bash
# From project root
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
pip install -e .
```

### Port Already in Use

**`Address already in use`**

```bash
# Use a different port
uvicorn api.main:app --port 8001

# Or kill existing process
lsof -i :8000
kill -9 <PID>
```

### WebSocket Connection Issues

**Connection refused or fails**

1. Verify API is running: `curl http://localhost:8000/api/health`
2. Check port: `netstat -ln | grep 8000`
3. Firewall blocking: `sudo ufw allow 8000`

### Slow Response Times

If endpoints are slow:

1. Check simulation isn't stuck: `curl http://localhost:8000/api/state`
2. Check CPU usage: `top` or `htop`
3. Check memory usage: `free -h`
4. Review logs for errors: `journalctl -u tank-sim-api`

## Next Steps

- **[Deploy to Production](../docs/DEPLOYMENT.md)** - Systemd service, nginx proxy, TLS setup
- **[Complete API Reference](../docs/API_REFERENCE.md)** - All endpoints, data models, examples
- **[Architecture Overview](../docs/plan.md)** - System design and technology decisions
