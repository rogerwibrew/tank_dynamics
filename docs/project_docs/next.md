# Next Tasks - Tank Dynamics Simulator

## Current Phase: Phase 3 - FastAPI Backend (Completion Tasks)

Phase 3 core implementation is complete (Tasks 13-15). The following tasks complete the backend by adding comprehensive testing, implementing the Brownian inlet mode enhancement, and finalizing documentation before proceeding to Phase 4 frontend development.

---

## Task 16: Comprehensive API Testing Suite

**Phase:** 3 - FastAPI Backend  
**Prerequisites:** Tasks 13-15 (all Phase 3 core implementation complete)

### Files to Create

- Create `api/tests/__init__.py` (empty file for test discovery)
- Create `api/tests/test_endpoints.py` (REST endpoint tests)
- Create `api/tests/test_websocket.py` (WebSocket protocol tests)
- Create `api/tests/test_concurrent.py` (concurrent client tests)
- Create `api/tests/conftest.py` (pytest fixtures and configuration)

### Requirements

The testing suite should provide comprehensive coverage of all API functionality, ensuring the backend behaves correctly under normal operation, edge cases, and error conditions.

#### Testing Architecture Principles:

**Test Isolation:** Each test should be independent and not rely on state from other tests. Use pytest fixtures to set up clean state before each test.

**Mock Strategy:** Tests should mock the tank_sim C++ module to avoid dependencies on the compiled binary. This allows tests to run in CI/CD environments without building C++, and allows testing specific simulation behaviors by controlling mock return values.

**Async Testing:** All FastAPI tests must be async and use appropriate test clients (httpx.AsyncClient for REST, WebSocket test client for WebSocket tests).

**Coverage Goals:** Aim for high coverage of API layer code, including normal paths, error paths, validation failures, and edge cases.

#### api/tests/conftest.py specifications:

This file should provide shared pytest fixtures used across all test files.

**Fixture: mock_tank_sim** - Mocks the tank_sim module before any imports occur. Returns a mock module with controllable behavior for TankSimulator class. Should use unittest.mock.MagicMock to create a fake tank_sim module. This fixture should have autouse=True and scope="session" to ensure it's applied before any API imports.

**Fixture: app** - Returns the FastAPI app instance with mocked tank_sim. This ensures each test gets a fresh app without real C++ dependencies.

**Fixture: client** - Returns an httpx.AsyncClient configured for testing the app. Should set base_url to "http://test" and follow_redirects to True.

**Fixture: simulation_state** - Provides a dictionary representing a typical simulation state with all expected fields (time, tank_level, setpoint, inlet_flow, outlet_flow, valve_position, error, controller_output). Tests can override specific values as needed.

**Fixture: default_config** - Provides a dictionary representing the simulation configuration returned by GET /api/config.

#### api/tests/test_endpoints.py specifications:

This file should test all REST endpoints with various scenarios.

**Test: test_health_endpoint** - Verify GET /health returns status 200 and contains "status": "healthy".

**Test: test_get_state** - Verify GET /api/state returns the current simulation state with all required fields. Check that response structure matches StateResponse Pydantic model.

**Test: test_get_config** - Verify GET /api/config returns configuration with all required fields (tank_area, tank_height, valve_coefficient, initial conditions, PID gains). Check that numeric values are reasonable.

**Test: test_get_history_default** - Verify GET /api/history without parameters returns recent history data. Check that response is a list of state entries with time, tank_level, and setpoint fields.

**Test: test_get_history_with_duration** - Verify GET /api/history?duration=60 returns history filtered to last 60 seconds. Check that returned entries span the requested duration.

**Test: test_get_history_validation** - Verify that invalid duration values (negative, zero, non-numeric) return appropriate 422 validation errors with helpful messages.

**Test: test_post_setpoint_valid** - Verify POST /api/setpoint with valid value (within 0 to max_height range) succeeds and returns confirmation message.

**Test: test_post_setpoint_out_of_range** - Verify POST /api/setpoint with value above max_height or below 0 returns 422 validation error.

**Test: test_post_inlet_flow_valid** - Verify POST /api/inlet-flow with valid positive flow value succeeds.

**Test: test_post_inlet_flow_negative** - Verify POST /api/inlet-flow with negative value returns 422 validation error.

**Test: test_post_pid_gains_valid** - Verify POST /api/pid-gains with valid PID parameters (Kc > 0, tau_I >= 0, tau_D >= 0) succeeds.

**Test: test_post_pid_gains_invalid** - Verify POST /api/pid-gains with invalid values (negative Kc, negative tau_I, negative tau_D) returns 422 validation error.

**Test: test_post_reset** - Verify POST /api/reset succeeds and returns confirmation. After reset, verify that GET /api/state returns initial conditions and GET /api/history returns empty or very short history.

**Test: test_cors_headers** - Verify that responses include appropriate CORS headers (Access-Control-Allow-Origin, Access-Control-Allow-Methods, etc.). Send OPTIONS preflight request and verify response.

**Test: test_404_unknown_endpoint** - Verify that requests to non-existent endpoints return 404 with appropriate error format.

**Test: test_json_parse_error** - Verify that POST requests with malformed JSON return 422 or 400 error with helpful message.

#### api/tests/test_websocket.py specifications:

This file should test WebSocket connection lifecycle and message handling.

**Test: test_websocket_connection** - Verify that a client can connect to /ws endpoint successfully. Check that connection is accepted and remains open.

**Test: test_websocket_receives_state_updates** - Connect to WebSocket and verify that state update messages arrive periodically. Check that message format matches expected structure with "type": "state" and "data" containing all state fields. Wait for at least 2 messages to verify continuous streaming.

**Test: test_websocket_setpoint_command** - Connect to WebSocket, send a setpoint command message (type: "setpoint", value: 3.0), and verify acknowledgment or state change. Check that no error is returned.

**Test: test_websocket_pid_command** - Send a PID gains command via WebSocket with valid parameters and verify acceptance.

**Test: test_websocket_inlet_flow_command** - Send an inlet flow command via WebSocket and verify acceptance.

**Test: test_websocket_invalid_json** - Send malformed JSON over WebSocket and verify that connection remains open and an error message is returned (not a connection close).

**Test: test_websocket_missing_fields** - Send a command with missing required fields (e.g., setpoint command without "value") and verify appropriate error message is returned.

**Test: test_websocket_invalid_command_type** - Send a message with unknown "type" field and verify error message indicating unknown command type.

**Test: test_websocket_multiple_clients** - Open multiple WebSocket connections simultaneously and verify all receive state updates. Close one connection and verify others continue receiving updates.

**Test: test_websocket_disconnect_cleanup** - Open a WebSocket connection, then close it, and verify that server properly removes it from the active connections set. Check that server logs indicate connection removed (if accessible).

#### api/tests/test_concurrent.py specifications:

This file should test behavior under concurrent access patterns.

**Test: test_concurrent_rest_requests** - Make multiple simultaneous REST requests (GET /api/state) using asyncio.gather and verify all succeed with consistent results.

**Test: test_concurrent_websocket_clients** - Open 10 WebSocket connections concurrently and verify all receive state updates simultaneously. Check that broadcasts go to all clients.

**Test: test_mixed_concurrent_operations** - Simultaneously perform REST GET requests, REST POST requests, and WebSocket connections. Verify no race conditions or errors occur.

**Test: test_history_query_during_updates** - Query GET /api/history repeatedly while simulation loop is adding new entries. Verify no corruption or exceptions occur due to concurrent access to the ring buffer.

**Test: test_setpoint_changes_rapid_succession** - Send multiple setpoint change commands in rapid succession (faster than 1 Hz simulation rate) and verify all are accepted and last one wins. Check for any rate limiting or error handling.

**Test: test_reset_during_active_connections** - Have multiple active WebSocket connections, then POST /api/reset, and verify that all clients receive subsequent state updates reflecting the reset state.

### Mock Implementation Strategy

The mock for tank_sim should behave as follows:

**TankSimulator constructor:** Accept config parameter but don't validate it. Store config for later inspection if needed.

**step() method:** Do nothing (no-op). Alternatively, increment an internal time counter if tests need to verify step was called.

**get_state() method:** Return a dictionary with realistic default values that tests can inspect. Allow tests to override return values via fixture configuration.

**set_setpoint() method:** Accept value parameter, no-op or store for inspection.

**set_inlet_flow() method:** Accept value parameter, no-op or store for inspection.

**set_pid_gains() method:** Accept Kc, tau_I, tau_D parameters, no-op or store for inspection.

**reset() method:** No-op or reset internal state to defaults.

This allows tests to verify that the API correctly calls the simulation methods without requiring the actual C++ implementation.

### Verification Strategy

Run the test suite with pytest and verify:

1. **All tests pass:** Run `pytest api/tests/ -v` and confirm all tests pass.

2. **Coverage report:** Generate coverage report with `pytest api/tests/ --cov=api --cov-report=term-missing` and verify coverage of main.py, simulation.py, and models.py is above 80%.

3. **Test isolation:** Run tests in random order using `pytest --random-order` to verify no test depends on another test's state.

4. **Fast execution:** Tests should complete in under 30 seconds since they mock the C++ layer.

5. **CI/CD ready:** Tests should run successfully in a clean environment without needing the C++ tank_sim library installed.

### Edge Cases

**Timing edge cases:** Tests that wait for WebSocket messages must use appropriate timeouts. Too short causes flaky tests, too long slows execution. Use 2-3 second timeouts for WebSocket receive operations.

**AsyncIO event loop issues:** Each test function must properly manage the asyncio event loop. Use pytest-asyncio markers (@pytest.mark.asyncio) for all async test functions.

**Fixture cleanup:** Ensure that httpx.AsyncClient instances are properly closed after tests using async context managers or proper cleanup.

**Mock patching timing:** The tank_sim mock must be applied before the api modules are imported. Use conftest.py session-scoped autouse fixture to ensure correct ordering.

**WebSocket client compatibility:** Use starlette.testclient.TestClient or httpx WebSocket support for testing. Be aware of differences in WebSocket test client APIs.

### Acceptance Criteria

- [ ] All 5 test files created in api/tests/ directory
- [ ] conftest.py provides reusable fixtures including mock_tank_sim
- [ ] test_endpoints.py contains at least 15 REST endpoint tests covering normal and error cases
- [ ] test_websocket.py contains at least 9 WebSocket tests covering connection, messages, and commands
- [ ] test_concurrent.py contains at least 6 concurrency tests
- [ ] All tests pass with `pytest api/tests/ -v`
- [ ] Test coverage of API layer exceeds 80%
- [ ] Tests run in under 30 seconds
- [ ] Tests do not require C++ tank_sim library to be installed
- [ ] No test failures when run with --random-order

---

## Task 17: Implement Brownian Inlet Flow Mode

**Phase:** 3 - FastAPI Backend  
**Prerequisites:** Task 16 (testing infrastructure in place)

### Files to Modify

- Modify `api/simulation.py` (add Brownian random walk logic)
- Modify `api/main.py` (no changes needed, already has inlet_mode endpoint)
- Create `api/tests/test_brownian.py` (tests for Brownian mode)

### Requirements

The Brownian inlet flow mode should generate realistic random disturbances to the inlet flow rate, simulating real-world process variability. This allows users to test how well the PID controller rejects disturbances and maintains level control.

#### Brownian Motion Background:

A Brownian random walk is a stochastic process where the value changes by small random steps over time. For inlet flow:

- Start at current inlet_flow value
- Each simulation step (1 Hz), add a random increment drawn from a normal distribution
- The increment has mean 0 and variance σ² (configurable)
- Clamp the resulting flow to [min, max] bounds to prevent unrealistic extremes

This creates a smooth, wandering inlet flow that mimics real disturbances like pump variability, upstream process changes, or measurement noise.

#### api/simulation.py modifications:

**In the SimulationManager class:**

Add a method called apply_brownian_inlet that computes the next inlet flow value using Brownian motion. This method should:

- Accept the current inlet flow value as a parameter
- Generate a random increment using numpy.random.normal with mean 0 and standard deviation based on inlet_mode_params["variance"]
- Add this increment to the current inlet flow
- Clamp the result to [inlet_mode_params["min"], inlet_mode_params["max"]]
- Return the new inlet flow value

**In the step method:**

Before calling self.simulator.step(), check if inlet_mode is "brownian". If so:

- Get the current inlet flow from self.simulator.get_inputs()[0]
- Call apply_brownian_inlet to compute the new inlet flow
- Set the new inlet flow using self.simulator.set_input(0, new_flow)

This ensures the inlet flow changes organically with each simulation step while the PID controller tries to compensate.

**Variance parameter interpretation:**

The variance parameter should be interpreted as the standard deviation of the step increment, not the variance of the final distribution. Typical useful values:

- Small disturbances: variance = 0.01 to 0.05 m³/s per step
- Moderate disturbances: variance = 0.05 to 0.1 m³/s per step
- Large disturbances: variance = 0.1 to 0.2 m³/s per step

The cumulative effect over many steps creates a wandering behavior. Too large a variance causes unrealistic jumps; too small is barely perceptible.

**Clamping behavior:**

When the random walk reaches a boundary (min or max), it should reflect rather than stick. Alternatively, simply clamp to bounds and allow the next random step to potentially move it away from the boundary. The simple clamp approach is sufficient and easier to implement.

**Seed control (optional but recommended):**

For reproducible testing, consider accepting an optional random seed parameter. This allows tests to verify expected behavior with a known random sequence.

#### WebSocket/REST interface (already implemented):

The inlet_mode endpoint is already implemented in Task 14. No changes needed to main.py. The command format is:

```json
{
  "type": "inlet_mode",
  "mode": "brownian",
  "min": 0.8,
  "max": 1.2,
  "variance": 0.05
}
```

To disable Brownian mode and return to constant inlet flow:

```json
{
  "type": "inlet_mode",
  "mode": "constant"
}
```

#### api/tests/test_brownian.py specifications:

This file should test the Brownian inlet flow implementation.

**Test: test_brownian_mode_changes_inlet_flow** - Set inlet mode to Brownian with reasonable parameters (min=0.8, max=1.2, variance=0.05). Run simulation for 10 steps. Verify that inlet flow changes between steps (not constant). Verify that inlet flow stays within bounds.

**Test: test_brownian_mode_respects_bounds** - Set inlet mode to Brownian with tight bounds (min=0.95, max=1.05) and high variance (variance=0.5). Run simulation for 50 steps. Verify that inlet flow NEVER exceeds min or max bounds despite high variance.

**Test: test_brownian_mode_mean_reversion** - Set inlet mode to Brownian centered at 1.0 with symmetric bounds (min=0.5, max=1.5). Run simulation for 1000 steps and collect all inlet flow values. Compute the mean of collected values. Verify that the mean is approximately 1.0 (within ±0.2), demonstrating that the random walk is unbiased.

**Test: test_brownian_variance_effect** - Run two simulations: one with low variance (0.01) and one with high variance (0.2). Run each for 100 steps. Compute the standard deviation of inlet flow values for each. Verify that high variance simulation has higher standard deviation than low variance simulation.

**Test: test_constant_mode_disables_brownian** - Set inlet mode to Brownian, run 5 steps to establish random walk, then switch back to constant mode. Verify that inlet flow stops changing after switching to constant mode.

**Test: test_brownian_with_seed_reproducible** - If seed parameter is implemented, verify that two simulations with the same seed produce identical inlet flow sequences.

**Test: test_brownian_mode_parameter_validation** - Verify that invalid Brownian parameters (min > max, negative variance, etc.) are rejected with appropriate validation errors.

**Test: test_pid_rejects_brownian_disturbances** - Set inlet mode to Brownian with moderate disturbances (variance=0.05). Run simulation for 300 steps (5 minutes). Verify that despite inlet flow changes, the tank level remains close to setpoint (error less than 0.5 m for majority of time). This demonstrates that the PID controller successfully rejects disturbances.

### Implementation Notes

**Import numpy:** Add `import numpy as np` at the top of api/simulation.py if not already present.

**Random seed management:** If implementing seed control, store the numpy random generator state in SimulationManager. Use `self.rng = np.random.default_rng(seed)` in __init__ and `self.rng.normal()` for generation.

**Performance considerations:** Generating a single random number per second (1 Hz) has negligible performance impact. No optimization needed.

**Thread safety:** The Brownian update happens in the simulation loop (single thread), so no locking required.

**Logging:** Consider logging when Brownian mode is enabled/disabled and when parameters change. Use INFO level: "Brownian inlet mode enabled: min=0.8, max=1.2, variance=0.05"

### Verification Strategy

1. **Visual verification:** Run the FastAPI server, enable Brownian mode via WebSocket, and watch the inlet flow in GET /api/state responses. It should wander smoothly within bounds.

2. **Unit tests pass:** Run `pytest api/tests/test_brownian.py -v` and verify all tests pass.

3. **Integration test:** Connect a WebSocket client, enable Brownian mode, and observe state messages. Verify that inlet_flow field changes over time while tank_level remains relatively stable.

4. **Parameter sweep:** Test with various variance values (0.01, 0.05, 0.1, 0.2) and observe behavior. Small variance should produce gentle wandering; large variance should produce more aggressive disturbances.

5. **Controller stability:** Verify that the PID controller remains stable under Brownian disturbances. Level should not oscillate wildly or drift far from setpoint.

### Edge Cases

**Boundary clipping frequency:** With very high variance and tight bounds, the inlet flow may hit boundaries frequently. Verify this doesn't cause numerical issues or unexpected behavior.

**Zero variance:** If variance=0, Brownian mode should behave identically to constant mode (no changes). Consider treating this as equivalent to constant mode.

**Very large variance:** With variance much larger than (max - min)/2, nearly every step will hit a boundary. This is valid but produces unrealistic "bang-bang" behavior. Consider documenting recommended variance ranges.

**Mode switching mid-simulation:** Switching from constant to Brownian should start the random walk from the current inlet flow value (no discontinuity). Switching from Brownian to constant should freeze inlet flow at the current value.

**Reset behavior:** When simulation is reset, inlet_mode should reset to constant, and inlet flow should return to initial conditions. Brownian parameters should be cleared.

### Acceptance Criteria

- [ ] apply_brownian_inlet method implemented in SimulationManager
- [ ] Brownian logic integrated into SimulationManager.step() method
- [ ] Inlet flow changes smoothly over time when Brownian mode enabled
- [ ] Inlet flow always stays within [min, max] bounds
- [ ] Switching between constant and Brownian modes works correctly
- [ ] Test file test_brownian.py created with at least 8 tests
- [ ] All Brownian tests pass
- [ ] PID controller successfully rejects Brownian disturbances (level stays near setpoint)
- [ ] Logging added for Brownian mode enable/disable events
- [ ] Documentation updated with Brownian mode usage examples

---

## Task 18: API Documentation and Production Deployment Guide

**Phase:** 3 - FastAPI Backend  
**Prerequisites:** Tasks 16-17 (testing and features complete)

### Files to Create/Modify

- Create `api/README.md` (API-specific documentation)
- Create `docs/API_REFERENCE.md` (comprehensive endpoint documentation)
- Create `docs/DEPLOYMENT.md` (production deployment guide)
- Create `examples/websocket_client.py` (Python WebSocket client example)
- Create `examples/websocket_client.html` (JavaScript WebSocket client example)
- Create `examples/rest_client.py` (Python REST API client example)
- Modify main `README.md` (update with Phase 3 completion status)

### Requirements

The documentation should provide everything a user or developer needs to understand, deploy, and use the API. This includes complete endpoint references, usage examples, deployment procedures, and troubleshooting guidance.

#### docs/API_REFERENCE.md specifications:

This document should provide comprehensive reference for all API endpoints.

**Structure:**

1. **Overview section** - Brief description of the API, base URL, authentication status (none currently), CORS configuration.

2. **REST Endpoints section** - For each endpoint, document:
   - HTTP method and path
   - Purpose and description
   - Request parameters (path, query, body) with types and constraints
   - Request body schema (JSON structure with field descriptions)
   - Success response (status code, body schema, example)
   - Error responses (possible status codes, error format, examples)
   - Example curl commands
   - Example responses

3. **WebSocket Endpoint section** - Document:
   - WebSocket URL path (/ws)
   - Connection procedure
   - Message format for server-to-client messages (state updates)
   - Message format for client-to-server commands (setpoint, PID, inlet_flow, inlet_mode)
   - Error message format
   - Connection lifecycle (connect, receive, send, disconnect)
   - Example message sequences

4. **Data Models section** - Define all Pydantic models with field descriptions, types, constraints, and example JSON.

5. **Error Handling section** - Explain error response format, common error codes, validation error structure.

6. **Rate Limits section** - Document any rate limiting (currently none, but mention this explicitly).

**Endpoints to document:**

- GET /health - Health check
- GET /api/state - Get current simulation state
- GET /api/config - Get simulation configuration
- GET /api/history - Get historical data with duration parameter
- POST /api/setpoint - Change setpoint
- POST /api/pid-gains - Update PID gains
- POST /api/inlet-flow - Set inlet flow rate
- POST /api/reset - Reset simulation
- WS /ws - WebSocket for real-time updates and commands

For each endpoint, provide at least one complete example showing request and response.

#### docs/DEPLOYMENT.md specifications:

This document should guide users through deploying the API in production.

**Structure:**

1. **Prerequisites section** - List requirements:
   - Python version (3.10+)
   - System packages needed (build tools for compiling pybind11 module)
   - C++ dependencies (Eigen, GSL, pybind11)
   - Network requirements (ports, firewall rules)

2. **Installation section** - Step-by-step instructions:
   - Clone repository
   - Build C++ simulation library
   - Install Python package (tank_sim)
   - Install API dependencies (pip install -r api/requirements.txt)
   - Verify installation with test commands

3. **Configuration section** - Explain:
   - Environment variables (.env file)
   - CORS configuration for production (update allowed origins)
   - Logging configuration
   - Port selection

4. **Running in Development section** - Explain uvicorn development server with --reload flag, appropriate for local testing.

5. **Running in Production section** - Explain:
   - Use uvicorn without --reload
   - MUST use --workers 1 (single worker for singleton simulation)
   - Bind to appropriate host (0.0.0.0 or specific IP)
   - Use HTTPS in production (TLS certificate setup)
   - Reverse proxy setup (nginx or Apache) with WebSocket support
   - Keep-alive configuration for WebSocket connections

6. **Systemd Service section** - Provide a complete systemd service file example:
   - Service definition with proper WorkingDirectory
   - Automatic restart on failure
   - Logging to journalctl
   - User/group configuration
   - Environment file integration
   - Enable and start commands

7. **Nginx Reverse Proxy section** - Provide nginx configuration example:
   - Proxy WebSocket connections (requires upgrade headers)
   - Proxy HTTP REST endpoints
   - TLS/SSL configuration
   - Appropriate timeouts for long-lived WebSocket connections
   - CORS headers if not handled by FastAPI

8. **Monitoring section** - Explain:
   - Health check endpoint for monitoring (GET /health)
   - Log locations and formats
   - Systemd journal inspection
   - Suggested monitoring tools (Prometheus, Grafana)

9. **Troubleshooting section** - Common issues and solutions:
   - Port already in use
   - WebSocket connection failures
   - CORS errors
   - Module import errors (tank_sim not found)
   - Permission issues
   - Multiple workers issue (simulation state divergence)

10. **Security Considerations section** - Discuss:
    - No authentication currently (suitable for trusted networks only)
    - Adding authentication if needed (OAuth2, API keys)
    - HTTPS/TLS requirement for production
    - CORS configuration for public deployment
    - Firewall rules
    - Input validation (already handled by Pydantic)

#### api/README.md specifications:

This file should provide a quick start guide for the API specifically.

**Contents:**

- Brief description of the FastAPI backend
- Quick start instructions (how to run locally)
- Link to comprehensive API_REFERENCE.md
- Link to DEPLOYMENT.md for production setup
- Directory structure explanation
- Testing instructions (pytest)
- Development tips (--reload, debugging)

#### examples/websocket_client.py specifications:

A complete, runnable Python script demonstrating WebSocket usage.

**Features:**

- Connect to ws://localhost:8000/ws
- Receive and print state updates
- Send a setpoint command after 5 seconds
- Send a PID gains command after 10 seconds
- Handle connection errors gracefully
- Use asyncio and websockets library
- Include comments explaining each step

**Usage:** `python examples/websocket_client.py`

#### examples/websocket_client.html specifications:

A complete, self-contained HTML file with JavaScript demonstrating WebSocket usage in the browser.

**Features:**

- Connect to WebSocket using browser WebSocket API
- Display received state updates in a table or formatted div
- Provide input fields for sending commands (setpoint, inlet_flow)
- Buttons to trigger commands
- Connection status indicator
- Error handling and display
- Works by opening the file directly in a browser (no server needed)

**Usage:** Open file in web browser, works with API running on localhost:8000.

#### examples/rest_client.py specifications:

A complete, runnable Python script demonstrating REST API usage.

**Features:**

- Use requests library for HTTP calls
- Demonstrate each REST endpoint with examples
- GET /api/state and print current state
- GET /api/config and print configuration
- GET /api/history?duration=60 and print summary of history
- POST /api/setpoint with new value
- POST /api/inlet-flow with new value
- POST /api/reset
- Error handling with try-except blocks
- Comments explaining each API call

**Usage:** `python examples/rest_client.py`

#### Main README.md modifications:

Update the main project README to reflect Phase 3 completion:

**In Project Status section:**

- Mark Phase 3 as ✅ COMPLETE
- Update "Current Phase" to indicate Phase 3 complete, Phase 4 upcoming
- Add completion date for Phase 3

**In Quick Start section:**

- Add instructions for running the FastAPI server
- Add link to API_REFERENCE.md for API details
- Add link to examples/ directory

**In Testing section:**

- Add instructions for running API tests (pytest api/tests/)
- Document test coverage results

### Verification Strategy

1. **Documentation completeness:** Review each document and verify that all sections are complete and accurate. No placeholder text (e.g., "TODO") should remain.

2. **Example functionality:** Run each example script and HTML file against a running API server. Verify they work without modifications:
   - `python examples/websocket_client.py` - connects and receives updates
   - `python examples/rest_client.py` - successfully calls all REST endpoints
   - Open `examples/websocket_client.html` in browser - UI works and can send commands

3. **Deployment procedure validation:** Follow DEPLOYMENT.md step-by-step on a clean system (or VM) and verify that the API can be deployed successfully following only the documentation.

4. **Systemd service test:** Install the systemd service file and verify:
   - Service starts successfully with `systemctl start tank-sim-api`
   - Service restarts automatically if killed
   - Logs appear in journalctl
   - Service starts on boot after `systemctl enable tank-sim-api`

5. **Nginx reverse proxy test:** Set up nginx with provided configuration and verify:
   - REST endpoints accessible through nginx
   - WebSocket connections work through nginx
   - TLS/SSL works if configured

6. **Link validation:** Check that all internal documentation links work (no broken references between documents).

7. **API reference accuracy:** For each endpoint documented in API_REFERENCE.md, make a test request and verify the response matches the documentation.

### Acceptance Criteria

- [ ] API_REFERENCE.md created with complete documentation of all endpoints
- [ ] DEPLOYMENT.md created with step-by-step production deployment guide
- [ ] api/README.md created with quick start instructions
- [ ] examples/websocket_client.py created and tested
- [ ] examples/websocket_client.html created and tested in browser
- [ ] examples/rest_client.py created and tested
- [ ] Systemd service file example provided in DEPLOYMENT.md
- [ ] Nginx configuration example provided in DEPLOYMENT.md
- [ ] Main README.md updated with Phase 3 completion status
- [ ] All example scripts run successfully against running API
- [ ] Documentation reviewed and contains no placeholders or TODOs
- [ ] At least one developer successfully deploys API following only DEPLOYMENT.md
- [ ] All links between documentation files are valid

---

## Upcoming Work (After Task 18)

After completing Tasks 16-18, Phase 3 (FastAPI Backend) will be fully complete. The next phase is:

### Phase 4: Next.js Frontend

**Goals:** Build the SCADA-style web UI with real-time process visualization and trend charts.

**Initial tasks will include:**

- Next.js project setup with App Router
- Tailwind CSS configuration with dark theme
- WebSocket connection hook
- Basic layout with tab navigation (Process View / Trends View)
- Tank visualization component
- Real-time data integration
- Trend charting with Recharts
- Control panel for setpoint, PID, and inlet flow

The frontend will be developed in subsequent sprints after Phase 3 is fully complete and merged to main.

---

## Notes on Phase 3 Testing

### Running the Full Test Suite

Run all API tests:
```bash
cd /home/roger/dev/tank_dynamics
pytest api/tests/ -v
```

Run with coverage:
```bash
pytest api/tests/ --cov=api --cov-report=html --cov-report=term
```

View coverage report:
```bash
xdg-open htmlcov/index.html
```

Run tests in random order (check for test interdependencies):
```bash
pytest api/tests/ --random-order
```

### Test Dependencies

The test suite requires:
```bash
pip install pytest pytest-asyncio pytest-cov pytest-random-order httpx
```

These should be added to a `api/requirements-dev.txt` file for development dependencies.

### Mocking Strategy Rationale

The tests mock tank_sim rather than using the real C++ library for several important reasons:

1. **CI/CD simplicity** - Tests can run in any Python environment without compiling C++
2. **Speed** - Mock tests run in seconds rather than minutes
3. **Isolation** - API layer can be tested independently of simulation layer
4. **Controlled behavior** - Tests can simulate specific edge cases by controlling mock return values
5. **Cross-platform** - Tests work on any OS without C++ build chain

The C++ simulation layer has its own comprehensive test suite (42 tests in Phase 1), so we don't need to retest the physics in the API tests. The API tests focus on verifying that the API layer correctly:
- Parses requests
- Validates inputs
- Calls the appropriate simulation methods
- Formats responses
- Handles errors

This separation of concerns is good testing practice.

### Brownian Mode Testing Strategy

Testing Brownian mode requires statistical validation:

- **Deterministic tests** - Use a fixed random seed to get reproducible behavior for regression testing
- **Statistical tests** - Verify properties like mean, variance, and bounds over many samples
- **Visual inspection** - Run the API and observe Brownian behavior manually to ensure it "looks right"

The tests should verify correctness (stays in bounds, unbiased random walk) without being overly brittle to specific random sequences.

### Documentation Review Process

Before considering Task 18 complete, conduct a documentation review:

1. **Completeness check** - Every endpoint, parameter, and feature documented
2. **Accuracy check** - Verify examples work and responses match reality
3. **Clarity check** - Can a new developer understand and use the API from docs alone?
4. **Link check** - All references and links work
5. **Example check** - All example code runs successfully

Consider asking someone unfamiliar with the project to attempt using the API following only the documentation as a validation test.

---

*Tasks written: 2026-02-09*  
*Senior Engineer: Claude Sonnet*  
*Branch: phase4-nextjs-frontend*
