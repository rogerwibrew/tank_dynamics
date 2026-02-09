# Architect Clarifying Questions

Before creating the architectural plan, I need clarification on the following items to avoid incorrect assumptions.

## Technical Architecture Questions

### 1. Frontend-Backend Communication Protocol

The spec mentions pybind11 exposing an API. Should the C++ simulation run:

- **Option A**: As a Python process with pybind11 bindings, with a Python web framework (FastAPI/Flask) serving the Next.js frontend?
- **Option B**: Directly from the Next.js backend using a different binding mechanism?

**Your answer:**  

We have had sucess before with option A.  This is my prefered tech stack currently.

---

### 2. Real-time Updates

At 60fps simulation, how should updates be pushed to the browser?

- WebSockets for continuous streaming?
- Server-Sent Events (SSE)?
- Polling (less ideal at 60fps)?

**Your answer:**

I think we said websockets in prior discussions.  Perhaps 60 fps is not required.  Most SCADA systems I have seen are more like 1 update per second.  I think this would also be acceptable for the ODE solver.

---

### 3. Data Storage

The spec mentions storing data "for display in the front end as well as for use later in a machine learning library." What format and persistence do you prefer?

- In-memory ring buffer for real-time display (last N points)?
- File-based (CSV/Parquet) for ML export?
- Database (SQLite/TimescaleDB)?
- How much history should be retained?

**Your answer:**

Let's drop for ML use.  We need the last say one to two hours of data.  I will take your advice on how to store that.

---

## Process Model Questions

### 4. Initial Conditions

What should the default starting state be?

- Tank at steady-state (50% level)?
- Empty tank?
- User-configurable?

**Your answer:**

This is key.  Not so important for this sim, but very improtant later when we have recyles.  Let's say 50% level and 1 m3/s inlet.  Valve consant such that this is steady state with the valve 50% open.

---

### 5. Time Acceleration

Should the simulation run in real-time or accelerated time? (e.g., 1 second of simulation per 0.1 seconds of wall time)

**Your answer:**

Real-time.  This needs to mimic real life.

---

### 6. Parameter Configurability

Which parameters should be user-configurable beyond PID settings?

- Tank dimensions (cross-sectional area, max height)?
- Valve coefficient (k_v)?
- Inlet flow characteristics?

**Your answer:**

We said that we should have a 1m3/s inlet flow initially.  Let;s say the tank will go from 50% full to full in 5 mins if the outlet valve is shut.  Let's go for 5m height.  Figure out the volume based on that.

The valve coefficient as above.  Linear profile.

Inlet flow can be set by the user.  That way we can introduce a step change.  It can also follow random brownian motion between two limits.

---

## UI/UX Questions

### 7. SCADA Visual Style

Any preference for the SCADA look?

- Modern dark theme (typical of newer DCS/SCADA)?
- Classic industrial grey?
- Reference images or existing systems to emulate?

**Your answer:**

Modern dark theme.  

---

### 8. Plot Library

Any preference for charting? (Chart.js, Recharts, D3, etc.)

**Your answer:**

I have used plotly before, but am open to suggestions.

---

### 9. Single Page vs Multi-Tab

Should the process screen and trends be:

- Two separate pages/routes?
- Tabs within the same page?
- Side-by-side panels?

**Your answer:**

if we can do tabs, then that is the way to go.

---

## Testing Questions

### 10. Testing Scope

You mentioned adding testing to the workflow. What level of coverage do you expect?

- Unit tests for C++ ODE functions?
- Integration tests for API?
- E2E tests for the UI?
- All of the above?

**Your answer:**

I think it is good pracice to get into testing and I think we should be comprehensive.  I am weak at this piont though so will lean heavily on your advice.

---

## Additional Notes

*Space for any other context or constraints you'd like to share:*

We should get a github repo as soon as possible.
I want to make sure we download our dependncies as fetchcontent in CMakeLists.txt.  I think we should have compiled libraries we link to the executable.
