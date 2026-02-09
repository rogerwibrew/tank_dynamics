# Tank Dynamics Simulator

A real-time tank level control simulator with a SCADA-style interface. The system models a tank with variable inlet flow and PID-controlled outlet valve, allowing operators to experiment with control parameters and observe process dynamics.

## Overview

Tank Dynamics is a proof-of-concept process simulation and control system. It demonstrates real-time simulation of a liquid tank with tunable PID level control. Process operators can monitor tank level, flow rates, and valve position in real-time, while experimenting with different PID controller tuning parameters to understand process control behavior.

## Features

- **Real-time Simulation**: Physics-based tank model running at 1 Hz with RK4 numerical integration
- **PID Control Loop**: Fully tunable proportional-integral-derivative controller for tank level setpoint
- **SCADA Interface**: Modern web-based operator interface with live process visualization
- **Trend Plotting**: Historical data visualization with configurable time ranges
- **Manual & Auto Inlet Modes**: Manual inlet flow control or simulated Brownian motion disturbances
- **Persistent Data**: Up to 2 hours of process history for analysis

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Browser (Next.js)                        │
│  ┌─────────────────────────┐  ┌─────────────────────────────┐  │
│  │    Process View (Tab)   │  │     Trends View (Tab)       │  │
│  │  - Tank visualization   │  │  - Level vs Setpoint plot   │  │
│  │  - PID controls         │  │  - Flow plots (in/out)      │  │
│  │  - Flow indicators      │  │  - Valve position           │  │
│  │  - Setpoint input       │  │  - Historical data          │  │
│  └─────────────────────────┘  └─────────────────────────────┘  │
└────────────────────────────────┬────────────────────────────────┘
                                 │ WebSocket (1 Hz updates)
                                 ▼
┌─────────────────────────────────────────────────────────────────┐
│                    FastAPI Server (Python)                      │
│  - WebSocket endpoint for real-time state                       │
│  - REST endpoints for control and history                       │
│  - Simulation orchestration (1 Hz tick rate)                    │
│  - Ring buffer history (~2 hours of data)                       │
└────────────────────────────────┬────────────────────────────────┘
                                 │ pybind11
                                 ▼
┌─────────────────────────────────────────────────────────────────┐
│                 C++ Simulation Library                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐  │
│  │  Tank Model  │  │ PID Control  │  │  RK4 Stepper (GSL)   │  │
│  │  - ODEs      │  │  - Gains     │  │  - Fixed timestep    │  │
│  │  - Valve     │  │  - Integral  │  │  - State integration │  │
│  └──────────────┘  └──────────────┘  └──────────────────────┘  │
│                           Eigen (vectors/matrices)              │
└─────────────────────────────────────────────────────────────────┘
```

**Components:**
- **C++ Simulation Core** (`libsim`): High-performance physics engine using GSL RK4 integrator and Eigen linear algebra
- **Python Bindings** (`tank_sim`): pybind11 interface exposing simulation to Python
- **FastAPI Backend** (`api/`): Real-time WebSocket server orchestrating simulation
- **Next.js Frontend** (`frontend/`): Modern React-based SCADA interface with Tailwind CSS styling

## Quick Start

### Prerequisites

**System Dependencies:**

Ubuntu/Debian:
```bash
sudo apt-get install cmake libgsl-dev build-essential
```

Arch Linux:
```bash
sudo pacman -S cmake gsl base-devel
```

macOS:
```bash
brew install cmake gsl
```

**Development Tools:**
- Node.js 18+ (for frontend)
- Python 3.9+ (for backend)
- C++17 capable compiler (GCC 9+, Clang 10+, MSVC 2019+)

### Building the C++ Core

```bash
# Configure build system (downloads Eigen, GSL, GoogleTest automatically)
cmake -B build -S .

# Compile C++ library and tests
cmake --build build

# Run test suite
ctest --test-dir build --output-on-failure
```

### Running the Complete System

```bash
# Start backend (requires C++ library built first)
pip install -e .  # Install Python bindings
python -m api.main

# In another terminal, start frontend
cd frontend
npm install
npm run dev
```

Then open http://localhost:3000 in your browser.

## Project Structure

```
tank_dynamics/
├── CMakeLists.txt              # C++ build configuration
├── src/                        # C++ simulation library
│   ├── tank_model.h            # Tank physics model
│   ├── tank_model.cpp
│   ├── pid_controller.h        # PID controller with state
│   ├── pid_controller.cpp
│   ├── stepper.h               # GSL RK4 integrator wrapper
│   ├── stepper.cpp
│   ├── simulator.h             # Main simulation orchestrator
│   └── simulator.cpp
├── bindings/                   # pybind11 Python bindings
│   └── bindings.cpp
├── tests/                      # C++ unit tests (GoogleTest)
│   ├── test_tank_model.cpp
│   ├── test_pid_controller.cpp
│   ├── test_stepper.cpp
│   └── test_simulator.cpp
├── api/                        # FastAPI backend
│   ├── main.py                 # WebSocket server
│   ├── simulation.py           # Simulation loop
│   └── models.py               # Data models
├── frontend/                   # Next.js frontend
│   ├── app/
│   │   ├── layout.tsx
│   │   ├── page.tsx
│   │   └── components/
│   │       ├── ProcessView.tsx
│   │       ├── TrendsView.tsx
│   │       └── TankGraphic.tsx
│   ├── tailwind.config.js
│   └── package.json
├── docs/                       # Project documentation
│   ├── specs.md                # Feature specifications
│   ├── plan.md                 # Architecture & design plan
│   └── next.md                 # Upcoming tasks
└── CLAUDE.md                   # AI workflow configuration
```

## Process Dynamics

### Tank Model

The tank is modeled as a first-order system with one state variable (liquid level `h`):

```
Material Balance: dh/dt = (q_in - q_out) / A
Valve Equation:   q_out = k_v * x * sqrt(h)
```

Where:
- `h`: Tank level (meters)
- `q_in`: Inlet volumetric flow (m³/s)
- `q_out`: Outlet volumetric flow (m³/s)
- `A`: Cross-sectional area = 120 m²
- `k_v`: Valve coefficient = 1.2649 m^2.5/s
- `x`: Valve position (0 = closed, 1 = fully open)

### Control Loop

The PID controller continuously compares tank level against the setpoint and outputs a valve position (0-1):

```
error = setpoint - actual_level
valve_position = Kc * (error + (1/tau_I) * ∫error + tau_D * d(error)/dt)
```

The controller gains are tunable in real-time:
- **Kc** (proportional gain): Larger = more aggressive response
- **tau_I** (integral time): Smaller = faster offset correction
- **tau_D** (derivative time): Larger = more damping of oscillations

## Development Guide

### Documentation

**For Developers:**
- **[Developer Guide](docs/DEVELOPER_GUIDE.md)** - Setup, building, testing, and development workflow
- **[API Reference](docs/API_REFERENCE.md)** - Complete C++ class documentation with examples
- **[Architecture Plan](docs/plan.md)** - System design, technology decisions, and phase breakdown

**For Understanding the Project:**
- **[Process Specifications](docs/specs.md)** - Feature requirements and acceptance criteria
- **[Tank Dynamics Theory](docs/TankDynamics.md)** - Process physics and control theory
- **[Detailed Class Specifications](docs/)** - `Model Class.md`, `PID Controller Class.md`, `Stepper Class.md`, `Simulator Class.md`

**For Current Work:**
- **[Project Status](docs/STATUS.md)** - Detailed progress report and completed work
- **[Next Tasks](docs/next.md)** - Current implementation phase and upcoming work

### Workflow Roles

This project uses a structured AI-assisted workflow:

| Role | Model | Responsibility |
|------|-------|-----------------|
| Architect | Claude Opus | Strategic planning & design |
| Senior Engineer | Claude Sonnet | Task breakdown & prioritization |
| Engineer | Local LLM + Human | Implementation |
| Code Reviewer | Claude Sonnet | Quality assurance |
| Documentation | Claude Haiku | User/developer documentation |

See `CLAUDE.md` for detailed role definitions and boundaries.

### Current Phase: Phase 3 - FastAPI Backend [COMPLETE]

**Progress:** 100% complete - All phases now ready for merge
- ✅ Phase 1: C++ Simulation Core (9 tasks, 42 C++ tests passing)
- ✅ Phase 2: Python Bindings (3 tasks, 28 Python tests passing)
- ✅ Phase 3: FastAPI Backend (3 tasks, complete with all 70+ tests passing)

**Phase 3 Deliverables:**
- ✅ Task 13: FastAPI project structure with Pydantic models and core endpoints
- ✅ Task 14: Simulation loop (1 Hz) and WebSocket real-time broadcasting
- ✅ Task 15: Ring buffer history (7200 entries, ~2 hours) and REST endpoints
- ✅ Full integration with C++ simulation backend via pybind11

**Fully Operational System:**
- Python bindings fully functional and tested (28 tests)
- C++ simulation core production-ready (42 tests)
- FastAPI server with WebSocket real-time updates at 1 Hz
- REST endpoints for configuration, control, and history queries
- Ring buffer for persistent historical data storage
- CORS enabled for frontend integration
- Comprehensive logging for debugging

### Running Tests

```bash
# C++ tests
ctest --test-dir build --output-on-failure

# C++ tests with detailed output
./build/tests/test_tank_model --gtest_detail=all

# Python tests (after bindings built)
pytest api/tests/ -v
```

### IDE Setup (clangd)

For proper code completion and go-to-definition:

```bash
# From project root - create symlink to compile database
ln -sf build/compile_commands.json compile_commands.json
```

Works with VSCode (clangd extension), Neovim (nvim-lspconfig), Emacs (eglot), etc.

## API Reference

### Running the API Server

```bash
# From project root
pip install -e .                          # Install Python bindings
pip install -r api/requirements.txt       # Install API dependencies

# Development mode (with auto-reload)
uvicorn api.main:app --reload --host 0.0.0.0 --port 8000

# Production mode (single worker, no reload)
uvicorn api.main:app --host 0.0.0.0 --port 8000 --workers 1
```

**Important:** Always use 1 worker for production to ensure a single simulation instance.

API documentation available at http://localhost:8000/docs (Swagger UI)

### Health Check: `GET /api/health`

Simple status check for monitoring and deployment health checks.

**Response:**
```json
{"status": "ok"}
```

### Configuration: `GET /api/config`

Get current simulation configuration including tank parameters, PID gains, and history capacity.

**Response:**
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
  "history_size": 245
}
```

### Reset: `POST /api/reset`

Reset the simulation to initial steady-state conditions and clear history buffer.

**Response:**
```json
{"message": "Simulation reset successfully"}
```

### Set Setpoint: `POST /api/setpoint`

Change the tank level setpoint (target level for PID controller).

**Request:**
```json
{"value": 3.5}
```

**Constraints:** 0.0 ≤ value ≤ 5.0 (meters)

**Response:**
```json
{"message": "Setpoint updated", "value": 3.5}
```

### Tune PID: `POST /api/pid`

Update PID controller gains dynamically (bumpless transfer).

**Request:**
```json
{
  "Kc": 1.5,
  "tau_I": 8.0,
  "tau_D": 2.0
}
```

**Constraints:**
- `Kc`: proportional gain (no restriction, can be negative for reverse-acting control)
- `tau_I`: integral time (≥ 0, 0 disables integral action)
- `tau_D`: derivative time (≥ 0, 0 disables derivative action)

**Response:**
```json
{
  "message": "PID gains updated",
  "gains": {"Kc": 1.5, "tau_I": 8.0, "tau_D": 2.0}
}
```

### Set Inlet Flow: `POST /api/inlet_flow`

Change the inlet flow rate manually (when in constant mode).

**Request:**
```json
{"value": 1.2}
```

**Constraints:** 0.0 ≤ value ≤ 2.0 (m³/s)

**Response:**
```json
{"message": "Inlet flow updated", "value": 1.2}
```

### Set Inlet Mode: `POST /api/inlet_mode`

Switch inlet between constant and Brownian (random walk) modes.

**Request:**
```json
{
  "mode": "brownian",
  "min_flow": 0.8,
  "max_flow": 1.2
}
```

**Constraints:**
- `mode`: "constant" or "brownian"
- `min_flow`: ≥ 0.0, ≤ 2.0
- `max_flow`: > min_flow, ≤ 2.0

**Response:**
```json
{
  "message": "Inlet mode updated",
  "mode": "brownian",
  "min_flow": 0.8,
  "max_flow": 1.2
}
```

### History: `GET /api/history?duration=3600`

Retrieve historical data points from the ring buffer.

**Query Parameters:**
- `duration`: seconds of history to return (1-7200, default 3600 = 1 hour)

**Response:** Array of state snapshots in chronological order (oldest first)
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
    "tank_level": 2.501,
    "setpoint": 2.5,
    "inlet_flow": 1.0,
    "outlet_flow": 0.999,
    "valve_position": 0.501,
    "error": -0.001,
    "controller_output": 0.499
  }
]
```

### WebSocket: `WS /ws`

Real-time bidirectional connection for continuous state updates and command input.

**Server → Client (State Updates - every 1 second):**
```json
{
  "type": "state",
  "data": {
    "time": 1234.5,
    "tank_level": 2.5,
    "setpoint": 3.0,
    "inlet_flow": 1.0,
    "outlet_flow": 1.0,
    "valve_position": 0.5,
    "error": -0.5,
    "controller_output": 0.48
  }
}
```

**Server → Client (Error Messages):**
```json
{
  "type": "error",
  "message": "Invalid JSON format"
}
```

**Client → Server (Control Commands):**
```json
{"type": "setpoint", "value": 3.0}
{"type": "pid", "Kc": 1.0, "tau_I": 10.0, "tau_D": 0.0}
{"type": "inlet_flow", "value": 1.2}
{"type": "inlet_mode", "mode": "brownian", "min": 0.8, "max": 1.2}
```

#### WebSocket Connection Example (Python)

```python
import asyncio
import json
import websockets

async def test_websocket():
    uri = "ws://localhost:8000/ws"
    async with websockets.connect(uri) as websocket:
        # Receive state updates for 10 seconds
        for _ in range(10):
            message = await websocket.recv()
            data = json.loads(message)
            print(f"Time: {data['data']['time']}, Level: {data['data']['tank_level']:.2f}m")
        
        # Send a setpoint change
        await websocket.send(json.dumps({"type": "setpoint", "value": 3.5}))
        
        # Continue receiving
        for _ in range(10):
            message = await websocket.recv()
            data = json.loads(message)
            print(f"Time: {data['data']['time']}, Level: {data['data']['tank_level']:.2f}m")

asyncio.run(test_websocket())
```

#### WebSocket Connection Example (JavaScript/Node.js)

```javascript
const WebSocket = require('ws');

const ws = new WebSocket('ws://localhost:8000/ws');

ws.on('open', () => {
  console.log('Connected to server');
  
  // Send a setpoint command after 5 seconds
  setTimeout(() => {
    ws.send(JSON.stringify({type: 'setpoint', value: 3.0}));
  }, 5000);
});

ws.on('message', (data) => {
  const message = JSON.parse(data);
  if (message.type === 'state') {
    console.log(`Level: ${message.data.tank_level.toFixed(2)}m, Setpoint: ${message.data.setpoint.toFixed(2)}m`);
  } else if (message.type === 'error') {
    console.error(`Error: ${message.message}`);
  }
});

ws.on('error', (err) => {
  console.error('WebSocket error:', err);
});

ws.on('close', () => {
  console.log('Disconnected from server');
});
```

## Process Parameters

| Parameter | Value | Unit |
|-----------|-------|------|
| Tank height | 5.0 | m |
| Tank area | 120.0 | m² |
| Tank volume | 600.0 | m³ |
| Valve coefficient (k_v) | 1.2649 | m^2.5/s |
| Steady-state level | 2.5 | m |
| Steady-state inlet flow | 1.0 | m³/s |

## Troubleshooting

### CMake FetchContent Issues

If `cmake -B build -S .` fails downloading dependencies:

```bash
# Clear CMake cache and try again
rm -rf build
cmake -B build -S .

# Or manually specify GSL location
cmake -B build -S . -DGSL_ROOT_DIR=/usr/local
```

### Build Errors

Ensure you have a C++17 capable compiler:

```bash
gcc --version  # Should be 9.0+
g++ -std=c++17 -v  # Verify C++17 support
```

### WebSocket Connection Issues

Check that the FastAPI backend is running on port 8000:

```bash
curl http://localhost:8000/api/config
```

If backend isn't running, start it:

```bash
python -m api.main
```

## References

- **Tank Dynamics Theory**: See `docs/TankDynamics.md`
- **Tennessee Eastman Process**: See `docs/Tennessee_Eastman_Process_Equations.md`
- **Complete Architecture Plan**: See `docs/plan.md`
- **Next Implementation Tasks**: See `docs/next.md`

## License

This project is provided for educational and personal use.

## Contributing

Follow the workflow defined in `CLAUDE.md` when contributing:

1. Read the relevant role prompt (`prompts/*.md`)
2. Follow role boundaries strictly
3. Commit after each completed task with descriptive message
4. Escalate to higher-tier Claude models when appropriate

---

**Last Updated:** 2026-02-09
**Current Status:** Phase 3 Complete - All Phases Ready for Merge
**Next Phase:** Phase 4 - Next.js Frontend (can begin immediately)
