# Next Tasks - Tank Dynamics Simulator

**Generated:** 2026-02-13  
**Phase:** Phase 6 - Trends View Enhancement & Polish  
**Previous Phase:** Phase 5 (Process View) - ✅ COMPLETE

---

## Current Phase: Phase 6 - Trends View Enhancement & Polish

Phase 5 delivered a complete SCADA interface with tank visualization and real-time controls. Phase 6 has two goals:

1. **Primary Goal:** Implement historical trend charts using Recharts (already installed v3.7.0)
2. **Secondary Goal:** Address 6 code review findings from `docs/feedback.md` (polish tasks)

The phase is divided into two sub-phases:
- **Phase 6A:** Polish tasks (Tasks 28a-28f) - Quick UX improvements
- **Phase 6B:** Trends View (Tasks 29a-29h) - Historical data visualization

**Total Tasks:** 14 micro-tasks (~6 hours estimated)

---

## Phase 6A: Polish Tasks (Address Code Review Feedback)

These tasks address findings from the Phase 5 code review in `docs/feedback.md`. Each task is independent and quick (5-15 minutes).

---

## Task 28a: Conditional Flow Indicator Animation

**Phase:** 6A - Polish  
**Prerequisites:** Phase 5 complete  
**Estimated Time:** 5 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/TankGraphic.tsx`

### Context and References

This addresses **Major Issue #1** from the code review. Currently, flow indicator arrows use the `animate-pulse` CSS class unconditionally, causing them to pulse even when flows are zero. This creates visual noise and wastes browser resources.

Reference the existing flow indicator code at lines 169 and 190 in TankGraphic.tsx.

### Requirements

Modify the flow indicator arrow line elements to apply the `animate-pulse` class conditionally based on flow state.

**For inlet arrow (currently around line 169):**
- Apply `animate-pulse` class ONLY when `inletFlow > 0`
- When `inletFlow === 0`, do NOT include `animate-pulse` in className
- Keep all other styling unchanged (color coding, stroke width, markers)

**For outlet arrow (currently around line 190):**
- Apply `animate-pulse` class ONLY when `outletFlow > 0`
- When `outletFlow === 0`, do NOT include `animate-pulse` in className
- Keep all other styling unchanged

**Implementation approach:**
- Use conditional expression in className attribute
- Pattern: `className={flowValue > 0 ? "animate-pulse" : ""}`
- Or template literal: `className={\`${flowValue > 0 ? "animate-pulse" : ""}\`}`

### Verification

Run development server:
```bash
cd frontend && npm run dev
```

Open browser to `http://localhost:3000` and navigate to Process View tab.

**Test scenarios:**
1. When inlet flow is zero: Inlet arrow should be gray and NOT pulsing
2. When inlet flow is > 0: Inlet arrow should be blue and pulsing
3. When outlet flow is zero: Outlet arrow should be gray and NOT pulsing  
4. When outlet flow is > 0: Outlet arrow should be blue and pulsing

### Escalation Hints

**Escalate to Haiku if:**
- Unclear how to conditionally apply CSS classes in React/JSX
- Multiple className values need to be combined

**Search for these terms if stuck:**
- "React conditional className"
- "JSX conditional CSS class"

### Acceptance Criteria
- [ ] Inlet arrow only pulses when `inletFlow > 0`
- [ ] Outlet arrow only pulses when `outletFlow > 0`
- [ ] Arrows remain visible (gray) when flow is zero
- [ ] Color coding (blue when flowing, gray when stopped) unchanged
- [ ] No console errors or warnings

---

## Task 28b: Extract SVG Magic Numbers to Constants

**Phase:** 6A - Polish  
**Prerequisites:** None  
**Estimated Time:** 15 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/TankGraphic.tsx`

### Context and References

This addresses **Minor Issue #1** from the code review. The SVG currently uses hardcoded coordinate values like `57.5`, `182.5`, `tankLeft + 75`, `tankLeft + 82.5` scattered throughout the file. These magic numbers make it difficult to understand spatial relationships or adjust the layout.

### Requirements

Extract hardcoded SVG coordinates to named constants at the top of the component function (after the props destructuring and before the calculations section).

**Constants to define:**
- Inlet pipe and arrow positions (currently hardcoded as 57.5 for x-coordinate)
- Outlet pipe and arrow positions (currently hardcoded as 182.5 for x-coordinate)
- Arrow vertical positions and offsets
- Valve symbol dimensions and positions

**Naming pattern:**
- Use UPPER_SNAKE_CASE for constants
- Prefix with element name: `INLET_ARROW_X`, `OUTLET_ARROW_X`, etc.
- Group related constants together with comments

**Example structure:**
```
// Inlet flow indicator positions
const INLET_ARROW_X = ...
const INLET_ARROW_Y_START = ...
const INLET_ARROW_Y_END = ...

// Outlet flow indicator positions
const OUTLET_ARROW_X = ...
...
```

Replace all hardcoded numbers in the JSX with these named constants. Add brief comments explaining spatial relationships (e.g., "centered above tank", "aligned with outlet pipe").

### Verification

Run development server:
```bash
cd frontend && npm run dev
```

**Visual verification:**
- Tank graphic renders identically to before
- All pipes, arrows, and valves in same positions
- No layout shifts or misalignments

**Code verification:**
- No hardcoded coordinate numbers in JSX (except viewBox dimensions)
- Constants defined at top of component
- Constants have descriptive names

### Escalation Hints

**Escalate to Haiku if:**
- Unsure which numbers should become constants vs which are calculations
- Coordinate system relationships unclear

**Search for these terms if stuck:**
- "React component constants best practices"
- "SVG coordinate system"

### Acceptance Criteria
- [ ] All magic number coordinates replaced with named constants
- [ ] Constants defined at component top with clear names
- [ ] Comments explain spatial relationships
- [ ] Visual appearance unchanged
- [ ] No layout regressions

---

## Task 28c: Add Reverse Acting Help Text

**Phase:** 6A - Polish  
**Prerequisites:** None  
**Estimated Time:** 10 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/PIDTuningControl.tsx`

### Context and References

This addresses **Minor Issue #2** from the code review. The "Reverse Acting" checkbox (around line 115-123) has no explanation of what it means or when to use it. Operators unfamiliar with control theory may not understand this term.

### Requirements

Add explanatory help text below the "Reverse Acting" checkbox to explain the concept in plain language for operators.

**Add a new div element** immediately after the checkbox label div, containing:
- Small gray text (Tailwind: `text-xs text-gray-400`)
- Plain language explanation suitable for process operators
- No control theory jargon

**Suggested text (adapt as needed):**
"Check this box if opening the outlet valve DECREASES tank level. For this tank system, valve opening increases drainage, so reverse acting should be checked."

**Styling:**
- Use `text-xs` for small font
- Use `text-gray-400` for muted color
- Add `mt-1` for small top margin
- Consider wrapping in `<div className="text-xs text-gray-400 mt-1">...</div>`

**Placement:**
- Insert after the closing `</label>` tag of the checkbox
- Before the Kc input section

### Verification

Run development server:
```bash
cd frontend && npm run dev
```

Navigate to Process View tab and scroll to PID Tuning section.

**Check:**
- Help text appears below "Reverse Acting" checkbox
- Text is small and gray (not prominent)
- Text explains concept clearly without jargon
- Layout not broken (no overflow or wrapping issues)

### Escalation Hints

**Escalate to Haiku if:**
- Unclear where to insert the help text in JSX structure
- Tailwind classes not applying correctly

**Search for these terms if stuck:**
- "Tailwind CSS text sizing"
- "React JSX text element"

### Acceptance Criteria
- [ ] Help text appears below checkbox
- [ ] Text uses small gray styling
- [ ] Explanation is clear for non-experts
- [ ] No layout issues or overflow
- [ ] Checkbox functionality unchanged

---

## Task 28d: Reset Brownian Params on Mode Switch

**Phase:** 6A - Polish  
**Prerequisites:** None  
**Estimated Time:** 10 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/InletFlowControl.tsx`

### Context and References

This addresses **Minor Issue #3** from the code review. When switching between Constant and Brownian modes, the component preserves pending Brownian parameters in local state. If a user enters Brownian values, switches to Constant, then switches back to Brownian, the old values reappear unexpectedly.

Current `handleModeChange` function is around lines 93-97.

### Requirements

Modify the `handleModeChange` function to reset Brownian parameters to defaults when switching modes.

**Current behavior:**
```
User enters: min=0.5, max=1.5, variance=0.1
User switches to Constant mode (Brownian inputs hidden)
User switches back to Brownian
Previous values appear: min=0.5, max=1.5, variance=0.1  ← Can be confusing
```

**Desired behavior:**
When mode changes, reset local state for the mode being switched AWAY from.

**Modification approach:**
In `handleModeChange` function:
- When switching to Constant mode: No action needed (Brownian params don't affect Constant)
- When switching to Brownian mode: Reset Brownian params to sensible defaults
  - `setLocalMin(0.8)`
  - `setLocalMax(1.2)`
  - `setLocalVariance(0.05)`

**Alternative (simpler) approach:**
Always reset to defaults when mode changes, regardless of direction. This provides clearest UX - mode change always gives fresh defaults.

Keep existing logic for:
- `setHasLocalChanges(true)`
- `setErrorMessage("")`

### Verification

Run development server:
```bash
cd frontend && npm run dev
```

Navigate to Process View tab, scroll to Inlet Flow Control section.

**Test sequence:**
1. Select Brownian mode
2. Enter custom values (e.g., min=0.5, max=1.5, variance=0.1)
3. Switch to Constant mode
4. Switch back to Brownian mode
5. **Verify:** Brownian inputs show defaults (0.8, 1.2, 0.05), NOT custom values from step 2

### Escalation Hints

**Escalate to Haiku if:**
- Unclear which state setter functions to call
- Uncertain about when to reset (both directions vs one direction)

**Search for these terms if stuck:**
- "React useState reset to default"
- "React component local state"

### Acceptance Criteria
- [ ] Switching from Constant to Brownian shows default Brownian values
- [ ] Previously entered Brownian values don't persist after mode switch
- [ ] hasLocalChanges flag still set correctly
- [ ] Error messages cleared on mode change
- [ ] No functional regressions

---

## Task 28e: Fetch PID Gains from /api/config

**Phase:** 6A - Polish  
**Prerequisites:** None  
**Estimated Time:** 20 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/ProcessView.tsx`

### Context and References

This addresses **Minor Issue #4** from the code review. Initial PID gains are currently hardcoded (lines 26-30: `Kc: 1.0, tau_I: 10.0, tau_D: 1.0`). The backend has actual configured gains available via `GET /api/config` endpoint, but the frontend doesn't fetch them.

**Backend endpoint:** `GET /api/config`  
**Returns:** `{ ..., pid_gains: { Kc: number, tau_I: number, tau_D: number }, ... }`

Reference: See `docs/API_REFERENCE.md` for endpoint details.

If unfamiliar with React useEffect hook or fetch API, search for "React useEffect fetch data on mount".

### Requirements

Add a useEffect hook to ProcessView component that fetches PID gains from the backend on component mount and updates the `currentPIDGains` state.

**Steps:**
1. Import useEffect from React (if not already imported)
2. Add a useEffect hook that runs only on mount (empty dependency array)
3. Inside useEffect, fetch from `/api/config` endpoint
4. Extract `pid_gains` from response
5. Update `currentPIDGains` state with fetched values (take absolute value of Kc for display)
6. Handle errors gracefully (log to console, fall back to existing hardcoded defaults)

**Error handling:**
- Use try-catch block around fetch
- On error, log to console but don't show error to user
- Keep hardcoded defaults as fallback if fetch fails

**Kc sign handling:**
- Backend stores Kc as negative (e.g., -2.0) for reverse-acting control
- Frontend displays positive value (e.g., 2.0) with separate reverseActing checkbox
- Take absolute value: `Math.abs(pid_gains.Kc)`

**Example pattern:**
```
useEffect hook that runs on mount
  Try to fetch from endpoint
    Parse JSON response
    Extract pid_gains object
    Update currentPIDGains state with abs(Kc), tau_I, tau_D
    Determine reverseActing based on original Kc sign
  Catch any errors
    Log error to console
    Keep existing hardcoded defaults
```

### Verification

Run full stack:
```bash
./scripts/dev.sh
```

Open browser DevTools Network tab and navigate to Process View.

**Check:**
1. Network tab shows successful GET request to `/api/config`
2. PID Tuning Control displays the fetched gains (not hardcoded 1.0, 10.0, 1.0)
3. If backend returns Kc=-2.0, frontend shows Kc=2.0 with "Reverse Acting" checked
4. If API is unavailable, component still renders with hardcoded defaults

**Console verification:**
- No fetch errors in console (or error logged gracefully)
- Component renders without errors

### Escalation Hints

**Escalate to Haiku if:**
- Unfamiliar with React useEffect hook lifecycle
- Fetch API usage unclear
- JSON parsing issues

**Search for these terms if stuck:**
- "React useEffect fetch on mount"
- "JavaScript fetch API tutorial"
- "React fetch data example"

### Acceptance Criteria
- [ ] useEffect hook fetches from `/api/config` on mount
- [ ] currentPIDGains state updated with fetched values
- [ ] Kc displayed as absolute value
- [ ] reverseActing state updated based on Kc sign
- [ ] Errors handled gracefully (logged, fallback to defaults)
- [ ] No breaking changes to existing functionality

---

## Task 28f: Improve Error Color Semantics

**Phase:** 6A - Polish  
**Prerequisites:** None  
**Estimated Time:** 15 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/SetpointControl.tsx`

### Context and References

This addresses **Minor Issue #5** from the code review. The error display (setpoint - level) currently uses green for positive error and red for negative error (lines 97-102). This might be counterintuitive for operators who associate green=good and red=bad.

Current logic:
- Positive error (setpoint > level) → Green
- Negative error (setpoint < level) → Red

### Requirements

Change the error display color scheme to use magnitude-based coloring instead of directional (positive/negative) coloring.

**New color scheme (magnitude-based):**
- Small error (|error| < 0.2): Gray (good control performance)
- Medium error (0.2 ≤ |error| < 0.5): Yellow/Amber (attention needed)
- Large error (|error| ≥ 0.5): Red (action needed)

**Implementation approach:**
- Calculate absolute error: `Math.abs(error)`
- Use conditional logic to determine color class
- Tailwind color classes: `text-gray-400` (small), `text-yellow-400` (medium), `text-red-400` (large)

**Keep existing:**
- Sign display (+ or -) in the formatted error value
- "Error (SP - PV):" label
- Number formatting (2 decimal places)

**Update only:** The color class applied to the error value span.

**Example logic pattern:**
```
Calculate absoluteError = Math.abs(error)
Determine color:
  If absoluteError < 0.2: use gray
  Else if absoluteError < 0.5: use yellow
  Else: use red
Apply appropriate text-[color]-400 class
```

### Verification

Run development server:
```bash
cd frontend && npm run dev
```

Navigate to Process View tab, observe Setpoint Control error display.

**Test scenarios:**
1. Set setpoint equal to level → Error ≈ 0 → Should be gray
2. Set setpoint slightly above level (0.1m difference) → Should be gray
3. Set setpoint moderately different (0.3m) → Should be yellow
4. Set setpoint very different (0.8m) → Should be red
5. Verify sign (+ or -) still displays correctly

### Escalation Hints

**Escalate to Haiku if:**
- Unclear how to apply conditional Tailwind classes
- Math.abs() usage unclear

**Search for these terms if stuck:**
- "React conditional className Tailwind"
- "JavaScript absolute value"

### Acceptance Criteria
- [ ] Small errors (< 0.2m) display in gray
- [ ] Medium errors (0.2-0.5m) display in yellow
- [ ] Large errors (≥ 0.5m) display in red
- [ ] Sign (+ or -) still displayed correctly
- [ ] Error value formatted to 2 decimal places unchanged
- [ ] No layout changes

---

## Phase 6B: Trends View Enhancement (Historical Charts)

These tasks implement Recharts visualizations for historical process data. Recharts v3.7.0 is already installed in `package.json`.

---

## Task 29a: Create useHistory Hook

**Phase:** 6B - Trends View  
**Prerequisites:** None  
**Estimated Time:** 25 minutes  
**Files:** 1 file

### File to Create
- `frontend/hooks/useHistory.ts`

### Context and References

This hook will fetch historical data from the backend `/api/history` endpoint and manage it in React state. It provides a clean abstraction for TrendsView and chart components to access historical data.

**Backend endpoint:** `GET /api/history?duration={seconds}`
- Duration range: 1-7200 seconds (1 second to 2 hours)
- Default: 3600 seconds (1 hour)
- Returns: Array of `SimulationState` objects in chronological order (oldest first)

Reference pattern: See `frontend/hooks/useWebSocket.ts` for custom hook structure.

If unfamiliar with custom React hooks, search for "React custom hooks tutorial".

### Requirements

Create a custom React hook that fetches and manages historical simulation data.

**Hook signature:**
```
function useHistory(durationSeconds: number): {
  history: SimulationState[]
  loading: boolean
  error: string | null
  refetch: () => void
}
```

**Hook responsibilities:**
1. Fetch historical data from `/api/history?duration={durationSeconds}` on mount
2. Refetch when `durationSeconds` parameter changes
3. Manage loading state while fetching
4. Handle errors gracefully
5. Provide refetch function for manual refresh

**State variables needed:**
- `history`: Array of SimulationState objects (default: empty array)
- `loading`: Boolean indicating fetch in progress (default: true)
- `error`: String error message or null (default: null)

**Implementation structure:**
1. Define state variables with useState hooks
2. Create fetch function that:
   - Sets loading to true
   - Fetches from endpoint with duration parameter
   - Parses JSON response
   - Updates history state with response array
   - Sets loading to false
   - Handles errors (sets error state, logs to console)
3. Create useEffect that calls fetch function on mount and when duration changes
4. Return object with history, loading, error, refetch

**Error handling:**
- Catch fetch errors and network failures
- Set error state with user-friendly message
- Log detailed error to console
- Don't throw errors (return error state instead)

**Dependencies array:**
- useEffect should depend on `durationSeconds` parameter
- Refetch when duration changes

### Verification

Create a test component that uses the hook:
```bash
# Add to frontend/app/page.tsx temporarily
import { useHistory } from '../hooks/useHistory'

// Inside component:
const { history, loading, error } = useHistory(3600)
console.log('History:', history.length, 'entries')
```

Run development server and check console:
```bash
cd frontend && npm run dev
```

**Expected:**
- Console shows "History: [number] entries"
- Number should be > 0 if backend has been running
- No fetch errors in Network tab
- No React errors in console

### Escalation Hints

**Escalate to Haiku if:**
- Custom hook pattern unclear
- useEffect dependency array behavior confusing
- Fetch error handling unclear

**Search for these terms if stuck:**
- "React custom hook fetch data"
- "React useEffect dependencies"
- "JavaScript fetch API error handling"

### Acceptance Criteria
- [ ] Hook fetches data on mount
- [ ] Hook refetches when duration changes
- [ ] Loading state accurate during fetch
- [ ] Errors caught and returned in error state
- [ ] refetch function works manually
- [ ] No memory leaks or infinite loops
- [ ] TypeScript types correct (no any types)

---

## Task 29b: Create LevelChart Component

**Phase:** 6B - Trends View  
**Prerequisites:** Task 29a complete  
**Estimated Time:** 30 minutes  
**Files:** 1 file

### File to Create
- `frontend/components/LevelChart.tsx`

### Context and References

Create a Recharts LineChart showing tank level and setpoint over time. This is the primary chart for operators to monitor control performance.

**Recharts documentation queried:** Used Context7 to confirm v3.7.0 patterns
- Library ID: `/recharts/recharts`
- Version installed: 3.7.0 (in package.json)
- Key note: Recharts v3 does NOT require xAxisId/yAxisId on CartesianGrid (this was documented incorrectly in some sources)

**Key Recharts components:**
- `ResponsiveContainer` - Auto-sizing wrapper
- `LineChart` - Main chart container
- `XAxis` - Time axis (bottom)
- `YAxis` - Value axis (left)
- `CartesianGrid` - Background grid lines
- `Tooltip` - Hover information display
- `Legend` - Series labels
- `Line` - Data series (two lines: level and setpoint)

Reference: See Context7 query results above for LineChart patterns.

If unfamiliar with Recharts, search for "Recharts LineChart multiple lines tutorial".

### Requirements

Create a React component that displays tank level and setpoint as two line series on a time-series chart.

**Component props:**
```
interface LevelChartProps {
  data: SimulationState[]
}
```

**Chart configuration:**
- Responsive container: 100% width, 300px height
- Two Line components:
  - Level line: Blue color (#3b82f6), dataKey="tank_level"
  - Setpoint line: Red dashed color (#ef4444), dataKey="setpoint", strokeDasharray="5 5"
- XAxis: Use "time" field, format as "MM:SS" or "HH:MM:SS"
- YAxis: Domain from 0 to 5 (tank height), label "Level (m)"
- CartesianGrid: Light gray, dashed
- Tooltip: Show time, level, setpoint values
- Legend: Show series names

**Data format:**
The `data` prop is an array of SimulationState objects:
```
[
  { time: 123.5, tank_level: 2.3, setpoint: 2.5, ... },
  { time: 124.5, tank_level: 2.35, setpoint: 2.5, ... },
  ...
]
```

**Time formatting:**
Time is in seconds (floating point). Format for display:
- If time < 3600: Format as "MM:SS"
- If time >= 3600: Format as "HH:MM:SS"
- Create helper function or use existing formatTime from utils

**Styling:**
- Dark theme consistent with rest of app
- Background: bg-gray-800
- Border: border border-gray-700
- Rounded corners: rounded-lg
- Padding: p-4

**Component structure:**
1. Import Recharts components
2. Import SimulationState type from lib/types
3. Define props interface
4. Create helper function for time formatting (or import from utils)
5. Return JSX with ResponsiveContainer wrapping LineChart
6. Configure chart components (XAxis, YAxis, Grid, Tooltip, Legend, Lines)

### Verification

Run development server:
```bash
cd frontend && npm run dev
```

Temporarily add LevelChart to ProcessView to test:
```
import LevelChart from './LevelChart'
// In ProcessView JSX, after data display:
{state && <LevelChart data={[state]} />}
```

**Check:**
- Chart renders without errors
- Two lines visible (blue for level, red dashed for setpoint)
- Axes labeled correctly
- Tooltip shows values on hover
- Legend shows "Level" and "Setpoint"
- Chart responsive (scales with window resize)

**Remove test code** from ProcessView after verification.

### Escalation Hints

**Escalate to Haiku if:**
- Recharts import errors
- Data format not matching chart expectations
- Time formatting complex

**Search for these terms if stuck:**
- "Recharts LineChart example"
- "Recharts time series XAxis"
- "JavaScript time formatting seconds"

### Acceptance Criteria
- [ ] Chart renders with sample data
- [ ] Two lines visible (level and setpoint)
- [ ] XAxis shows formatted time
- [ ] YAxis shows level values 0-5
- [ ] Tooltip displays on hover
- [ ] Legend identifies series
- [ ] Dark theme styling applied
- [ ] No TypeScript errors

---

## Task 29c: Create FlowsChart Component

**Phase:** 6B - Trends View  
**Prerequisites:** Task 29b complete  
**Estimated Time:** 25 minutes  
**Files:** 1 file

### File to Create
- `frontend/components/FlowsChart.tsx`

### Context and References

Create a Recharts LineChart showing inlet and outlet flows over time. This helps operators understand flow balance and disturbances.

Reference: Use same Recharts pattern as Task 29b (LevelChart), but with different data keys and colors.

### Requirements

Create a React component that displays inlet and outlet flows as two line series on a time-series chart.

**Component props:**
```
interface FlowsChartProps {
  data: SimulationState[]
}
```

**Chart configuration:**
- Responsive container: 100% width, 300px height
- Two Line components:
  - Inlet flow line: Cyan color (#06b6d4), dataKey="inlet_flow"
  - Outlet flow line: Orange color (#f97316), dataKey="outlet_flow"
- XAxis: Use "time" field, format same as LevelChart
- YAxis: Domain from 0 to 2 (max flow), label "Flow Rate (m³/s)"
- CartesianGrid: Light gray, dashed
- Tooltip: Show time, inlet, outlet values
- Legend: Show series names

**Data keys:**
- inlet_flow: Inlet volumetric flow rate (m³/s)
- outlet_flow: Outlet volumetric flow rate (m³/s)

**Styling:**
- Same dark theme as LevelChart
- Background: bg-gray-800
- Border, rounded, padding consistent

**Component structure:**
Follow same pattern as LevelChart:
1. Import Recharts components
2. Import SimulationState type
3. Define props interface
4. Reuse or create time formatting helper
5. Return JSX with chart configuration

**Differences from LevelChart:**
- Different dataKey values (inlet_flow, outlet_flow vs tank_level, setpoint)
- Different colors (cyan/orange vs blue/red)
- Different Y-axis domain (0-2 vs 0-5)
- Different Y-axis label ("Flow Rate (m³/s)" vs "Level (m)")

### Verification

Run development server:
```bash
cd frontend && npm run dev
```

Temporarily test in ProcessView:
```
import FlowsChart from './FlowsChart'
{state && <FlowsChart data={[state]} />}
```

**Check:**
- Chart renders with two flow lines
- Cyan line for inlet, orange for outlet
- Y-axis range 0-2 m³/s
- Tooltip shows flow values
- Legend correct

Remove test code after verification.

### Escalation Hints

**Escalate to Haiku if:**
- Pattern from LevelChart unclear
- Data key names don't match

**Search for these terms if stuck:**
- "Recharts multiple line series"
- "Recharts YAxis domain"

### Acceptance Criteria
- [ ] Chart renders with flow data
- [ ] Two lines visible (inlet cyan, outlet orange)
- [ ] Y-axis range 0-2 m³/s
- [ ] Tooltip displays flow values
- [ ] Legend identifies series
- [ ] Styling consistent with LevelChart
- [ ] No errors

---

## Task 29d: Create ValveChart Component

**Phase:** 6B - Trends View  
**Prerequisites:** Task 29b complete  
**Estimated Time:** 20 minutes  
**Files:** 1 file

### File to Create
- `frontend/components/ValveChart.tsx`

### Context and References

Create a Recharts LineChart showing valve position (controller output) over time. This shows how the PID controller is responding to level errors.

Reference: Same pattern as previous chart components, but single line instead of two.

### Requirements

Create a React component that displays valve position as a single line series.

**Component props:**
```
interface ValveChartProps {
  data: SimulationState[]
}
```

**Chart configuration:**
- Responsive container: 100% width, 250px height (slightly shorter)
- Single Line component:
  - Valve position line: Purple color (#a855f7), dataKey="valve_position"
- XAxis: Time, formatted same as other charts
- YAxis: Domain from 0 to 1 (valve range), label "Valve Position"
- CartesianGrid: Light gray, dashed
- Tooltip: Show time, valve position
- Legend: Show "Valve Position"

**Data key:**
- valve_position: Controller output from 0 (closed) to 1 (fully open)

**Styling:**
- Same dark theme as other charts
- Slightly shorter (250px vs 300px) since it's a single series

**Component structure:**
Follow same pattern as LevelChart and FlowsChart, but simpler (only one Line component).

### Verification

Run development server and test in ProcessView temporarily.

**Check:**
- Single purple line visible
- Y-axis range 0-1
- Values between 0 and 1
- Tooltip works

Remove test code after verification.

### Escalation Hints

**Escalate to Haiku if:**
- Pattern unclear from previous charts

**Search for these terms if stuck:**
- "Recharts single line chart"

### Acceptance Criteria
- [ ] Chart renders with valve position data
- [ ] Purple line visible
- [ ] Y-axis range 0-1
- [ ] Tooltip displays valve position
- [ ] Styling consistent
- [ ] No errors

---

## Task 29e: Integrate Charts into TrendsView

**Phase:** 6B - Trends View  
**Prerequisites:** Tasks 29a, 29b, 29c, 29d complete  
**Estimated Time:** 25 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/TrendsView.tsx`

### Context and References

Replace the placeholder table in TrendsView with the three chart components, using the useHistory hook to fetch historical data.

Current TrendsView (Phase 5) shows only a table with last 10 WebSocket updates. This task replaces it with historical charts.

### Requirements

Modify TrendsView component to display historical data charts instead of the table.

**Changes needed:**

1. **Import chart components:**
   - LevelChart from './LevelChart'
   - FlowsChart from './FlowsChart'
   - ValveChart from './ValveChart'

2. **Import useHistory hook:**
   - useHistory from '../hooks/useHistory'

3. **Replace useSimulation with useHistory:**
   - Remove: `const { history } = useSimulation()`
   - Add: `const { history, loading, error } = useHistory(3600)` (default 1 hour)

4. **Update component layout:**
   - Remove placeholder blue message box
   - Remove table display
   - Add three chart sections vertically stacked
   - Each chart in its own div with heading

**New JSX structure:**
```
Header section (keep existing)

Loading state (if loading):
  "Loading historical data..."

Error state (if error):
  Display error message in red

Main content (if history.length > 0):
  Section 1: Level Chart
    Heading: "Tank Level vs Setpoint"
    LevelChart component with history data
  
  Section 2: Flow Rates Chart
    Heading: "Inlet and Outlet Flows"
    FlowsChart component with history data
  
  Section 3: Valve Position Chart
    Heading: "Controller Output (Valve Position)"
    ValveChart component with history data

Empty state (if history.length === 0 and not loading):
  "No historical data available"
```

**Styling:**
- Each chart section in `bg-gray-800 rounded-lg p-4 mb-4`
- Chart headings: `text-lg font-semibold text-white mb-3`
- Loading/error messages centered
- Maintain dark theme consistency

**Remove:**
- Old table HTML
- Old history.length === 0 check based on WebSocket history
- Placeholder message about Phase 4

### Verification

Run full stack:
```bash
./scripts/dev.sh
```

Navigate to Trends View tab.

**Check:**
1. Three charts display vertically
2. Each chart shows historical data (if backend has data)
3. If no data yet, shows "No historical data available"
4. While loading, shows loading message
5. Charts update as backend accumulates more data
6. No console errors

**Test with fresh backend:**
- Start backend fresh
- Navigate to Trends View immediately
- Should show "No historical data available"
- Wait 30 seconds
- Refresh page
- Should now show charts with ~30 data points

### Escalation Hints

**Escalate to Haiku if:**
- Component composition unclear
- Conditional rendering logic complex

**Search for these terms if stuck:**
- "React conditional rendering"
- "React component composition"

### Acceptance Criteria
- [ ] Three charts visible in Trends View
- [ ] Charts display historical data from useHistory hook
- [ ] Loading state shows while fetching
- [ ] Error state displays if fetch fails
- [ ] Empty state shows if no data
- [ ] Table display removed
- [ ] Placeholder message removed
- [ ] No console errors

---

## Task 29f: Add Real-Time Data Append to Charts

**Phase:** 6B - Trends View  
**Prerequisites:** Task 29e complete  
**Estimated Time:** 25 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/TrendsView.tsx`

### Context and References

Currently, charts show historical data fetched once on mount. This task adds real-time updates by appending new WebSocket state updates to the historical data, creating a continuously updating chart.

Pattern: Combine fetched historical data with real-time WebSocket updates.

### Requirements

Modify TrendsView to append real-time WebSocket state updates to the historical data array shown in charts.

**Changes needed:**

1. **Import useSimulation:**
   - Add: `const { state } = useSimulation()` (in addition to existing useHistory)
   - This provides real-time WebSocket updates

2. **Create combined data array:**
   - Use useMemo or state to combine historical data with real-time updates
   - Start with historical data from useHistory
   - Append new state updates from WebSocket

3. **Append logic:**
   - When new `state` arrives from WebSocket, add to end of history array
   - Prevent duplicates (check if state.time already exists in array)
   - Limit total length (keep last 7200 entries = 2 hours at 1Hz)

**Implementation approach:**

Create a local state that combines both data sources:
- Initialize with history from useHistory
- Use useEffect to watch for new state from WebSocket
- When state updates, append if time is newer than last entry
- Trim array if exceeds 7200 entries (oldest entries removed)

**Example pattern:**
```
const { history, loading, error } = useHistory(3600)
const { state } = useSimulation()
const [chartData, setChartData] = useState([])

useEffect to initialize chartData from history when history loads

useEffect to append new state when it arrives:
  Check if state is newer than last chartData entry
  If yes, append to chartData
  Trim to last 7200 entries
```

**Data flow:**
1. Component mounts → useHistory fetches historical data (3600 seconds)
2. Historical data → chartData state
3. WebSocket provides new state every second → append to chartData
4. Charts render with chartData (growing over time)

### Verification

Run full stack:
```bash
./scripts/dev.sh
```

Navigate to Trends View tab and observe charts.

**Check:**
1. Charts initially show historical data
2. Charts update every second with new data point
3. Lines extend to the right as time progresses
4. No jumps or discontinuities in data
5. Console shows no duplicate entries or errors

**Test continuous operation:**
- Leave Trends View open for 2 minutes
- Verify charts show continuous data from start to current time
- Change setpoint in Process View
- Verify Trends View charts update to show the change

### Escalation Hints

**Escalate to Haiku if:**
- Combining two data sources unclear
- useEffect for appending logic complex
- Array manipulation issues

**Search for these terms if stuck:**
- "React combine multiple data sources"
- "React useEffect append to array"
- "JavaScript array push and slice"

### Acceptance Criteria
- [ ] Charts show historical data on mount
- [ ] Charts update in real-time (every second)
- [ ] No duplicate data points
- [ ] Array limited to 7200 entries max
- [ ] Smooth continuous lines (no gaps)
- [ ] Changes in Process View reflected in Trends View
- [ ] No performance issues or memory leaks

---

## Task 29g: Add Time Range Selector

**Phase:** 6B - Trends View  
**Prerequisites:** Task 29f complete  
**Estimated Time:** 20 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/TrendsView.tsx`

### Context and References

Add a time range selector to TrendsView that allows users to choose how much historical data to display: 1 minute, 5 minutes, 30 minutes, 1 hour, or 2 hours.

Currently, useHistory is hardcoded to 3600 seconds (1 hour). This task makes it configurable.

### Requirements

Add a time range selector UI control above the charts that changes the duration parameter passed to useHistory hook.

**UI component:**
- Radio buttons or button group for time range selection
- Options: "1 min" (60s), "5 min" (300s), "30 min" (1800s), "1 hr" (3600s), "2 hr" (7200s)
- Default selected: 1 hr (3600s)
- Styled consistently with dark theme

**Changes needed:**

1. **Add state for selected duration:**
   - `const [duration, setDuration] = useState(3600)`

2. **Update useHistory call:**
   - Change `useHistory(3600)` to `useHistory(duration)`

3. **Add selector UI:**
   - Position: Below header, above charts
   - Layout: Horizontal row of buttons
   - Active button: Highlighted with blue background
   - Inactive buttons: Gray background, hover effect

**Button styling (Tailwind):**
- Container: `flex gap-2 mb-6`
- Active button: `bg-blue-600 text-white`
- Inactive button: `bg-gray-700 text-gray-300 hover:bg-gray-600`
- All buttons: `px-4 py-2 rounded font-medium transition-colors cursor-pointer`

**Implementation pattern:**
```
Create duration state
Create array of time range options: [60, 300, 1800, 3600, 7200]
Create labels: ["1 min", "5 min", "30 min", "1 hr", "2 hr"]

Render button group:
  For each time range option:
    Button with label
    onClick sets duration state
    Conditional styling (active vs inactive)

Pass duration to useHistory(duration)
```

### Verification

Run development server:
```bash
cd frontend && npm run dev
```

Navigate to Trends View tab.

**Check:**
1. Five time range buttons visible
2. "1 hr" button highlighted by default
3. Clicking different buttons changes duration
4. Charts refetch with new duration
5. Active button styling updates on click
6. Charts display appropriate time range

**Test sequence:**
- Click "1 min" → Charts show last 60 seconds
- Click "2 hr" → Charts show last 2 hours (if available)
- Click "30 min" → Charts show last 30 minutes

### Escalation Hints

**Escalate to Haiku if:**
- Button group styling unclear
- State management for active selection confusing

**Search for these terms if stuck:**
- "React button group active state"
- "Tailwind CSS button group"

### Acceptance Criteria
- [ ] Five time range buttons visible
- [ ] Default selection is "1 hr"
- [ ] Clicking buttons changes selected duration
- [ ] Active button visually highlighted
- [ ] Charts refetch with new duration
- [ ] Styling consistent with dark theme
- [ ] No console errors

---

## Task 29h: Add Chart Interactions

**Phase:** 6B - Trends View  
**Prerequisites:** Tasks 29b, 29c, 29d complete  
**Estimated Time:** 25 minutes  
**Files:** 3 files

### Files to Modify
- `frontend/components/LevelChart.tsx`
- `frontend/components/FlowsChart.tsx`
- `frontend/components/ValveChart.tsx`

### Context and References

Enhance chart components with better tooltips and interactive legend toggles that allow hiding/showing individual lines.

Recharts provides built-in support for interactive legends and custom tooltips.

Reference: See Context7 query results for interactive legend pattern.

### Requirements

Add two enhancements to each chart component:

**Enhancement 1: Custom Tooltip**
- Replace default Tooltip with custom styled version
- Dark theme tooltip (dark background, white text, border)
- Show formatted values with units
- Display time in readable format

**Enhancement 2: Interactive Legend**
- Allow clicking legend items to toggle line visibility
- Use React state to track which lines are hidden
- Apply `hide` prop to Line components based on state

**For each chart component:**

1. **Add state for hidden lines:**
   - LevelChart: Track visibility of "tank_level" and "setpoint"
   - FlowsChart: Track visibility of "inlet_flow" and "outlet_flow"
   - ValveChart: Only one line, but still add for consistency

2. **Create custom Tooltip component:**
   - Function component accepting `active`, `payload`, `label` props
   - Return null if not active
   - Display dark-themed box with formatted values
   - Include units in display (m for level, m³/s for flow, unitless for valve)

3. **Add onClick handler to Legend:**
   - Handle legend click to toggle line visibility
   - Update hidden state
   - Style legend with cursor pointer

4. **Apply hide prop to Line components:**
   - Based on hidden state

**Example pattern for custom tooltip:**
```
Custom tooltip component:
  If not active, return null
  Return div with:
    Dark background (bg-gray-900)
    White text
    Border (border-gray-600)
    Padding and rounded corners
    Display label (time)
    Map over payload to display each series value with unit
```

**Example pattern for interactive legend:**
```
State: hiddenLines object { dataKey: boolean }
Handler: Toggle boolean for clicked dataKey
Legend: Add onClick prop with handler
Line: Add hide prop based on hiddenLines[dataKey]
```

### Verification

Run development server:
```bash
cd frontend && npm run dev
```

Navigate to Trends View tab.

**Test custom tooltips:**
1. Hover over each chart
2. Verify dark-themed tooltip appears
3. Verify values show correct units
4. Verify time formatted readably

**Test interactive legends:**
1. Click "Level" in legend → Level line hides
2. Click "Setpoint" in legend → Setpoint line hides
3. Click both → Both lines hide, chart axes remain
4. Click again → Lines reappear
5. Repeat for other charts

### Escalation Hints

**Escalate to Haiku if:**
- Custom tooltip pattern unclear
- State management for legend toggles complex
- Recharts Tooltip/Legend props confusing

**Search for these terms if stuck:**
- "Recharts custom tooltip"
- "Recharts interactive legend toggle"
- "Recharts hide line"

### Acceptance Criteria
- [ ] All charts have custom dark-themed tooltips
- [ ] Tooltips show formatted values with units
- [ ] Clicking legend items toggles line visibility
- [ ] Legend cursor changes to pointer on hover
- [ ] Hidden lines don't appear on chart
- [ ] Charts remain functional when all lines hidden
- [ ] Styling consistent across all charts
- [ ] No console errors

---

## Summary of Phase 6 Tasks

### Phase 6A: Polish (6 tasks, ~75 minutes)
| Task | Description | Time | File |
|------|-------------|------|------|
| 28a | Conditional flow animation | 5 min | TankGraphic.tsx |
| 28b | Extract SVG magic numbers | 15 min | TankGraphic.tsx |
| 28c | Add Reverse Acting help text | 10 min | PIDTuningControl.tsx |
| 28d | Reset Brownian params on mode switch | 10 min | InletFlowControl.tsx |
| 28e | Fetch PID gains from /api/config | 20 min | ProcessView.tsx |
| 28f | Improve error color semantics | 15 min | SetpointControl.tsx |

### Phase 6B: Trends View (8 tasks, ~3 hours 35 minutes)
| Task | Description | Time | File |
|------|-------------|------|------|
| 29a | Create useHistory hook | 25 min | hooks/useHistory.ts (NEW) |
| 29b | Create LevelChart component | 30 min | LevelChart.tsx (NEW) |
| 29c | Create FlowsChart component | 25 min | FlowsChart.tsx (NEW) |
| 29d | Create ValveChart component | 20 min | ValveChart.tsx (NEW) |
| 29e | Integrate charts into TrendsView | 25 min | TrendsView.tsx |
| 29f | Add real-time data append | 25 min | TrendsView.tsx |
| 29g | Add time range selector | 20 min | TrendsView.tsx |
| 29h | Add chart interactions | 25 min | 3 chart files |

**Total: 14 tasks, ~4 hours 50 minutes estimated**

---

## Upcoming Work (After Phase 6 Complete)

### Phase 7: Integration and Polish
- End-to-end testing with Playwright
- Performance profiling and optimization
- Error boundary components
- Loading states and skeleton screens
- Data export functionality (CSV download)
- Advanced chart features (zoom, pan, download as image)
- Alarm thresholds and notifications
- System identification tools

### Phase 8: Deployment and Documentation
- Production build optimization
- Docker containerization
- Deployment guide updates
- User manual creation
- Video tutorials

---

## Development Workflow Notes

### Context Preservation

When working on chart components (Tasks 29b-29d):
- Each chart follows the same pattern
- Reference LevelChart when creating FlowsChart and ValveChart
- Reuse time formatting logic across all charts
- Keep consistent styling (dark theme, colors, spacing)

### Testing Strategy

**After each polish task (28a-28f):**
- Run `npm run dev`
- Navigate to Process View tab
- Verify specific component changed
- Check for regressions in other components

**After each Trends View task (29a-29h):**
- Run `./scripts/dev.sh` (full stack)
- Navigate to Trends View tab
- Verify chart functionality
- Check browser console for errors

### Git Workflow

Commit after each completed task:
```bash
git add [modified files]
git commit -m "Task [number]: [brief description]"
```

Example commit messages:
- `Task 28a: Add conditional flow animation to TankGraphic`
- `Task 29b: Create LevelChart component with Recharts`
- `Task 29g: Add time range selector to TrendsView`

### Recharts Version Notes

**Important:** This project uses Recharts v3.7.0, which has breaking changes from v2.x:

1. **CartesianGrid does NOT require xAxisId/yAxisId** (some docs incorrectly say it does)
2. **Custom components** can be nested directly (no Customized wrapper needed)
3. **Tooltip/Legend** API unchanged from v2
4. **Line hide prop** works the same as v2

If documentation seems contradictory, trust the official Recharts v3 migration guide and the Context7 query results provided in tasks.

---

## Key Principles for Local LLM Success

### 1. Task Independence
Each task can be completed without referencing others (except stated prerequisites). Charts share patterns but are separate files.

### 2. Reference-First Design
Tasks provide:
- Links to existing patterns (LevelChart for other charts)
- Search keywords for external research
- Context7 query results for Recharts

### 3. Escalation Clarity
Every task specifies:
- When to escalate (unfamiliar patterns, repeated errors)
- What to search (specific keywords)
- Simpler alternatives if stuck

### 4. Verification at Scale
Each task has:
- Exact command to verify
- Expected outcome described
- Visual checks for UI tasks

### 5. Structure Over Flexibility
Tasks describe:
- Component structure (sections, props, hooks)
- Styling classes (exact Tailwind classes)
- Implementation patterns (not code, but clear guidance)

---

## Notes on Architecture Decisions

### Why useHistory Hook Instead of Extending useSimulation?

**Decision:** Create separate useHistory hook rather than adding history fetching to useSimulation.

**Reasoning:**
- useSimulation provides real-time WebSocket data (last 10 entries)
- useHistory provides bulk historical data (up to 7200 entries)
- Separation of concerns: real-time vs historical
- TrendsView needs both: historical baseline + real-time append
- ProcessView doesn't need historical data (uses current state only)

### Why Three Separate Chart Components?

**Decision:** Create LevelChart, FlowsChart, ValveChart as separate components instead of one configurable chart.

**Reasoning:**
- Each chart has different data keys, colors, Y-axis ranges
- Separate components are easier to understand for local LLMs
- Future customization per chart type (e.g., reference lines on level chart)
- Clear task boundaries (one chart per task)
- Pattern reuse is explicit (copy and modify)

### Why Fetch Historical Data Instead of Building from WebSocket?

**Decision:** Fetch from /api/history on mount rather than accumulating from WebSocket over time.

**Reasoning:**
- Immediate data availability (don't wait to accumulate)
- Works after page refresh (data preserved on backend)
- Backend ring buffer stores 2 hours (7200 entries at 1Hz)
- WebSocket only provides last 10 entries
- Operator needs historical context immediately when opening Trends View

### Why Time Range Selector Instead of Zoom/Pan?

**Decision:** Implement time range selector (preset buttons) before zoom/pan controls.

**Reasoning:**
- Simpler to implement (state + refetch)
- Common use case: "show me last 5 minutes"
- Preset ranges clearer for operators than free-form zoom
- Zoom/pan can be added later (Task 29h enhancement or Phase 7)
- Works well with backend duration parameter

---

**End of Phase 6 Task Breakdown**

Next steps after Phase 6 complete:
1. Senior Engineer reviews Phase 6 implementation
2. Code Reviewer provides feedback
3. Senior Engineer creates Phase 7 task breakdown
