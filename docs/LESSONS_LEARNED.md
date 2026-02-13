# Lessons Learned - Tank Dynamics Simulator

**Project:** Tank Dynamics Simulator (Proof of Concept)  
**Date:** 2026-02-09  
**Purpose:** Document key insights for scaling to larger, more complex simulations

---

## Executive Summary

This document captures lessons learned from building a small proof-of-concept simulation system combining C++ physics engines with Python/FastAPI backends. These insights are critical for scaling to larger, multi-physics simulations with hundreds or thousands of state variables.

**Key Takeaway:** Small architectural decisions made early have massive implications at scale. Test integration patterns early with mocks to avoid costly refactoring later.

**New Key Takeaway (2026-02-10):** Task granularity matters enormously when working with local LLMs or smaller models. Tasks that seem "simple" to Sonnet/Opus can be insurmountably complex for Haiku or local models. Breaking work into micro-tasks (1-2 files, 15-30 minutes) dramatically improves success rate.

**Critical Key Takeaway (2026-02-10):** LLMs working from memory (knowledge cutoff) will produce outdated task specifications for rapidly evolving frameworks. Always query authoritative documentation sources (Context7, official docs) when creating tasks involving external frameworks, especially for project initialization and configuration steps.

---

## 0. Framework Documentation: Query, Don't Assume

### The Problem We Encountered

**What happened:** Phase 4 tasks specified expected output for `create-next-app` based on LLM training data:
- Expected: `tailwind.config.ts` created by default
- Expected: `eslintrc.json` configuration format
- Expected: `next.config.js` (JavaScript config)

**Actual behavior (Next.js v16 + Tailwind v4):**
- Tailwind CSS **not installed by default** by create-next-app
- ESLint uses new flat config format (`eslint.config.mjs`)
- Next.js uses TypeScript config by default (`next.config.ts`)

**Impact:**
- Engineer followed verification steps and found mismatches
- Had to escalate back to Senior Engineer for corrections
- Wasted time debugging "why doesn't my setup match docs?"
- Required complete task rewrite for 6+ tasks

**Root cause:** Senior Engineer (Sonnet) relied on training data instead of querying current documentation via Context7 MCP.

### The Lesson

**For framework-dependent tasks:**

1. **Always query documentation first** before writing tasks
   - Use Context7 MCP for framework docs (Next.js, React, Tailwind, etc.)
   - Check actual version being used
   - Verify project structure, configuration files, CLI flags

2. **Especially critical for:**
   - Project initialization (`create-*` commands)
   - Configuration file formats and locations
   - Default dependencies and what's included out-of-box
   - Breaking changes between major versions
   - Installation and setup procedures

3. **High-risk frameworks** (update frequently):
   - Next.js (App Router vs Pages Router, Turbopack, etc.)
   - Tailwind CSS (v3 vs v4 configuration changes)
   - React (Compiler, Server Components, etc.)
   - Vite, Vue, Angular, etc.

4. **Verification commands must match reality:**
   - Don't specify "you should see X files" unless verified
   - Don't assume defaults haven't changed
   - Don't trust memory for configuration syntax

### Specific Recommendations

**When writing tasks involving external frameworks:**

```markdown
## Task Xa: Initialize [Framework] Project

**BEFORE writing this task, Senior Engineer must:**
1. Query Context7: "[Framework] latest version installation setup"
2. Query Context7: "[Framework] create command default project structure"
3. Query Context7: "[Framework] configuration files and format"
4. Verify actual CLI flags and their current behavior
5. Document the framework version number in task

**In the task specification:**
- State exact framework version (e.g., "Next.js 16.1.5")
- List ONLY files that are actually created
- Note what is NOT included by default
- Provide actual verification command output (if available)
```

**Example query sequence for Next.js project:**
```
1. resolve-library-id: "next.js"
2. query-docs: "create-next-app default files generated structure"
3. query-docs: "next.js latest version configuration format"
4. query-docs: "tailwind css installation with next.js"
```

**Example: Recharts dependency version mismatch**

During Phase 4 implementation, Task 19b originally specified "recharts (2.x)" but `npm install recharts` installed v3.7.0 by default. This caused a secondary issue:

**Problem discovered:**
- Task specified: "recharts version 2.x"
- Actual installed: recharts 3.7.0
- Breaking changes: CartesianGrid requires explicit `xAxisId`/`yAxisId` props in v3

**Resolution:**
1. Queried Context7: `/recharts/recharts` library documentation
2. Reviewed [Recharts 3.0 migration guide](https://github.com/recharts/recharts/wiki/3.0-migration-guide)
3. Updated Task 19b to specify "recharts (^3.7.0)"
4. Added migration notes to Task 25 (future Recharts usage)
5. Documented breaking changes: CartesianGrid axis IDs, new hooks, state management rewrite

**Lesson:** Always query documentation for **dependency versions** too, not just project initialization. Even within a single framework ecosystem, major version changes can introduce breaking changes that affect task specifications.

**For the Engineer executing tasks:**

If task verification doesn't match reality:
1. **Don't assume you made a mistake** - framework may have changed
2. **Escalate with specifics:** "Expected X, got Y, here's the diff"
3. **Request doc query:** "Can you check current [framework] docs?"

### Impact Assessment

**Without doc queries:**
- ❌ 6 tasks needed rewriting
- ❌ ~2 hours wasted on mismatch debugging
- ❌ Engineer loses confidence in task accuracy
- ❌ Risk of building on wrong assumptions

**With doc queries:**
- ✅ Tasks match actual framework behavior
- ✅ Verification steps work first try
- ✅ Engineer can proceed with confidence
- ✅ Catches breaking changes between versions

### Tools Required

**Senior Engineer must have access to:**
- Context7 MCP (`resolve-library-id`, `query-docs`)
- Ability to query before writing tasks
- Framework version detection if available

**Fallback:** If Context7 unavailable, explicitly mark tasks as "unverified" and instruct Engineer to verify first step and escalate if mismatch.

---

## 11. Task Granularity: Local LLM Success Factors

### The Problem We Encountered

**What happened:** When creating Phase 4 (Next.js frontend) tasks, the initial task breakdown was:
- Task 19: Next.js Project Initialization (8+ files, multiple configurations)
- Task 20: WebSocket Connection and State Management (6 files, complex patterns)
- Task 21: Tab Navigation and Layout (4 components, integration)

**User feedback:** "These tasks are too large for my local LLM. We've had several instances where we couldn't handle the task and had to escalate to you or Opus. Please think about breaking into smaller substeps."

**Root cause:** Senior Engineer role was optimized for Sonnet-level execution, not Haiku/local LLM constraints.

**Additional discovery:** During project initialization, interactive prompts can introduce unnecessary complexity (React Compiler, custom aliases) that isn't documented in tasks, causing confusion.

### The Pattern Recognition

Successful tasks (Phases 1-3) had natural granularity:
- C++ classes: One class per task (one .h, one .cpp file)
- Python bindings: One module per task
- API endpoints: Small files, clear scope

Failed tasks (initial Phase 4) tried to do too much:
- "Set up entire project" (dozens of decisions)
- "Implement WebSocket system" (architecture + implementation)
- "Create full UI layout" (multiple components + integration)

### The Lesson

**For local LLM success:**

1. **One file at a time** (maximum two related files)
   ```
   ❌ Task 19: Set up Next.js project
      - package.json
      - tsconfig.json
      - tailwind.config.js
      - next.config.js
      - app/layout.tsx
      - app/page.tsx
      - lib/types.ts
      - lib/utils.ts
   
   ✅ Task 19a: Initialize Next.js project (command only)
   ✅ Task 19b: Install dependencies (command only)
   ✅ Task 19c: Configure TypeScript (tsconfig.json)
   ✅ Task 19d: Configure Tailwind (tailwind.config.js)
   ✅ Task 19e: Create type definitions (lib/types.ts)
   ✅ Task 19f: Create utilities (lib/utils.ts)
   ✅ Task 19g: Create root layout (app/layout.tsx)
   ✅ Task 19h: Create home page (app/page.tsx)
   ```

2. **Provide context, not code**
   ```
   ❌ Bad: Show example code
   ```typescript
   class WebSocketClient {
     connect() { ... }
   }
   ```
   
   ✅ Good: Describe structure with references
   "Create a WebSocket client class with:
   - Constructor accepting URL parameter
   - Connect method creating WebSocket instance
   - Disconnect method closing connection
   
   Reference: MDN WebSocket API documentation
   Search: 'JavaScript WebSocket client' if unfamiliar
   Escalate to Haiku if: Pattern unclear after docs review"
   ```

3. **Include escalation hints**
   ```
   Every task should specify:
   - When to escalate (e.g., "If React Context is unfamiliar...")
   - What to search (e.g., "Search: React Context API tutorial")
   - Alternative approach (e.g., "Simpler: Use props drilling first")
   ```

4. **Simple verification per task**
   ```
   ❌ Complex: "Test WebSocket connection with full integration"
   ✅ Simple: "Run: npm run dev. Check console for errors."
   ```

### Specific Recommendations for Task Breakdown

#### Pattern: Project Initialization

**Instead of:** "Set up [framework] project"

**Break into:**
1. Run initialization command (exact command provided)
2. Install dependencies (exact npm/pip command)
3. Configure tool A (one config file)
4. Configure tool B (one config file)
5. Create first component (one file)

#### Pattern: Complex Class/Component

**Instead of:** "Implement [feature] with reconnection and error handling"

**Break into:**
1. Create basic structure (connect/disconnect only)
2. Add message handling (send/receive)
3. Add reconnection logic (separate task)
4. Add error handling (separate task)
5. Add logging/debugging (separate task)

#### Pattern: UI Layout

**Instead of:** "Create SCADA interface with tabs and controls"

**Break into:**
1. Create tab navigation component (visual only)
2. Add tab state management (functionality)
3. Create view A placeholder (one component)
4. Create view B placeholder (one component)
5. Integrate tabs with views (wire together)

#### Pattern: API Integration

**Instead of:** "Integrate with backend API"

**Break into:**
1. Define TypeScript types matching API (one file)
2. Create HTTP client class (basic GET/POST)
3. Add error handling to client
4. Create React hook wrapping client
5. Create Context provider for hook
6. Integrate provider in app

### Task Size Guidelines

| Metric | Too Large ❌ | Right Size ✅ |
|--------|-------------|--------------|
| Files touched | 5+ files | 1-2 files |
| Time estimate | 1-2 hours | 15-30 minutes |
| Lines of prose | 200+ lines | 50-100 lines |
| Decisions required | Many unclear choices | Clear path forward |
| Context needed | Multiple frameworks | Single concept |
| Verification | Complex multi-step | One command |

### Interactive Prompts and Default Choices

**Problem:** CLI tools (create-next-app, create-react-app, etc.) often have interactive prompts for experimental or optional features. When tasks don't document what to choose, engineers must make decisions without context.

**Solution:** In task descriptions, explicitly state answers for all interactive prompts:

```markdown
When prompted during npx create-next-app:
- React Compiler: No (experimental, not needed for 1 Hz updates)
- Import alias: No (keep default @/* convention)
- Turbopack: No (not stable yet, stick with webpack)
```

**Principle:** Default to stable, well-documented options. Avoid experimental features unless they solve a specific problem. Local LLMs struggle with "cutting edge" features that have limited training data.

### Information Structure Per Task

**Essential elements for local LLM success:**

1. **Exact file path(s)** - No ambiguity where code goes
2. **Structure template** - List sections file should contain
3. **Reference links** - Docs for unfamiliar patterns
4. **Search keywords** - What to Google if stuck
5. **Escalation trigger** - When to ask for help
6. **Verification command** - Exact command to test
7. **Acceptance checklist** - 3-5 clear items

**Template:**
```markdown
## Task Xa: [Single action description]

**Files:** [exact/path/to/file.ts]
**Time:** 15-30 minutes

### Structure
File should contain:
- Section 1: [purpose]
- Section 2: [purpose]

### Reference
If unfamiliar with [pattern]:
- Search: "[keywords]"
- Docs: [URL]
- Escalate if: [condition]

### Verification
```bash
[exact command]
```

Expected: [outcome]

### Acceptance
- [ ] File exists at path
- [ ] Contains required sections
- [ ] Verification passes
```

### Impact at Scale

**Without micro-tasks (Phase 4 initial attempt):**
- Local LLM: High failure rate, frequent escalation
- Developer frustration: "This is too vague"
- Time wasted: Reading docs, trial-and-error
- Escalations: 50-70% of tasks need Sonnet/Opus

**With micro-tasks (revised Phase 4):**
- Local LLM: High success rate on focused tasks
- Clear path: Exactly what to create
- Fast iteration: 15-30 min per task
- Escalations: 10-20% for genuinely hard tasks

**Cost implications:**
- Local LLM: Free
- Haiku: $0.25 per million input tokens
- Sonnet: $3 per million input tokens  
- Opus: $15 per million input tokens

For a 20-task phase:
- Poor granularity: 14 escalations to Sonnet = ~42M tokens = ~$126
- Good granularity: 3 escalations to Haiku = ~9M tokens = ~$2.25

**ROI:** Better task breakdown = 50x cost reduction + faster development.

---

## 1. Testing Strategy: Mock Early, Mock Often

### The Problem We Encountered

**What happened:** Haiku attempted to implement Task 16 (API testing suite) but got completely stuck because:
- TestClient wasn't properly invoking the FastAPI lifespan context
- Tests expected a `/api/state` endpoint that didn't exist
- Async test patterns were incorrectly applied to synchronous TestClient methods
- The C++ dependency made it impossible to run tests without compilation

**Root cause:** Testing was an afterthought. We built the API first, then tried to retrofit tests.

### The Lesson

**For larger simulations:**

1. **Create mock interfaces FIRST** before implementing physics engines
   ```python
   # Define the interface
   class SimulatorInterface(Protocol):
       def step(self) -> None: ...
       def get_state(self) -> dict[str, float]: ...
       def set_input(self, idx: int, value: float) -> None: ...
   
   # Mock for testing
   class MockSimulator:
       def __init__(self): 
           self.state = {"temperature": 300.0, "pressure": 101.3}
       # ... implement interface
   
   # Real implementation later
   class ThermalSimulator:
       # ... actual physics
   ```

2. **Test the API layer independently** from physics
   - Validates HTTP contracts
   - Validates WebSocket protocols
   - Validates serialization/deserialization
   - Runs in milliseconds, not minutes

3. **Design for testability from day one**
   - Use dependency injection
   - Avoid tight coupling to compiled code
   - Make simulation state easily mockable

### Specific Recommendations for Larger Systems

```python
# BAD: Tight coupling to C++ module
from thermal_sim import ThermalSimulator
sim = ThermalSimulator()  # Requires compilation to test

# GOOD: Dependency injection
class SimulationManager:
    def __init__(self, simulator: SimulatorInterface):
        self.simulator = simulator
    
# Test with mock
manager = SimulationManager(MockSimulator())

# Production with real physics
manager = SimulationManager(thermal_sim.ThermalSimulator())
```

**Impact at scale:** With 10+ coupled physics engines (thermal, fluid, structural, electrical), being unable to test the API layer without compiling everything becomes a major productivity bottleneck.

---

## 2. API Design: Missing Endpoints Discovered Late

### The Problem We Encountered

**What happened:** Tests expected `GET /api/state` endpoint but it didn't exist. We only had:
- `GET /api/config` (configuration, not current state)
- `GET /api/history` (historical data)
- WebSocket (real-time streaming)

**Why this matters:** Frontend developers need to:
1. Fetch current state on page load (can't wait for WebSocket)
2. Poll for state if WebSocket disconnects
3. Display state in REST API responses (e.g., "current value: 2.5m")

### The Lesson

**For larger simulations:**

1. **Always provide both snapshot and streaming interfaces**
   - `GET /api/state` - current state snapshot
   - `WebSocket /ws` - real-time streaming
   - `GET /api/history` - historical data

2. **Design API endpoints BEFORE implementation**
   - Write OpenAPI spec first
   - Generate types/clients from spec
   - Review with frontend team early
   - Validate with stakeholders

3. **Consider different access patterns**
   ```
   Research scientist: Needs bulk historical data → GET /history
   Control operator: Needs real-time updates → WebSocket
   Dashboard: Needs current value on load → GET /state
   Mobile app: Needs efficient updates → WebSocket with compression
   ```

### Specific Recommendations for Larger Systems

For a multi-physics simulation with 1000+ state variables:

```python
# Provide filtered state access
GET /api/state?subsystem=thermal
GET /api/state?variables=temp1,temp2,pressure
GET /api/state/summary  # Only critical values

# Provide different update rates
WebSocket /ws/fast    # 100 Hz for control
WebSocket /ws/normal  # 1 Hz for monitoring  
WebSocket /ws/slow    # 0.1 Hz for trends
```

**Impact at scale:** With 1000+ variables, sending all state every second becomes bandwidth-prohibitive. Need selective subscriptions and variable update rates.

---

## 3. Singleton Pattern: Subtle Bugs Waiting to Happen

### The Problem We Encountered

**What happened:** The `SimulationManager` uses a singleton pattern:
```python
class SimulationManager:
    _instance = None
    
    def __new__(cls, config):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance
```

**The trap:** Second instantiation silently ignores the new config:
```python
mgr1 = SimulationManager(config_a)  # Uses config_a
mgr2 = SimulationManager(config_b)  # SILENTLY uses config_a!
```

**Why we did it:** Ensured single simulation instance across the application.

### The Lesson

**For larger simulations:**

1. **Singletons are dangerous at scale** because:
   - You'll want multiple simulation instances (scenarios, A/B testing)
   - Testing becomes difficult (can't isolate tests)
   - Concurrent simulations impossible (multi-user systems)

2. **Better pattern: Explicit lifecycle management**
   ```python
   class SimulationRegistry:
       def __init__(self):
           self.simulations: dict[str, Simulator] = {}
       
       def create(self, sim_id: str, config: Config) -> Simulator:
           if sim_id in self.simulations:
               raise ValueError(f"Simulation {sim_id} already exists")
           sim = Simulator(config)
           self.simulations[sim_id] = sim
           return sim
       
       def get(self, sim_id: str) -> Simulator:
           return self.simulations[sim_id]
       
       def delete(self, sim_id: str) -> None:
           sim = self.simulations.pop(sim_id)
           sim.cleanup()
   ```

3. **Support multiple concurrent simulations**
   ```
   POST /api/simulations                  # Create new simulation
   GET  /api/simulations/{id}/state       # Get specific simulation state
   WS   /ws/{id}                          # Connect to specific simulation
   DELETE /api/simulations/{id}           # Clean up simulation
   ```

### Specific Recommendations for Larger Systems

For a platform running multiple simulations:

```python
# Bad: Global singleton
simulation = Simulator()  # Only one ever

# Good: Registry pattern
registry = SimulationRegistry()

# Scenario analysis
baseline = registry.create("baseline", config_a)
scenario1 = registry.create("scenario1", config_b)
scenario2 = registry.create("scenario2", config_c)

# Run in parallel, compare results
```

**Impact at scale:** Engineering teams often need to run parameter sweeps (100s of scenarios), A/B testing (multiple configs), or multi-user access (each user has own simulation). Singleton pattern makes this impossible.

---

## 4. Lifespan Management: FastAPI Context Gotchas

### The Problem We Encountered

**What happened:** TestClient wasn't invoking the FastAPI lifespan context:
```python
# This didn't work
@pytest.fixture
def client(app):
    return TestClient(app)  # Lifespan not run!

# This worked
@pytest.fixture
def client(app):
    with TestClient(app) as test_client:
        yield test_client  # Lifespan runs properly
```

**Root cause:** FastAPI lifespan events only run when using TestClient as a context manager.

### The Lesson

**For larger simulations:**

1. **Understand framework lifecycle hooks deeply**
   - Read documentation thoroughly
   - Test startup/shutdown explicitly
   - Verify resource cleanup

2. **Lifespan events are critical for:**
   - Loading large simulation data (GB+ datasets)
   - Establishing database connections
   - Initializing compiled modules
   - Spawning worker processes
   - Allocating GPU resources

3. **Test cleanup explicitly**
   ```python
   def test_simulation_cleanup():
       with TestClient(app) as client:
           # Startup runs: resources allocated
           response = client.get("/api/state")
           assert response.status_code == 200
       
       # Shutdown runs: resources freed
       # Verify no memory leaks, files closed, etc.
       assert check_no_leaked_resources()
   ```

### Specific Recommendations for Larger Systems

For a simulation that loads 10GB datasets on startup:

```python
@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup
    logger.info("Loading datasets...")
    app.state.thermal_data = load_thermal_dataset()  # 5GB
    app.state.structural_data = load_structural_dataset()  # 5GB
    logger.info("Datasets loaded")
    
    yield
    
    # Shutdown - CRITICAL for preventing memory leaks
    logger.info("Cleaning up datasets...")
    del app.state.thermal_data
    del app.state.structural_data
    gc.collect()
    logger.info("Cleanup complete")

app = FastAPI(lifespan=lifespan)
```

**Impact at scale:** A simulation running continuously that doesn't properly clean up resources on restart will accumulate memory leaks, file handles, or GPU memory until the system crashes.

---

## 5. Async vs Sync: The Testing Confusion

### The Problem We Encountered

**What happened:** Tests were incorrectly marked as async:
```python
@pytest.mark.asyncio
async def test_websocket_connection(app):
    client = TestClient(app)  # Sync client!
    with client.websocket_connect("/ws") as ws:  # Sync method!
        # ...
```

**Why this happened:** Confusion between:
- FastAPI endpoints (async)
- TestClient methods (sync, despite testing async endpoints)
- Real WebSocket clients (async)

### The Lesson

**For larger simulations:**

1. **Understand sync vs async at every layer**
   ```
   Layer 1: C++ Physics Engine       → Sync (blocking computation)
   Layer 2: Python Bindings          → Sync (GIL-bound)
   Layer 3: FastAPI Orchestration    → Async (I/O-bound)
   Layer 4: WebSocket Broadcasting   → Async (I/O-bound)
   Layer 5: HTTP Endpoints           → Async (I/O-bound)
   ```

2. **Don't cargo-cult async keywords**
   - Use async for I/O-bound operations (network, disk, database)
   - Use sync for CPU-bound operations (physics, math, data processing)
   - Use `asyncio.to_thread()` to run sync code in async context

3. **Testing patterns differ**
   ```python
   # Testing async endpoints with SYNC TestClient
   def test_endpoint(client):  # Not async!
       response = client.get("/api/state")  # Sync call
   
   # Testing WebSocket with SYNC TestClient
   def test_websocket(client):  # Not async!
       with client.websocket_connect("/ws") as ws:  # Sync!
           data = ws.receive_json()  # Sync!
   
   # Testing with ASYNC real client
   @pytest.mark.asyncio
   async def test_real_websocket():
       async with websockets.connect("ws://...") as ws:  # Async!
           data = await ws.recv()  # Async!
   ```

### Specific Recommendations for Larger Systems

For a multi-physics simulation with mixed sync/async:

```python
class PhysicsEngine:
    def compute_step(self) -> StateVector:
        # CPU-intensive, sync, releases GIL
        return run_expensive_simulation()

class APILayer:
    async def get_state(self):
        # I/O-bound, async
        async with self.lock:
            # Run sync computation in thread pool
            state = await asyncio.to_thread(
                self.physics.compute_step
            )
            return state

# Don't block the async event loop
async def bad_approach():
    state = self.physics.compute_step()  # BLOCKS EVENT LOOP!
    
# Do run CPU work in thread pool
async def good_approach():
    state = await asyncio.to_thread(self.physics.compute_step)
```

**Impact at scale:** Blocking the async event loop with CPU-intensive work causes all HTTP requests to freeze. With 100+ concurrent users, this creates unacceptable latency.

---

## 6. State Endpoint Design: Snapshot vs Streaming

### The Problem We Encountered

**What we built:**
- WebSocket: Real-time streaming at 1 Hz
- History: Past data with duration filter
- Config: Static configuration

**What was missing:**
- Current state snapshot (needed for page loads)

**Why this pattern emerged:** We focused on the "streaming" use case without considering "point-in-time" needs.

### The Lesson

**For larger simulations:**

1. **Always provide snapshot + stream**
   ```
   GET /api/state          → Current state (point in time)
   GET /api/history        → Historical data (time range)
   WebSocket /ws           → Real-time updates (continuous)
   ```

2. **Different clients need different patterns**
   - Web dashboard: Snapshot on load, then stream
   - Data analyst: Bulk history download
   - Control system: Low-latency streaming only
   - Mobile app: Snapshot with fallback polling

3. **Consider caching for snapshots**
   ```python
   class SimulationManager:
       def __init__(self):
           self._state_cache = None
           self._cache_time = 0
           self._cache_ttl = 0.1  # 100ms
       
       def get_state(self):
           now = time.time()
           if now - self._cache_time > self._cache_ttl:
               self._state_cache = self._compute_state()
               self._cache_time = now
           return self._state_cache
   ```

### Specific Recommendations for Larger Systems

For a simulation with 10,000 state variables:

```python
# Provide hierarchical state access
GET /api/state                  # Error: Too large
GET /api/state/summary          # Top 20 critical values
GET /api/state/thermal          # Thermal subsystem only
GET /api/state/thermal/zone1    # Specific zone

# Provide different formats
GET /api/state?format=json      # Web clients
GET /api/state?format=binary    # High-performance clients
GET /api/state?format=csv       # Data analysts

# Provide compression
GET /api/state?compress=gzip    # Reduce bandwidth
```

**Impact at scale:** Returning 10,000 floats (80KB) on every request from 100 concurrent users = 8MB/s sustained bandwidth. Need selective queries, caching, and compression.

---

## 7. Error Handling: Production vs Development

### What We Did Well

Our error handling is comprehensive:
```python
try:
    state = simulation_manager.get_state()
    return state
except Exception as e:
    logger.error(f"Error getting state: {e}")
    return JSONResponse(
        status_code=500, 
        content={"error": str(e)}
    )
```

### The Hidden Issue

**Development:** Stack traces are helpful  
**Production:** Stack traces expose internal details

```python
# Development error (helpful):
{
  "error": "Tank height cannot be negative: h=-0.5 at t=123.4"
}

# Production error (secure):
{
  "error": "Internal simulation error",
  "error_id": "a1b2c3d4",
  "timestamp": "2026-02-09T10:30:00Z"
}
```

### The Lesson

**For larger simulations:**

1. **Different error handling for dev vs prod**
   ```python
   class ErrorHandler:
       def __init__(self, environment: str):
           self.is_dev = (environment == "development")
       
       def format_error(self, e: Exception) -> dict:
           error_id = generate_uuid()
           logger.error(f"Error {error_id}: {e}", exc_info=True)
           
           if self.is_dev:
               return {
                   "error": str(e),
                   "type": type(e).__name__,
                   "traceback": traceback.format_exc()
               }
           else:
               return {
                   "error": "Internal error",
                   "error_id": error_id,
                   "timestamp": datetime.utcnow().isoformat()
               }
   ```

2. **Log everything, return little**
   - Detailed logs server-side for debugging
   - Generic errors client-side for security
   - Error IDs to correlate logs with user reports

3. **Monitor error rates**
   ```python
   metrics.increment("simulation.errors", tags={
       "error_type": type(e).__name__,
       "endpoint": request.url.path
   })
   ```

### Specific Recommendations for Larger Systems

For production simulation platforms:

```python
# Add structured logging
logger.error(
    "Simulation step failed",
    extra={
        "error_id": error_id,
        "sim_id": sim.id,
        "timestep": sim.time,
        "state": sim.get_debug_state(),
        "inputs": sim.get_inputs()
    }
)

# Add error recovery
try:
    sim.step()
except NumericalInstability as e:
    logger.warning(f"Numerical instability, reducing timestep")
    sim.reduce_timestep()
    sim.step()  # Retry
except UnrecoverableError as e:
    logger.error(f"Unrecoverable error, freezing simulation")
    sim.freeze()
    alert_operators(e)
```

**Impact at scale:** In production, uninformative error messages prevent debugging while verbose errors expose security vulnerabilities. Need structured logging with correlation IDs.

---

## 8. Dependency Management: Compilation Bottleneck

### The Problem

**Current architecture:**
```
Python Tests → Require FastAPI → Requires tank_sim → Requires C++ compilation
```

**Impact:**
- Can't run tests without compiling C++
- Compilation takes 30-60 seconds
- CI/CD pipelines slow
- Quick iteration impossible

### The Lesson

**For larger simulations:**

1. **Separate concerns with clear boundaries**
   ```
   Layer 1: Physics (C++/Fortran/Julia)    → Compile separately
   Layer 2: Bindings (pybind11/cffi)       → Compile separately
   Layer 3: Orchestration (Pure Python)    → Test independently
   Layer 4: API (FastAPI)                  → Test with mocks
   Layer 5: Frontend (TypeScript)          → Test with MSW
   ```

2. **Use dependency injection everywhere**
   ```python
   # Bad: Hard dependency
   import tank_sim
   simulator = tank_sim.Simulator()
   
   # Good: Injected dependency
   def create_app(simulator: SimulatorInterface):
       app = FastAPI()
       app.state.simulator = simulator
       return app
   
   # Test with mock
   app = create_app(MockSimulator())
   
   # Production with real
   app = create_app(tank_sim.Simulator())
   ```

3. **Provide pre-compiled wheels for CI**
   ```yaml
   # .github/workflows/test.yml
   - name: Install from wheel
     run: uv pip install tank_sim-0.1.0-cp311-cp311-linux_x86_64.whl
   
   - name: Run API tests  # No compilation needed!
     run: uv run pytest api/tests/
   ```

### Specific Recommendations for Larger Systems

For a multi-physics platform with 10+ compiled modules:

```python
# Define abstract interfaces
class ThermalSimulator(Protocol):
    def step(self, dt: float) -> ThermalState: ...

class FluidSimulator(Protocol):
    def step(self, dt: float) -> FluidState: ...

# Test orchestration without compilation
def test_coupled_simulation():
    thermal = MockThermalSim()
    fluid = MockFluidSim()
    
    manager = CoupledSimManager(thermal, fluid)
    manager.step()  # Tests coupling logic only
    
    assert thermal.step.called
    assert fluid.step.called

# Production uses real implementations
thermal = thermal_engine.RealThermalSim()
fluid = fluid_engine.RealFluidSim()
manager = CoupledSimManager(thermal, fluid)
```

**Impact at scale:** With 10 physics engines (thermal, structural, fluid, electrical, chemical, optical, acoustic, magnetic, particle, plasma), requiring compilation for every test makes iteration unbearably slow.

---

## 9. WebSocket Design: Broadcast vs Targeted

### What We Built

Current WebSocket implementation broadcasts to ALL connected clients:
```python
async def broadcast(self, message: dict):
    for connection in self.connections:
        await connection.send_json(message)
```

**This works fine for:**
- Single user
- All users need same data
- Low client count (<100)

**This breaks at scale:**
- 1000 users × 1 Hz × 1KB = 1 MB/s sustained
- Users on mobile/slow connections get overwhelmed
- Users only care about specific subsystems

### The Lesson

**For larger simulations:**

1. **Implement selective subscriptions**
   ```python
   class ConnectionManager:
       def __init__(self):
           # Map: connection → subscribed variables
           self.subscriptions: dict[WebSocket, set[str]] = {}
       
       async def subscribe(self, ws: WebSocket, variables: list[str]):
           self.subscriptions[ws] = set(variables)
       
       async def broadcast(self, state: dict):
           for ws, vars in self.subscriptions.items():
               # Send only subscribed variables
               filtered = {k: v for k, v in state.items() if k in vars}
               await ws.send_json(filtered)
   ```

2. **Implement different update rates**
   ```python
   # Fast updates: Control-critical variables
   /ws/fast?rate=100    # 100 Hz: valve_position, flow_rate
   
   # Normal updates: Monitoring
   /ws/normal?rate=1    # 1 Hz: tank_level, temperatures
   
   # Slow updates: Trends
   /ws/slow?rate=0.1    # 0.1 Hz: daily_production, efficiency
   ```

3. **Implement compression for large states**
   ```python
   import msgpack
   
   # JSON: 1000 floats = ~20KB
   json_msg = json.dumps({"state": [0.1] * 1000})
   
   # MessagePack: 1000 floats = ~8KB
   msgpack_msg = msgpack.packb({"state": [0.1] * 1000})
   
   # Compression ratio: 2.5x
   ```

### Specific Recommendations for Larger Systems

For a plant-wide simulation with 10,000 sensors:

```python
# Client subscribes to specific data
ws.send_json({
    "type": "subscribe",
    "subsystems": ["thermal_zone_1", "pump_3"],
    "variables": ["temperature", "pressure", "flow_rate"],
    "rate": 1.0  # Hz
})

# Server sends only subscribed data
for ws, subscription in subscriptions.items():
    filtered_state = filter_state(
        full_state, 
        subscription.subsystems,
        subscription.variables
    )
    
    if should_send(ws, subscription.rate):
        await ws.send_json(filtered_state)
```

**Impact at scale:** Broadcasting 10,000 variables to 1,000 users at 1 Hz = 10 million values/second = ~80 MB/s. With selective subscriptions (average 10 variables/user) = 10,000 values/second = ~80 KB/s (1000x reduction).

---

## 10. Testing Philosophy: Integration vs Unit

### What We Did

Created comprehensive integration tests:
- Mock C++ module
- Test full FastAPI stack
- Test WebSocket protocol
- Test concurrent access

**This validated:**
- ✅ API contracts
- ✅ Error handling
- ✅ Serialization
- ✅ Protocol compliance

**This did NOT validate:**
- ❌ Physics correctness
- ❌ Numerical stability
- ❌ Performance characteristics
- ❌ Memory usage patterns

### The Lesson

**For larger simulations:**

1. **Need multi-level testing strategy**
   ```
   Level 1: Unit Tests (C++)           → Physics correctness
   Level 2: Integration Tests (Python) → Binding correctness
   Level 3: API Tests (Mock)           → Protocol correctness
   Level 4: End-to-End Tests (Real)    → System correctness
   Level 5: Performance Tests          → Scalability validation
   ```

2. **Each level has different goals**
   ```python
   # Level 1: Validate physics
   def test_energy_conservation():
       model = ThermalModel()
       initial_energy = model.total_energy()
       model.step(dt=0.1)
       final_energy = model.total_energy()
       assert abs(final_energy - initial_energy) < 1e-6
   
   # Level 3: Validate API contract
   def test_api_returns_valid_json():
       response = client.get("/api/state")
       assert response.status_code == 200
       data = response.json()
       assert "temperature" in data
   
   # Level 5: Validate performance
   def test_step_performance():
       sim = LargeSimulation()
       times = []
       for _ in range(100):
           start = time.perf_counter()
           sim.step()
           times.append(time.perf_counter() - start)
       
       assert np.mean(times) < 0.01  # < 10ms per step
       assert np.std(times) < 0.002   # Consistent timing
   ```

3. **Don't over-mock in integration tests**
   ```python
   # Bad: Mocks everything, tests nothing
   def test_simulation():
       sim = Mock()
       sim.step.return_value = None
       sim.get_state.return_value = {"level": 2.5}
       # This tests Mock(), not your code!
   
   # Good: Mocks external dependencies only
   def test_simulation():
       sim = RealSimulator(MockHardwareInterface())
       sim.step()  # Tests real simulation logic
       state = sim.get_state()
       assert state["level"] == pytest.approx(2.5, abs=0.1)
   ```

### Specific Recommendations for Larger Systems

For a production simulation platform:

```python
# tests/unit/ - Pure physics, no I/O
def test_navier_stokes_solver():
    solver = NavierStokesSolver()
    # Test numerical accuracy
    
# tests/integration/ - Python bindings
def test_python_bindings():
    sim = compiled_module.FluidSim()
    # Test data conversion

# tests/api/ - FastAPI layer (mocked simulator)
def test_rest_endpoints():
    client = TestClient(app)
    # Test HTTP contracts

# tests/e2e/ - Full system (real simulator)
def test_end_to_end():
    # Start real API server
    # Run real simulation
    # Test actual behavior

# tests/performance/ - Benchmarks
def test_performance():
    sim = RealSimulation()
    # Measure timing, memory, etc.
```

**Impact at scale:** Without proper test levels, you either:
- Over-mock (fast tests, no confidence)
- Under-mock (slow tests, CI timeouts)
- Miss critical bugs at boundaries

---

## 12. Spec-Implementation Drift: Verify Backend Protocol Before Writing Frontend Tasks

### The Problem We Encountered

**What happened:** Task 20b (Add Message Sending Methods) specified that the WebSocketClient class should have domain-specific methods: `sendSetpoint()`, `sendPIDGains()`, `sendInletFlow()`, `sendInletMode()`. However, the Engineer implemented a generic `sendCommand(command, params)` method that wrapped messages as `{"type": "command", "command": "...", "params": {...}}`.

This created a chain of problems:

1. **Backend mismatch:** The backend (`api/main.py`) expects flat messages like `{"type": "setpoint", "value": 42}`. The `sendCommand()` wrapper format (`{"type": "command", ...}`) would be rejected as `Unknown message type: command`.

2. **Task 20d cascading failure:** Task 20d (useWebSocket hook) referenced the domain-specific methods (`websocket.sendSetpoint(value)`) that were never implemented. The Engineer had to escalate to resolve the mismatch.

3. **Dead code:** `subscribe()` and `unsubscribe()` methods were also implemented, but the backend has no subscribe/unsubscribe protocol — it broadcasts all state to all clients unconditionally.

4. **Type system gap:** `frontend/lib/types.ts` already had a correct `WebSocketMessage` union type matching the backend protocol, but neither the task spec nor the implementation referenced it.

**Root cause:** The Senior Engineer wrote Task 20b from general WebSocket patterns (generic command/subscribe model) rather than verifying the actual backend protocol defined in `api/main.py` and `frontend/lib/types.ts`.

### Additional Discovery: State Field Name Mismatch

While reviewing, we found another spec-implementation drift:

- **Backend sends:** `tank_level` (in `api/simulation.py:get_state()`)
- **Frontend expects:** `level` (in `frontend/lib/types.ts:SimulationState`)
- **Backend sends:** `controller_output` — Frontend has no corresponding field
- **Frontend expects:** `inlet_mode`, `inlet_config` — Backend doesn't send these in state updates

This means when the frontend receives state data, `state.level` will be `undefined` while the actual data is in `state.tank_level`. This bug is dormant until Task 21c (ProcessView) tries to display the data.

### The Lesson

**For tasks involving frontend-backend communication:**

1. **Read the actual backend code before writing frontend tasks** — Don't design frontend message handlers from assumed protocols. Check:
   - What message format does the backend expect to receive?
   - What message format does the backend send?
   - What field names does the backend use?

2. **Cross-reference types.ts with backend models** — If you have a types file that claims to match backend models, verify it actually does. Compare field-by-field against the backend's actual serialization output.

3. **Use the type system as the contract** — If `WebSocketMessage` in `types.ts` defines the correct message format, task specs should reference it explicitly: "Send messages matching the `WebSocketMessage` type from `lib/types.ts`" rather than inventing a different API.

4. **Don't add protocols the backend doesn't support** — Generic patterns like subscribe/unsubscribe are common in WebSocket tutorials but may not exist in your backend. Verify before specifying.

### Specific Recommendations

**When writing frontend WebSocket/API tasks:**

```markdown
BEFORE writing tasks, Senior Engineer must:
1. Read the backend endpoint handler (e.g., api/main.py websocket_endpoint)
2. List exact message formats the backend sends and receives
3. Compare backend field names with frontend type definitions
4. Note any mismatches and resolve them BEFORE writing tasks
5. Reference existing type definitions rather than re-describing formats
```

**Checklist for frontend-backend integration tasks:**
- [ ] Backend receive format documented and matches frontend send format
- [ ] Backend send format documented and matches frontend receive types
- [ ] Field names match exactly (e.g., `tank_level` vs `level`)
- [ ] All fields accounted for (no missing or extra fields)
- [ ] No protocols assumed that backend doesn't implement

### Resolution Applied

1. Removed `sendCommand()` (wrapper format backend doesn't understand)
2. Removed `subscribe()`/`unsubscribe()` (protocol backend doesn't have)
3. Added single `send()` method that passes messages through without wrapping
4. Hook constructs messages matching the `WebSocketMessage` type and backend expectations

### Outstanding Issue

The `SimulationState` type in `types.ts` still has field name mismatches with the backend. This needs to be resolved before Tasks 21c-21e (which display state data). Either:
- Update `types.ts` to match backend field names (`tank_level` instead of `level`, add `controller_output`)
- Or update the backend to match frontend field names

This should be addressed as a prerequisite to Task 21c.

### Impact Assessment

**Without backend verification:**
- ❌ WebSocket messages silently rejected by backend
- ❌ State data fields undefined in frontend
- ❌ Dead code methods that can never work
- ❌ Multiple escalation rounds to diagnose

**With backend verification:**
- ✅ Messages match protocol on first try
- ✅ State fields map correctly
- ✅ No wasted implementation effort
- ✅ Types serve as verified contract

---

## Summary: Critical Recommendations

### For Scaling to Larger Simulations

#### 0. Framework Documentation (NEW - CRITICAL)
- ✅ **Query docs before writing tasks** - Use Context7 MCP, don't rely on memory
- ✅ **Verify framework versions** - State exact versions in tasks (e.g., "Next.js 16.1.5")
- ✅ **Check project initialization** - Verify what's created by default, what's not
- ✅ **Validate configuration formats** - File names, syntax, and structure change between versions
- ✅ **Test verification commands** - Don't promise output you haven't verified
- ⚠️ **High-risk frameworks** - Next.js, React, Tailwind, Vue, Angular (update frequently)

#### 1. Task Granularity (NEW)
- ✅ **Break tasks into micro-steps** - 1-2 files, 15-30 minutes per task
- ✅ **Provide context, not code** - Links to docs, search keywords
- ✅ **Include escalation hints** - When to ask for help
- ✅ **Simple verification** - One command to test
- ✅ **Optimize for local LLMs** - Smaller models need smaller tasks

#### 2. Spec-Implementation Alignment (NEW)
- ✅ **Verify backend protocol before writing frontend tasks** - Read endpoint handlers, not assumptions
- ✅ **Cross-reference types.ts with backend models** - Field names must match exactly
- ✅ **Use type system as contract** - Reference existing types, don't reinvent
- ✅ **Don't assume protocols** - Verify subscribe/unsubscribe, command formats exist
- ⚠️ **State field names** - Backend `tank_level` vs frontend `level` must be reconciled

#### 3. Architecture
- ✅ **Use dependency injection** - Test layers independently
- ✅ **Define clean interfaces** - Physics, orchestration, API are separate
- ✅ **Avoid singletons** - Support multiple simulations
- ✅ **Support horizontal scaling** - Stateless API design

#### 4. Testing
- ✅ **Mock early and often** - Don't depend on compilation for API tests
- ✅ **Multi-level test strategy** - Unit, integration, E2E, performance
- ✅ **Test cleanup explicitly** - Memory leaks caught early
- ✅ **Test error paths** - Not just happy paths

#### 5. API Design
- ✅ **Provide snapshot + streaming** - Different clients, different needs
- ✅ **Selective subscriptions** - Don't broadcast everything
- ✅ **Variable update rates** - Fast control, slow monitoring
- ✅ **Design before implementing** - OpenAPI spec first

#### 6. Performance
- ✅ **Compression for large states** - MessagePack, gzip
- ✅ **Caching for expensive queries** - State snapshots, computations
- ✅ **Async for I/O, sync for CPU** - Don't block event loop
- ✅ **Hierarchical state access** - Subsystems, not everything

#### 7. Operations
- ✅ **Structured logging** - JSON logs with correlation IDs
- ✅ **Different errors for dev/prod** - Helpful vs secure
- ✅ **Health checks and metrics** - Observability from day one
- ✅ **Graceful degradation** - Handle partial failures

---

## Estimated Impact at Scale

| Aspect | Small System (PoC) | Large System (Production) | Impact of Lessons |
|--------|-------------------|---------------------------|-------------------|
| State variables | 8 | 10,000 | 1250× larger |
| Update rate | 1 Hz | 1-100 Hz | 100× faster |
| Concurrent users | 1 | 1,000 | 1000× more |
| Bandwidth | 1 KB/s | 80 MB/s → 80 KB/s | **1000× reduction** |
| Test time | 20s | 5 min → 20s | **15× faster** |
| Development cycle | 1 min | 30 min → 1 min | **30× faster** |
| Task completion (local LLM) | 50% | 95% | **45% improvement** |
| LLM costs per phase | $126 | $2.25 | **50× reduction** |

**Key insight:** Architectural decisions that seem minor in a PoC create exponential scaling problems in production. Getting these right early saves months of refactoring later. **Task granularity matters just as much as architecture - optimize for the tools you're using.**

---

## 13. Dev Server Startup: Multiple Interacting Failures

### The Problem We Encountered

The `scripts/dev.sh` startup script required three rounds of debugging to get a clean start. Each fix revealed the next issue:

1. **`uv run` rebuilds C++ every time.** Even though `tank_sim` was already installed in the venv, `uv run` triggers a full rebuild of the C++ extension (~60s) on every invocation.

2. **`uvicorn --reload` watches the entire project tree.** Without `--reload-dir`, uvicorn watches everything including `frontend/.next/`, which contains thousands of rapidly-changing files during Next.js development. The two file watchers (uvicorn's WatchFiles and Next.js's Turbopack) conflict, causing the frontend to hang on browser refresh.

3. **Turbopack persistent cache corruption.** After a crash or unclean shutdown, Turbopack's on-disk cache (`frontend/.next/`) can become corrupted. This causes panics on startup: `range start index 22940143 out of range for slice of length 22938863` in `turbo-persistence`. Deleting only the lock file (`frontend/.next/dev/lock`) is insufficient — the entire `.next/` directory must be removed.

4. **Next.js silently falls back to another port.** Without `--port 3000`, Next.js detects port 3000 is in use and quietly starts on port 3001. This breaks the frontend-backend relationship without any obvious error. Passing `--port 3000` explicitly makes it fail fast with `EADDRINUSE` instead.

5. **`lsof` misses processes on wildcard addresses.** The original cleanup used `lsof -ti :3000` to find processes holding ports. However, Next.js binds to `*:3000` (all interfaces, including IPv6 `:::3000`), and `lsof -ti :3000` does not detect this. The stale Next.js process survived the cleanup, causing the `EADDRINUSE` error. `ss -tlnp` reliably detects processes regardless of bind address.

### The Lesson

- Use `.venv/bin/uvicorn` directly to skip the C++ rebuild.
- Scope `--reload-dir` to only the directories that contain backend code.
- **Clear the entire `frontend/.next/` directory** before starting, not just the lock file. Turbopack cache corruption is a known issue.
- **Pass `--port` explicitly** to Next.js so it fails fast rather than silently falling back.
- **Use `ss` instead of `lsof`** for port detection on Linux. `lsof` has blind spots for wildcard/IPv6 listeners; `ss` is part of `iproute2` and always available on modern Linux.
- **Poll for port release** after killing a process — `kill -9` returns immediately but the kernel may take time to free the socket.

### Resolution

The `scripts/dev.sh` script encodes all of these lessons:

```bash
# Port cleanup: use ss (not lsof) to find listeners, then poll until free
pids=$(ss -tlnp "sport = :$port" | grep -oP 'pid=\K[0-9]+' | sort -u)
echo "$pids" | xargs kill -9
# Poll every 100ms for up to 5s until port is released

# Clear entire Turbopack cache
rm -rf frontend/.next

# Backend: direct venv invocation, scoped reload
.venv/bin/uvicorn api.main:app --reload --reload-dir api --reload-dir tank_sim

# Frontend: explicit port to fail fast
npm run dev -- --port 3000
```

### Specific Recommendations

- **Always use `scripts/dev.sh`** to start the dev environment.
- **Never use `uv run`** for starting the backend server. Only use `uv run` for one-off commands like `uv run pytest` where the rebuild check is acceptable.
- **If the frontend hangs on refresh**, check whether another file watcher is scanning `frontend/.next/`.
- **If Next.js panics on startup** with Turbopack errors, delete `frontend/.next/` entirely.
- **Never use `lsof` for port detection** in scripts on Linux — use `ss -tlnp "sport = :PORT"` instead.
- **If `EADDRINUSE` persists** after killing processes, run `ss -tlnp | grep :PORT` to see what's actually holding the port.

---

## 14. Recharts Performance: Real-Time Update Render Cascades

### The Problem We Encountered

The Trends View charts (LevelChart, FlowsChart, ValveChart) were extremely slow to update when new data arrived or when changing time ranges. The UI would visibly lag, making the operator experience unacceptable.

### Root Cause Analysis

Multiple compounding issues were causing excessive re-renders:

1. **Render cascade from dependency loop (CRITICAL):** The `useEffect` that appended real-time WebSocket data to `chartData` listed `chartData.length` as a dependency. Since appending data changes the length, this caused the effect to re-evaluate on every update — triggering at least 2 renders per WebSocket message instead of 1. With 3 unmemoized chart components each doing full SVG recomputation, this doubled the rendering work.

2. **No `React.memo` on chart components (CRITICAL):** All three chart components were plain function components. Every TrendsView re-render caused all three charts to fully re-render — recalculating Recharts layout and recomputing SVG paths for up to 7,200 data points per chart. At 1 Hz updates with the cascade doubling, this meant 6 full Recharts render cycles per second.

3. **CustomTooltip defined inside render function (MEDIUM):** Each chart component defined its `CustomTooltip` as a new function inside the component body. This created a new function reference on every render, passed as a prop to Recharts `<Tooltip content={...}>`, defeating any internal memoization Recharts might do.

4. **Unmemoized legend click handlers (MEDIUM):** `handleLegendClick` was recreated on every render, causing Recharts `<Legend>` to receive new `onClick` references and potentially re-render unnecessarily.

5. **Inline style objects in JSX (LOW):** `wrapperStyle={{ fontSize: 14, cursor: "pointer" }}` created new object references on every render.

### The Lesson

Real-time charting with Recharts requires careful attention to React rendering behaviour. Recharts is SVG-based — every re-render recomputes the entire chart layout and redraws all SVG paths. With thousands of data points, this is expensive. The key principles:

- **Never use derived state as a dependency for the effect that changes it.** Using `chartData.length` as a dependency for the effect that appends to `chartData` creates a feedback loop. Use a `useRef` to track values you need to read without subscribing to them.
- **Always wrap expensive components in `React.memo`.** Recharts components are inherently expensive. Wrapping them prevents re-renders when props haven't actually changed.
- **Hoist stable function components outside the render body.** Components like custom tooltips that don't depend on component state should be defined at module scope so they maintain referential identity.
- **Use `useCallback` for event handlers passed as props.** Recharts components receive handlers as props — stable references prevent unnecessary re-renders.
- **Hoist constant objects to module scope.** Style objects like `{ fontSize: 14, cursor: "pointer" }` should be defined once outside the component.

### Resolution Applied

1. **Replaced `chartData.length` dependency with `useRef`:** Added `latestTimeRef` to track the latest timestamp without subscribing to state changes. The `useEffect` for real-time updates now depends only on `[state]`.
2. **Wrapped all chart components in `React.memo`:** `LevelChart`, `FlowsChart`, and `ValveChart` now skip re-renders when their `data` prop reference hasn't changed.
3. **Hoisted `CustomTooltip` to module scope:** Moved outside the component function body in all three chart components.
4. **Memoized `handleLegendClick` with `useCallback`:** Stable handler references in all chart components.
5. **Hoisted `LEGEND_STYLE` constant to module scope:** Single object created once per module.
6. **Avoided unnecessary `slice()`:** Only call `slice(-7200)` when the array actually exceeds the limit.
7. **Added data downsampling:** The render-cascade fixes alone were insufficient for the 2-hour (7,200 point) case — Recharts simply cannot render that many SVG path segments across 3 charts. Added an evenly-spaced `downsample()` utility that caps chart data at 500 points while preserving first/last endpoints. The full dataset is retained in state for accurate appends; only the display path is downsampled via `useMemo`.

### Specific Recommendations for Real-Time Charting

- **SVG charting libraries have hard limits.** Recharts, Victory, and similar SVG-based libraries become unusable above ~500-1000 data points per chart. This is a fundamental limitation of SVG rendering in browsers. If you need more points, either downsample or switch to a Canvas-based library (e.g., uPlot, Chart.js).
- **Downsample at the display layer, not the data layer.** Keep the full-resolution data in state so appends and time-range switches remain accurate. Only downsample in the `useMemo` that feeds the chart components.
- **Profile before optimising:** Use React DevTools Profiler to identify which components are re-rendering and why. The "Highlight updates" feature makes render cascades immediately visible.
- **Consider the update frequency vs render cost ratio.** At 1 Hz with 3 charts of 7,200 points each, you need sub-100ms render times to stay smooth. Recharts with 7,200 points typically takes 50-150ms per chart — there's no room for wasted renders.
- **`useRef` for read-only values in effects.** If an effect needs to read current state but shouldn't re-run when that state changes, store it in a ref.
- **Be cautious with `useMemo` when using React Compiler.** If the project uses the React Compiler (as this one does), manual `useMemo` calls can conflict with the compiler's automatic memoization. The compiler warned about mismatched dependencies. In such cases, either match the compiler's inferred dependencies exactly or remove the manual memo and let the compiler handle it.
- **Test with production data volumes.** A chart with 60 data points (1 minute) will feel fine. The same chart with 7,200 data points (2 hours) will expose every rendering inefficiency.

### Impact Assessment

- **Before:** Charts noticeably lagging on every 1 Hz update; 2-hour time range would not render at all
- **After:** Single render per WebSocket update, chart data capped at 500 display points regardless of time range, chart components skip re-render when data reference unchanged, stable prop references throughout
- **Estimated improvement:** 2-hour case goes from non-functional to responsive. All time ranges now render ~500 SVG points instead of up to 7,200, combined with elimination of render cascades and unnecessary re-renders

## Next Steps

1. **Immediate:** Apply micro-task breakdown to Phase 4 tasks
2. **Short-term:** Review this document with team
3. **Medium-term:** Update all role prompts with granularity lessons
4. **Long-term:** Incorporate into engineering standards document

---

**Document maintained by:** Engineering team  
**Last updated:** 2026-02-13 (added Lesson 14: Recharts real-time rendering performance)  
**Review cycle:** After each major phase
