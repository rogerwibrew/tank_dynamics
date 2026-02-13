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
sudo apt-get install cmake libgsl-dev build-essential python3-pip
```

Arch Linux:
```bash
sudo pacman -S cmake gsl base-devel python
```

macOS:
```bash
brew install cmake gsl python
```

**Development Tools:**
- Python 3.10+ (for backend)
- C++17 capable compiler (GCC 9+, Clang 10+, MSVC 2019+)
- Node.js 18+ (optional, for frontend in Phase 4)

### Building the C++ Core

```bash
# Configure build system (downloads Eigen, GSL, GoogleTest automatically)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Compile C++ library and tests
cmake --build build --config Release

# Run test suite
ctest --test-dir build --output-on-failure
```

### Running the FastAPI Backend

```bash
# Install Python bindings and API dependencies
pip install -e .
pip install -r api/requirements.txt

# Development mode (with auto-reload)
uvicorn api.main:app --reload --host 0.0.0.0 --port 8000

# Production mode (always use 1 worker)
uvicorn api.main:app --host 0.0.0.0 --port 8000 --workers 1
```

The API is now running at `http://localhost:8000`. Access Swagger UI documentation at `http://localhost:8000/docs`.

### Testing the API

```bash
# Run all API tests
pytest api/tests/ -v

# Run with coverage
pytest api/tests/ --cov=api

# Try the interactive examples
python examples/rest_client.py           # REST API client
python examples/websocket_client.py      # WebSocket client
# Or open examples/websocket_client.html in a web browser
```

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

## Documentation

### API Documentation

- **[API_REFERENCE.md](docs/API_REFERENCE.md)** - Complete endpoint reference with examples for all 9 REST endpoints and WebSocket interface
- **[api/README.md](api/README.md)** - Quick start guide, testing, and development tips
- **[DEPLOYMENT.md](docs/DEPLOYMENT.md)** - Production deployment guide with systemd service, nginx proxy, TLS/SSL setup, and troubleshooting

### Example Code

Located in `examples/` directory:

- **[websocket_client.py](examples/websocket_client.py)** - Python WebSocket client demonstrating real-time state monitoring and command sending
- **[rest_client.py](examples/rest_client.py)** - Python REST API client showing all endpoint usage patterns
- **[websocket_client.html](examples/websocket_client.html)** - Self-contained HTML/JavaScript WebSocket client with interactive control panel (works in any browser)

### Development Documentation

**For Developers:**
- **[Developer Guide](docs/DEVELOPER_GUIDE.md)** - Setup, building, testing, and development workflow
- **[Architecture Plan](docs/plan.md)** - System design, technology decisions, and phase breakdown

**For Understanding the Project:**
- **[Process Specifications](docs/specs.md)** - Feature requirements and acceptance criteria
- **[Tank Dynamics Theory](docs/TankDynamics.md)** - Process physics and control theory
- **[Detailed Class Specifications](docs/)** - Tank Model, PID Controller, Stepper, and Simulator classes

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

### Current Phase: Phase 6 - Trends View Enhancement & Polish [✅ COMPLETE]

**Progress:** Trends View complete (2026-02-13) - Historical data visualization with real-time updates
- ✅ Phase 1: C++ Simulation Core (9 tasks, 42 C++ tests passing)
- ✅ Phase 2: Python Bindings (3 tasks, 28 Python tests passing)
- ✅ Phase 3: FastAPI Backend (4 tasks, complete with all 70+ tests passing)
- ✅ Phase 4: Next.js Frontend Foundation (21 tasks complete, base interface ready)
- ✅ Phase 5: Process View (12 tasks complete, SVG graphics + controls + flow indicators)
- ✅ Phase 6: Trends View Enhancement (14 tasks complete, charts + interactions + polish)

**Phase 3 Deliverables:**
- ✅ Task 13: FastAPI project structure with Pydantic models and core endpoints
- ✅ Task 14: Simulation loop (1 Hz) and WebSocket real-time broadcasting
- ✅ Task 15: Ring buffer history (7200 entries, ~2 hours) and REST endpoints
- ✅ Task 16: Comprehensive API Testing Suite (conftest, endpoints, WebSocket, concurrency, Brownian)
- ✅ Task 17: Brownian Inlet Flow Mode (stochastic disturbance simulation)
- ✅ Task 18: API Documentation and Production Deployment Guide

**Phase 4 Deliverables (Foundation):**
- ✅ Task 19a-19k: Next.js project initialization, TypeScript config, Tailwind CSS theming
- ✅ Task 20a-20f: WebSocket client, useWebSocket hook, SimulationProvider context
- ✅ Task 21a-21e: TabNavigation, ConnectionStatus, ProcessView, TrendsView, Home Page integration
- ✅ Task 21f: Complete frontend application testing (all components functional)

**Phase 5 Deliverables (Process View):**
- ✅ Task 22-27: Tank visualization (SVG graphics with realistic valve/inlet design)
- ✅ Task 25: Real-time inlet flow control with PID parameter tuning
- ✅ Task 26-27: Flow direction indicators, Brownian inlet mode toggle, smooth animations
- ✅ Complete SCADA interface with responsive layout and error handling

**Phase 6 Deliverables (Trends View Enhancement):**
- ✅ Task 28a-28f: Polish tasks (animation control, constants extraction, help text, error colors, PID config fetch)
- ✅ Task 29a-29h: Historical data visualization
  - Task 29a: useHistory hook for ring buffer data retrieval
  - Task 29b: LevelChart component (tank level vs setpoint)
  - Task 29c: FlowsChart component (inlet and outlet flow rates)
  - Task 29d: ValveChart component (valve position over time)
  - Task 29e: Integration into TrendsView with responsive layout
  - Task 29f: Real-time data append for live chart updates
  - Task 29g: Time range selector (1 hour, 30 mins, custom range)
  - Task 29h: Interactive chart features (custom tooltips, legend interactions)

**Fully Operational System:**
- Python bindings fully functional and tested (28 tests)
- C++ simulation core production-ready (42 tests)
- FastAPI server with WebSocket real-time updates at 1 Hz
- REST endpoints for configuration, control, and history queries
- Ring buffer for persistent historical data storage (7200 entries, ~2 hours)
- Comprehensive API test suite (70+ tests)
- Brownian inlet flow mode for disturbance testing
- Complete deployment documentation with systemd service, nginx proxy, and TLS setup
- CORS enabled for frontend integration
- Comprehensive logging for debugging
- Interactive example clients (Python WebSocket, Python REST, HTML WebSocket)
- SCADA interface with tank visualization and real-time controls
- Historical trend charts with Recharts (Level, Flows, Valve Position)
- Time range selection and chart interactions
- Production-ready frontend with error handling and responsive design

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

**Complete API documentation with examples:**
- **[API_REFERENCE.md](docs/API_REFERENCE.md)** - Comprehensive endpoint reference with request/response examples
- **[api/README.md](api/README.md)** - Quick start guide and development tips
- **[Interactive Swagger UI](http://localhost:8000/docs)** - Available when API server is running

### Quick API Examples

**Health Check:**
```bash
curl http://localhost:8000/api/health
```

**Get Current State:**
```bash
curl http://localhost:8000/api/state
```

**Set Setpoint:**
```bash
curl -X POST http://localhost:8000/api/setpoint \
  -H "Content-Type: application/json" \
  -d '{"value": 3.5}'
```

**Update PID Gains:**
```bash
curl -X POST http://localhost:8000/api/pid \
  -H "Content-Type: application/json" \
  -d '{"Kc": 1.5, "tau_I": 8.0, "tau_D": 2.0}'
```

**Get Historical Data:**
```bash
curl 'http://localhost:8000/api/history?duration=3600'
```

For complete examples, see [examples/](examples/) directory.



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

## Testing

### Running Tests

```bash
# C++ unit tests
cmake --build build --config Release
ctest --test-dir build --output-on-failure

# Python API tests
pytest api/tests/ -v

# With coverage report
pytest api/tests/ --cov=api --cov-report=html
```

**Test Coverage:**
- C++ simulation core: 42 tests
- Python bindings: 28 tests
- FastAPI endpoints: 70+ tests
- WebSocket integration: Full coverage
- Brownian inlet mode: Complete test suite

## References

- **Complete API Reference**: See `docs/API_REFERENCE.md`
- **Deployment Guide**: See `docs/DEPLOYMENT.md`
- **Tank Dynamics Theory**: See `docs/TankDynamics.md`
- **Tennessee Eastman Process**: See `docs/Tennessee_Eastman_Process_Equations.md`
- **Architecture Plan**: See `docs/plan.md`
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

**Last Updated:** 2026-02-13 (Phase 6 Trends View Complete)
**Current Status:** Phase 6 Complete - 6 phases with full SCADA interface and historical charts
**Next Phase:** Phase 7 - Integration and Polish

**Phase 6 Completion Summary:**
- ✅ Historical data visualization with Recharts
- ✅ Three chart types: Level, Flows, Valve Position
- ✅ Real-time data streaming to charts
- ✅ Time range selector (1 hour, 30 mins, custom)
- ✅ Interactive tooltips and legend controls
- ✅ Code review feedback addressed (6 polish tasks)
- ✅ All animation and styling improvements
- ✅ Complete TrendsView component integration

**System Completion Status:**
- ✅ Phase 1: C++ Core (42 tests, 100% pass)
- ✅ Phase 2: Python Bindings (28 tests, 100% pass)
- ✅ Phase 3: FastAPI Backend (70+ tests, production-ready)
- ✅ Phase 4: Frontend Foundation (21 tasks, base interface)
- ✅ Phase 5: Process View (12 tasks, SCADA interface)
- ✅ Phase 6: Trends View (14 tasks, historical visualization)
