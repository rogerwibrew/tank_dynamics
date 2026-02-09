# Next Tasks - Tank Dynamics Simulator

## Current Phase: Phase 3 - FastAPI Backend

**Status:** Starting Phase 3 - Python bindings complete and tested

**Progress:** 0% - Phase 3 just beginning

**Phase 2 Completion:**
- ✅ pybind11 module structure created
- ✅ All C++ classes bound to Python
- ✅ 28 Python tests passing (100% pass rate)
- ✅ Modern Python packaging with scikit-build-core
- ✅ Code review feedback implemented
- ✅ Complete documentation and examples
- ✅ Ready for FastAPI integration

---

## Task 13: Create FastAPI Project Structure

**Phase:** 3 - FastAPI Backend
**Prerequisites:** Phase 2 complete (Python bindings working)

### Files to Create

- Create `/home/roger/dev/tank_dynamics/api/__init__.py`
- Create `/home/roger/dev/tank_dynamics/api/main.py` (FastAPI application entry point)
- Create `/home/roger/dev/tank_dynamics/api/models.py` (Pydantic models for API)
- Create `/home/roger/dev/tank_dynamics/api/simulation.py` (Simulation orchestration)
- Create `/home/roger/dev/tank_dynamics/api/requirements.txt` (API dependencies)
- Create `/home/roger/dev/tank_dynamics/api/.env.example` (Example environment configuration)

### Requirements

This task establishes the FastAPI project structure and basic application setup without implementing the simulation loop or WebSocket functionality yet. The goal is to create a working FastAPI application with proper project organization and configuration.

#### Directory Structure:

The api directory should be organized as follows:
- main.py contains the FastAPI application instance, startup/shutdown events, and route imports
- models.py contains Pydantic models for request/response validation and type safety
- simulation.py will contain the simulation loop and state management (minimal stub for now)
- requirements.txt lists all Python dependencies needed for the API server
- __init__.py makes the directory a Python package

#### api/models.py specifications:

This file defines Pydantic models representing the data structures used in the API. These provide automatic validation, serialization, and documentation.

Create the following Pydantic models:

A model representing the simulation state snapshot:
- time: float (simulation time in seconds)
- tank_level: float (current tank level in meters)
- setpoint: float (level setpoint in meters)
- inlet_flow: float (inlet flow rate in cubic meters per second)
- outlet_flow: float (outlet flow rate in cubic meters per second)
- valve_position: float (valve position from 0 to 1)
- error: float (control error: setpoint minus level)
- controller_output: float (PID controller output, 0 to 1)

A model for setpoint change commands:
- value: float (new setpoint in meters)
- Validation: must be between 0.0 and 5.0 (maximum tank height)

A model for PID tuning commands:
- Kc: float (proportional gain, must be non-negative)
- tau_I: float (integral time constant in seconds, 0 means no integral action)
- tau_D: float (derivative time constant in seconds, 0 means no derivative action)
- Validation: all values must be non-negative

A model for inlet flow commands:
- value: float (new inlet flow in cubic meters per second)
- Validation: must be between 0.0 and 2.0 (reasonable operating range)

A model for inlet mode commands:
- mode: string (either "constant" or "brownian")
- min_flow: optional float (minimum flow for Brownian mode, default 0.8)
- max_flow: optional float (maximum flow for Brownian mode, default 1.2)
- Validation: mode must be one of the allowed values, min must be less than max

A model for configuration response:
- tank_height: float (maximum tank height in meters)
- tank_area: float (cross-sectional area in square meters)
- valve_coefficient: float (valve k_v parameter)
- initial_level: float (starting level in meters)
- initial_setpoint: float (starting setpoint in meters)
- pid_gains: object containing Kc, tau_I, tau_D
- timestep: float (simulation time step in seconds)

A model for history query parameters:
- duration: optional integer (seconds of history to return, default 3600)
- Validation: must be positive and not exceed 7200 (2 hours max buffer)

Use Pydantic's Field function to add constraints:
- ge for greater-than-or-equal-to constraints
- le for less-than-or-equal-to constraints
- description strings for API documentation

All models should have clear docstrings explaining their purpose.

#### api/simulation.py specifications:

This file manages the simulation state and provides an interface for the API to interact with the tank simulator. For this task, create a minimal stub that will be expanded in later tasks.

Create a class called SimulationManager with the following structure:

The class should be designed as a singleton (only one instance exists) to manage the shared simulation state across all API requests and WebSocket connections.

For now, implement these methods as stubs:

A constructor that:
- Accepts a configuration dictionary
- Stores the configuration
- Sets initialized flag to False
- Prepares for future initialization of the tank_sim.Simulator instance

A method called initialize that:
- Creates the tank_sim.Simulator instance using the stored configuration
- Sets the initialized flag to True
- Will be called during FastAPI startup

A method called get_state that:
- Returns a dictionary with current simulation state
- For now, return dummy data matching the StateSnapshot model structure
- Will be implemented properly in the next task

A method called step that:
- Advances the simulation by one time step
- For now, just return without doing anything
- Will be implemented in the next task

A method called reset that:
- Resets the simulation to initial conditions
- For now, just set a flag
- Will be implemented in the next task

Methods for control commands (setpoint, PID, inlet flow):
- Accept the appropriate parameters
- For now, just store them in instance variables
- Will be implemented to actually call simulator methods in the next task

The purpose of this stub is to define the interface between the API and the simulation so that main.py can be written and tested without the full simulation loop running yet.

#### api/main.py specifications:

This file creates the FastAPI application, defines all endpoints, and manages the application lifecycle.

Create a FastAPI application instance with:
- Title: "Tank Dynamics Simulator API"
- Description: "Real-time tank level control simulation with PID control"
- Version: "0.1.0"

Enable CORS middleware to allow frontend connections:
- Allow origins from localhost ports 3000 and 5173 (Next.js and Vite dev servers)
- Allow credentials
- Allow all methods
- Allow all headers

Create a global SimulationManager instance that will be shared across all requests.

Define a startup event handler that:
- Initializes the SimulationManager with default configuration
- Logs that the application has started
- Will eventually start the simulation loop (in next task)

Define a shutdown event handler that:
- Logs that the application is shutting down
- Will eventually stop the simulation loop (in next task)

Create the following REST endpoints:

GET /api/health:
- Returns a simple health check response with status "ok"
- No authentication needed
- Used for monitoring and deployment health checks

GET /api/config:
- Returns the current simulation configuration
- Uses the ConfigResponse Pydantic model
- Calls SimulationManager to get configuration data

POST /api/reset:
- Resets the simulation to initial steady state
- Returns success message
- Calls SimulationManager.reset()

POST /api/setpoint:
- Accepts SetpointCommand in request body
- Updates the simulation setpoint
- Returns success message with new setpoint value
- Validates input using Pydantic model

POST /api/pid:
- Accepts PIDCommand in request body
- Updates PID controller gains
- Returns success message with new gains
- Validates input using Pydantic model

POST /api/inlet_flow:
- Accepts InletFlowCommand in request body
- Updates inlet flow rate
- Returns success message with new flow
- Validates input using Pydantic model

POST /api/inlet_mode:
- Accepts InletModeCommand in request body
- Switches inlet between constant and Brownian modes
- Returns success message with mode and parameters
- Validates input using Pydantic model

GET /api/history:
- Accepts duration query parameter (optional, default 3600 seconds)
- Returns historical data points
- For now, return empty list (will implement ring buffer in next task)
- Will eventually return list of StateSnapshot objects

WebSocket endpoint /ws:
- For now, create a basic WebSocket endpoint that accepts connections
- Log when clients connect and disconnect
- Echo back any received messages (for testing)
- Will be implemented with real-time state broadcasting in next task

Use appropriate HTTP status codes:
- 200 for successful GET/POST
- 400 for validation errors (automatic via Pydantic)
- 500 for server errors

Include error handling:
- Wrap endpoint logic in try-except blocks
- Return appropriate error responses with details
- Log errors for debugging

#### api/requirements.txt specifications:

List all dependencies needed to run the FastAPI server:

- fastapi version 0.110.0 or higher (modern async support)
- uvicorn version 0.27.0 or higher with standard extras (ASGI server)
- pydantic version 2.6.0 or higher (data validation)
- python-multipart (for form data, even though we use JSON)
- websockets version 12.0 or higher (WebSocket support)
- python-dotenv (environment variable management)
- numpy version 1.20 or higher (array handling)

For development and testing:
- pytest version 7.0 or higher
- pytest-asyncio version 0.23.0 or higher (async test support)
- httpx version 0.26.0 or higher (async HTTP client for testing)

Pin to specific versions using == to ensure reproducibility, or use >= with maximum known-good versions.

The API will use the tank_sim package installed from the parent directory, so do not list it in requirements.txt (it will be installed separately).

#### api/.env.example specifications:

Create an example environment file showing configuration options:

Include these variables with example values:
- HOST: 0.0.0.0 (bind to all interfaces)
- PORT: 8000 (default FastAPI port)
- LOG_LEVEL: info (uvicorn logging level)
- RELOAD: true (auto-reload during development, false for production)

Add comments explaining each variable and when to change them.

### Verification Strategy

After creating all files:

Test that FastAPI application starts:
- Run uvicorn api.main:app from the project root
- Application should start without errors
- Navigate to http://localhost:8000/docs to see auto-generated API documentation
- Swagger UI should display all endpoints with proper models

Test health endpoint:
- curl http://localhost:8000/api/health
- Should return JSON with status "ok"

Test config endpoint:
- curl http://localhost:8000/api/config
- Should return configuration data (even if dummy data for now)

Test WebSocket connection:
- Use a WebSocket client tool or simple Python script
- Connect to ws://localhost:8000/ws
- Send a test message
- Should receive the message echoed back

Verify API documentation:
- Open http://localhost:8000/docs
- Check that all endpoints are documented
- Verify Pydantic models show up with validation rules
- Test request/response examples in Swagger UI

Check that all imports work:
- Python should be able to import tank_sim
- All Pydantic models should validate correctly
- FastAPI should detect all route handlers

### Edge Cases and Potential Issues

Import paths:
- The api directory should be importable from the project root
- Run the server from the project root directory, not from inside api/
- Make sure PYTHONPATH includes the project root if needed

Tank_sim package availability:
- The Python bindings must be installed before running the API
- Use pip install -e . from project root to install in development mode
- Or use uv pip install -e . if using uv for environment management

CORS configuration:
- If frontend runs on a different port, add it to allowed origins
- Development typically uses localhost:3000 (Next.js) or localhost:5173 (Vite)

Port conflicts:
- Default port 8000 may already be in use
- Can override with --port flag when running uvicorn
- Or set PORT in .env file

Validation errors:
- Pydantic will automatically validate all request bodies
- Returns 422 Unprocessable Entity for validation failures
- Error messages show which fields failed and why

Async vs sync:
- FastAPI works best with async/await
- The tank_sim package is synchronous (C++ bindings)
- For now this is fine; simulation loop will run in background task

### Acceptance Criteria

- [ ] All files created in api/ directory
- [ ] FastAPI application starts without errors
- [ ] Health endpoint returns 200 OK
- [ ] Config endpoint returns valid configuration
- [ ] All POST endpoints accept and validate requests
- [ ] Pydantic models validate input correctly
- [ ] WebSocket endpoint accepts connections
- [ ] API documentation available at /docs
- [ ] requirements.txt includes all dependencies
- [ ] No import errors when running application
- [ ] Code follows FastAPI best practices

---

## Task 14: Implement Simulation Loop and WebSocket Broadcasting

**Phase:** 3 - FastAPI Backend
**Prerequisites:** Task 13 complete (FastAPI structure created)

### Files to Modify

- Modify `/home/roger/dev/tank_dynamics/api/simulation.py`
- Modify `/home/roger/dev/tank_dynamics/api/main.py`

### Requirements

This task implements the real-time simulation loop that runs at 1 Hz and broadcasts state updates to all connected WebSocket clients. This is the core functionality that makes the API a live, real-time system.

#### Background Task Architecture:

FastAPI provides background tasks that run alongside the application. The simulation loop will run as an asyncio task that:
- Starts when the application starts (in the startup event)
- Runs continuously in the background at 1 Hz
- Stops when the application shuts down (in the shutdown event)
- Broadcasts state to all connected WebSocket clients after each step

The challenge is coordinating between:
- The synchronous tank_sim.Simulator (C++ code)
- The async FastAPI WebSocket handlers
- The 1 Hz timing requirement

#### api/simulation.py modifications:

Replace the stub methods with full implementations.

Add a connections set to track active WebSocket connections:
- Use a Python set to store WebSocket connection objects
- Add connections when clients connect
- Remove connections when clients disconnect
- Thread-safe access since multiple async tasks may modify it

Implement the initialize method properly:
- Import tank_sim
- Call tank_sim.create_default_config() to get base configuration
- Create a tank_sim.Simulator instance
- Store it as an instance variable
- Set initialized flag to True

Implement the get_state method:
- Check that simulator is initialized
- Call appropriate methods on the simulator to get current state:
  - getTime() for simulation time
  - getState() returns numpy array - extract tank level (index 0)
  - getInputs() returns numpy array - extract inlet flow and valve position
  - getSetpoint() for the level setpoint
  - getError() for control error
- Calculate outlet flow using valve equation: q_out = k_v * valve_position * sqrt(tank_level)
- Return a dictionary matching the StateSnapshot Pydantic model
- Handle any exceptions and return safe default values

Implement the step method:
- Call simulator.step() to advance simulation by one time step
- This is a synchronous call to C++ code
- Will be called from the simulation loop at 1 Hz

Implement control command methods:
- set_setpoint: call simulator.setSetpoint(0, value) - 0 is the controller index
- set_pid_gains: call simulator.setControllerGains(0, gains) with a gains dictionary
- set_inlet_flow: call simulator.setInput(0, value) - 0 is the inlet flow input index
- set_inlet_mode: store mode and parameters for Brownian implementation (future task)

Implement the reset method:
- Call simulator.reset() to restore initial conditions
- Reset any internal state variables

Add a broadcast method:
- Accepts a message dictionary
- Iterates through all connected WebSockets
- Sends the message to each connection
- Removes connections that fail (client disconnected)
- Use asyncio to send messages concurrently
- Handle WebSocket errors gracefully

Add a simulation_loop coroutine:
- Runs forever in a while True loop
- Uses asyncio.sleep(1.0) to maintain 1 Hz timing
- Calls self.step() to advance simulation
- Calls self.get_state() to get current state
- Formats state as JSON message: {"type": "state", "data": {...}}
- Calls self.broadcast() to send to all clients
- Handles exceptions without crashing the loop
- Logs each iteration for debugging

#### api/main.py modifications:

Add a module-level variable to store the background task:
- simulation_task: asyncio.Task or None
- Used to track and cancel the task on shutdown

Modify the startup event handler:
- After initializing SimulationManager, create the simulation loop task
- Use asyncio.create_task(simulation_manager.simulation_loop())
- Store the task reference in simulation_task
- Log that the simulation loop has started

Modify the shutdown event handler:
- Cancel the simulation_task if it exists
- Use task.cancel() and await it
- Log that the simulation loop has stopped

Implement the WebSocket endpoint /ws properly:
- Accept the WebSocket connection with await websocket.accept()
- Add the connection to simulation_manager.connections
- Start a receive loop to handle incoming client messages
- Parse JSON messages from clients
- Route messages based on "type" field:
  - "setpoint": extract value and call set_setpoint
  - "pid": extract Kc, tau_I, tau_D and call set_pid_gains
  - "inlet_flow": extract value and call set_inlet_flow
  - "inlet_mode": extract mode and parameters, call set_inlet_mode
- Handle WebSocketDisconnect exception when client disconnects
- Remove connection from simulation_manager.connections in finally block
- Log all connections and disconnections

Add error handling:
- Catch and log any exceptions in message parsing
- Send error messages back to client on invalid commands
- Don't crash the WebSocket connection on errors

### Timing Considerations

The 1 Hz simulation loop must be accurate and consistent:

Use asyncio.sleep(1.0) for timing:
- This is sufficient for 1 Hz and won't accumulate drift
- More sophisticated timing could use time.time() to measure actual elapsed time
- For this application, simple sleep is acceptable

The simulator.step() call is synchronous (C++):
- It should complete in well under 1 second (likely microseconds)
- Running it in the async event loop is fine for this use case
- If it becomes a bottleneck, could use run_in_executor to run in thread pool

Broadcasting to WebSocket clients is async:
- Send to all clients concurrently using asyncio.gather or similar
- Don't wait for slow clients - drop messages if send fails

### WebSocket Message Format

Messages from server to clients (broadcast):
```json
{
  "type": "state",
  "data": {
    "time": 125.0,
    "tank_level": 2.5,
    "setpoint": 2.5,
    "inlet_flow": 1.0,
    "outlet_flow": 1.0,
    "valve_position": 0.5,
    "error": 0.0,
    "controller_output": 0.5
  }
}
```

Messages from clients to server:
```json
{"type": "setpoint", "value": 3.0}
{"type": "pid", "Kc": 1.5, "tau_I": 10.0, "tau_D": 0.0}
{"type": "inlet_flow", "value": 1.2}
{"type": "inlet_mode", "mode": "brownian", "min": 0.8, "max": 1.2}
```

Error messages from server to clients:
```json
{"type": "error", "message": "Invalid command format"}
```

### Verification Strategy

Test simulation loop startup:
- Start the FastAPI server
- Check logs for "Simulation loop started" message
- Verify no errors in startup

Test WebSocket connection and broadcasting:
- Use a WebSocket client (wscat, websocat, or Python script)
- Connect to ws://localhost:8000/ws
- Should immediately start receiving state messages every second
- Verify time field increments by 1 each message
- Verify state values are reasonable (level around 2.5, flows around 1.0)

Test setpoint command:
- Send setpoint change: {"type": "setpoint", "value": 3.0}
- Watch state messages - level should start increasing
- Error should change from ~0 to negative
- Valve position should change as PID responds

Test PID tuning:
- Send new PID gains: {"type": "pid", "Kc": 2.0, "tau_I": 5.0, "tau_D": 0.0}
- Make a setpoint change
- Observe different response (faster, potentially more oscillation)

Test inlet flow change:
- Send inlet flow change: {"type": "inlet_flow", "value": 0.8}
- Level should start decreasing (outlet > inlet)
- PID should open valve to compensate

Test multiple WebSocket clients:
- Connect 2-3 clients simultaneously
- All should receive the same state updates
- Commands from one client should affect state seen by all

Test client disconnect:
- Connect a client and disconnect
- Server should remove from connections list
- No errors should be logged

Test reset endpoint:
- POST to /api/reset
- Simulation should return to initial state
- Level should go back to 2.5 m
- Time should reset to 0

### Edge Cases

WebSocket connection failures:
- Client disconnects unexpectedly - should be caught by WebSocketDisconnect
- Network errors during send - should catch and remove connection
- Invalid JSON from client - should send error message, not crash

Simulation errors:
- If simulator.step() raises exception - log and continue loop
- If get_state() fails - send last known good state or default values

Concurrent access:
- Multiple WebSocket handlers may call set_setpoint etc. simultaneously
- For now this is acceptable - last command wins
- Could add locking if needed, but 1 Hz is slow enough it's unlikely to matter

Startup timing:
- Ensure simulator is initialized before first loop iteration
- Handle case where no clients are connected (don't error)

Memory management:
- Ring buffer not implemented yet (next task)
- State is only current snapshot, no history stored yet

### Acceptance Criteria

- [ ] Simulation loop runs at 1 Hz continuously
- [ ] WebSocket clients receive state updates every second
- [ ] Time field in state updates increments correctly
- [ ] Setpoint changes propagate to simulator
- [ ] PID gain changes work correctly
- [ ] Inlet flow changes work correctly
- [ ] Multiple clients can connect simultaneously
- [ ] Client disconnects handled gracefully
- [ ] Reset endpoint works correctly
- [ ] No errors in simulation loop during normal operation
- [ ] State values are physically reasonable

---

## Task 15: Implement History Ring Buffer and REST Endpoints

**Phase:** 3 - FastAPI Backend
**Prerequisites:** Task 14 complete (simulation loop and WebSocket working)

### Files to Modify

- Modify `/home/roger/dev/tank_dynamics/api/simulation.py`
- Modify `/home/roger/dev/tank_dynamics/api/main.py`

### Requirements

This task implements the historical data storage and retrieval system. The API needs to maintain a ring buffer of the last 2 hours of simulation data (approximately 7200 data points at 1 Hz) and provide REST endpoints to query this history.

#### Ring Buffer Architecture:

A ring buffer (circular buffer) is a fixed-size data structure that overwrites the oldest data when full. This is perfect for maintaining a sliding window of recent history without unbounded memory growth.

Key characteristics:
- Fixed capacity: 7200 entries (2 hours at 1 Hz)
- FIFO behavior: oldest data is automatically discarded
- Efficient: O(1) insertion, no memory allocation after initialization
- Thread-safe: needs to handle concurrent reads (from REST endpoint) and writes (from simulation loop)

Python implementation options:
- collections.deque with maxlen parameter (built-in, thread-safe for our use case)
- Custom circular buffer implementation (more control but unnecessary complexity)
- Simple list with manual wraparound (error-prone)

Recommendation: Use collections.deque with maxlen=7200 for simplicity and correctness.

#### api/simulation.py modifications:

Add ring buffer initialization:
- Import collections.deque
- In __init__ or initialize method, create a deque with maxlen=7200
- Store as instance variable: self.history
- Each entry should be a complete state snapshot (dictionary matching StateSnapshot model)

Modify the simulation_loop coroutine:
- After calling get_state(), store the result in the ring buffer
- Use self.history.append(state_dict)
- The deque will automatically discard oldest entry when at max capacity
- This happens after broadcasting to WebSocket clients

Add a get_history method:
- Accepts duration parameter (seconds of history to return)
- Default to 3600 (1 hour)
- Validate that duration is between 1 and 7200
- Calculate number of entries to return: min(duration, len(history))
- Return the last N entries from the deque as a list
- Convert deque to list using list(self.history)[-N:]
- Return in chronological order (oldest first)

Add thread safety if needed:
- For this application, deque operations are atomic enough
- If issues arise, could add a threading.Lock around append and read operations
- For 1 Hz update rate, this is unlikely to be necessary

#### api/main.py modifications:

Implement the GET /api/history endpoint properly:
- Currently returns empty list - replace with actual history query
- Extract duration query parameter (default 3600)
- Validate duration is positive and <= 7200
- Call simulation_manager.get_history(duration)
- Return the list of state snapshots
- FastAPI will automatically serialize using StateSnapshot model

The history endpoint should return JSON array of state snapshots:
```json
[
  {
    "time": 0.0,
    "tank_level": 2.5,
    "setpoint": 2.5,
    ...
  },
  {
    "time": 1.0,
    "tank_level": 2.501,
    "setpoint": 2.5,
    ...
  },
  ...
]
```

Add query parameter validation:
- Use FastAPI's Query with constraints
- duration: int = Query(default=3600, ge=1, le=7200)
- This provides automatic validation and documentation

Handle edge cases:
- If history is empty (just started): return empty list
- If requested duration exceeds available history: return all available
- If duration is invalid: FastAPI automatically returns 422 error

Update the config endpoint if needed:
- Should return actual configuration from simulation_manager
- Include ring buffer capacity: "history_capacity": 7200
- Include current history size: "history_size": len(simulation_manager.history)

### Verification Strategy

Test ring buffer accumulation:
- Start the server and let it run for 10+ seconds
- Query /api/history?duration=10
- Should return approximately 10 data points
- Verify time field increases monotonically
- Verify timestamps are accurate (first entry around time=0 if just started)

Test maximum history:
- Let server run for several minutes
- Query /api/history?duration=7200
- Should return all available data up to 7200 points
- If not running for 2 hours, returns whatever is available

Test duration parameter:
- Query with duration=60: should return ~60 points
- Query with duration=1: should return ~1 point
- Query with duration=0: should return validation error (422)
- Query with duration=10000: should return validation error (422)
- Query with no duration: should default to 3600 and return up to 1 hour

Test data consistency:
- Each entry in history should match StateSnapshot structure
- All required fields should be present
- Values should be physically reasonable
- Time values should be sequential

Test concurrent access:
- Make history requests while simulation is running
- Should not cause errors or inconsistent data
- Simulation loop continues unaffected

Test reset behavior:
- Call POST /api/reset
- History buffer should clear
- New data should accumulate from time=0
- Query history after reset should return only post-reset data

Test memory stability:
- Let server run for 3+ hours (exceeding ring buffer capacity)
- Memory usage should stabilize (not grow indefinitely)
- Buffer should contain exactly 7200 entries
- Oldest data should be discarded automatically

### Ring Buffer Memory Calculation

Each state snapshot contains approximately:
- 8 floats × 8 bytes = 64 bytes of numeric data
- Dictionary overhead: ~200 bytes
- Total per entry: ~300 bytes

Ring buffer capacity:
- 7200 entries × 300 bytes = 2.16 MB
- Negligible memory usage for modern systems

This confirms the ring buffer approach is appropriate - memory usage is bounded and small.

### Edge Cases

Empty history:
- Server just started, no data yet
- Return empty list, not an error

Partial history:
- Requested duration exceeds available history
- Return all available data, not an error
- Client can check length of returned array

Time discontinuity after reset:
- History contains data from before reset (old time) and after (restarted from 0)
- Option 1: Clear history on reset (recommended)
- Option 2: Keep old data, client deals with time jump
- Recommendation: Clear history in reset() method for consistency

Very slow clients:
- If HTTP request to /api/history takes longer than 1 second
- Ring buffer continues updating in background
- Client gets a consistent snapshot at time of request
- No locking needed due to Python GIL and atomic deque operations

### Acceptance Criteria

- [ ] Ring buffer accumulates state snapshots at 1 Hz
- [ ] Buffer capacity limited to 7200 entries
- [ ] Oldest data automatically discarded when buffer full
- [ ] GET /api/history returns correct number of entries
- [ ] Duration parameter validated correctly
- [ ] History data matches StateSnapshot model structure
- [ ] History can be queried while simulation running
- [ ] Reset clears history buffer
- [ ] Memory usage bounded after long runtime
- [ ] All history entries have sequential time values
- [ ] Default duration (3600) works correctly

---

## Upcoming Work (After Task 15)

After completing the three core FastAPI tasks, the following work remains:

### Task 16: API Testing Suite (pytest)
- Write comprehensive tests for all REST endpoints
- Test WebSocket connection and message handling
- Test simulation loop timing accuracy
- Test ring buffer behavior
- Test concurrent client scenarios
- Mock the tank_sim module for testing without C++ dependencies

### Task 17: Brownian Inlet Flow Mode (Enhancement)
- Implement random walk for inlet flow
- Add variance and bounds to Brownian parameters
- Test that Brownian mode generates realistic disturbances
- Ensure PID controller can reject Brownian disturbances

### Task 18: API Documentation and Deployment Guide
- Document all endpoints with examples
- Create API client examples in Python and JavaScript
- Write deployment guide for production (systemd, nginx, etc.)
- Document environment variables and configuration options

### Phase 4: Next.js Frontend
Once the API is complete and tested, proceed to Phase 4 to build the web UI.

---

## Notes

### Running the FastAPI Server

Development mode:
```bash
cd /home/roger/dev/tank_dynamics
uvicorn api.main:app --reload --host 0.0.0.0 --port 8000
```

Production mode:
```bash
uvicorn api.main:app --host 0.0.0.0 --port 8000 --workers 1
```

**Important:** Use only 1 worker. Multiple workers would create multiple simulation instances, which is incorrect. The simulation state must be singular.

### Testing WebSocket Connection

Using Python:
```python
import asyncio
import websockets
import json

async def test_ws():
    uri = "ws://localhost:8000/ws"
    async with websockets.connect(uri) as websocket:
        # Receive state updates
        for _ in range(10):
            message = await websocket.recv()
            data = json.loads(message)
            print(f"Time: {data['data']['time']}, Level: {data['data']['tank_level']}")
        
        # Send command
        await websocket.send(json.dumps({"type": "setpoint", "value": 3.0}))
        
        # Continue receiving
        for _ in range(10):
            message = await websocket.recv()
            data = json.loads(message)
            print(f"Time: {data['data']['time']}, Level: {data['data']['tank_level']}")

asyncio.run(test_ws())
```

Using wscat (command line tool):
```bash
npm install -g wscat
wscat -c ws://localhost:8000/ws
# Will print state updates every second
# Type: {"type": "setpoint", "value": 3.0}
# Press enter to send
```

### Understanding the 1 Hz Timing

Why 1 Hz?
- SCADA systems typically update at 0.1 to 10 Hz
- 1 Hz is sufficient for tank level control (slow process)
- Comfortable for human operators to observe
- Low bandwidth for WebSocket
- Easy to achieve without real-time OS

The simulation timestep (dt in C++) is separate from the broadcast rate:
- C++ simulation may use dt = 0.1s for numerical accuracy
- API calls simulator.step() which may advance by 1.0s (or 10 × 0.1s steps internally)
- Broadcast happens after each step, at 1 Hz
- Clients receive updates at 1 Hz regardless of internal timestep

### CORS Configuration

The CORS middleware allows browser-based frontends to connect:
- Browsers enforce same-origin policy by default
- CORS headers tell browser it's safe to allow cross-origin requests
- Development: allow localhost:3000 (Next.js)
- Production: update allowed origins to actual frontend domain

### Project Root vs API Directory

Always run the server from the project root, not from inside the api directory:

Correct:
```bash
cd /home/roger/dev/tank_dynamics
uvicorn api.main:app --reload
```

Incorrect:
```bash
cd /home/roger/dev/tank_dynamics/api
uvicorn main:app --reload  # Won't find tank_sim package!
```

The tank_sim package is installed relative to the project root, so PYTHONPATH must include that directory.

### Environment Variables

Create a `.env` file in the api directory (copy from .env.example):
```bash
cp api/.env.example api/.env
```

FastAPI with python-dotenv will automatically load these variables.

For production, set environment variables via systemd service file or docker-compose.

### Dependencies Installation

The API depends on the tank_sim package being installed:

```bash
cd /home/roger/dev/tank_dynamics

# Install tank_sim in development mode
pip install -e .

# Install API dependencies
pip install -r api/requirements.txt

# Or using uv:
uv pip install -e .
uv pip install -r api/requirements.txt
```

### Logging

FastAPI uses uvicorn's logging:
- Info level: shows each request
- Debug level: shows detailed information
- Set via --log-level flag or LOG_LEVEL env var

For simulation-specific logging, add Python logging:
```python
import logging
logger = logging.getLogger(__name__)
logger.info("Simulation loop started")
```

### Common Issues

**ImportError: tank_sim not found**
- Make sure tank_sim is installed: `pip install -e .`
- Make sure running from project root, not api directory

**WebSocket connection refused**
- Server not running: start with uvicorn
- CORS issue: check allowed origins
- Firewall blocking port 8000

**Simulation loop not running**
- Check startup event fired: look for log message
- Check for exceptions in background task
- Use `asyncio.create_task()` not `asyncio.run()` in startup event

**Memory leak**
- Ring buffer should be fixed size (7200 entries)
- Check that WebSocket connections are properly removed on disconnect
- Monitor with `ps aux | grep uvicorn`

**Timing drift**
- asyncio.sleep(1.0) should be sufficient
- If drift occurs, measure actual elapsed time and adjust sleep
- For 1 Hz, drift should be negligible over hours
