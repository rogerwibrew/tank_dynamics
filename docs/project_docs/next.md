# Next Tasks - Tank Dynamics Simulator

## Current Phase: Phase 5 - Process View (SCADA Interface)

**Phase Goal:** Build out the Process View tab with tank visualization, live data displays, and operator controls.

**What's Complete:**
- ✅ Phase 4: Next.js foundation (layout, routing, WebSocket integration, placeholders)
- ✅ ProcessView placeholder component with data display
- ✅ SimulationState type definitions matching backend

**What's Next:**
- Build SVG tank visualization with animated liquid level
- Create control input components (setpoint, PID tuning, inlet flow)
- Integrate controls with WebSocket commands
- Add flow direction indicators
- Add status badges (auto/manual, mode indicators)

---

## Task 22a: Create TankGraphic SVG Component

**Phase:** Phase 5 - Process View  
**Prerequisites:** Phase 4 complete  
**Estimated Time:** 20-30 minutes  
**Files:** 1 file

### File to Create
- `frontend/components/TankGraphic.tsx`

### Context and References

This component renders an SVG representation of the tank with animated liquid level. The tank should look like a SCADA graphic: industrial, clear, with visible level indication.

**Visual design:**
- Tank outline (rectangular container)
- Animated liquid fill (blue, height based on tank_level)
- Level markers on side (0m, 1m, 2m, 3m, 4m, 5m)
- Setpoint indicator line (horizontal dashed line)
- Inlet pipe at top
- Outlet pipe at bottom with valve symbol

**Reference:**
- SVG basics: https://developer.mozilla.org/en-US/docs/Web/SVG
- Search: "React SVG component" if unfamiliar
- Search: "CSS animations SVG" for smooth level transitions

### Requirements

Create a React component that accepts props for tank_level, setpoint, and tank_height (from config).

**Component structure:**
- Function component accepting props: `{ level: number, setpoint: number, maxHeight: number }`
- Returns SVG element with viewBox for responsive scaling
- Use Tailwind classes for colors where possible
- Add smooth CSS transition for level changes (transition-all duration-500)

**SVG elements to include:**
1. Tank outline (stroke="#4b5563", fill="none", strokeWidth="3")
2. Liquid fill rectangle (fill="#3b82f6", height calculated from level/maxHeight ratio)
3. Scale marks on left side (horizontal lines at 20% intervals)
4. Scale labels (text showing 0, 1, 2, 3, 4, 5 meters)
5. Setpoint line (stroke="#ef4444", strokeDasharray="5,5")
6. Inlet pipe graphic at top
7. Outlet pipe graphic at bottom

**Level calculation:**
```
Percentage = (level / maxHeight) * 100
If tank is 5m tall and level is 2.5m, liquid fills 50% of tank height
```

**Responsive sizing:**
- Use viewBox="0 0 400 600" for consistent aspect ratio
- Parent div should constrain width/height
- Component should scale proportionally

### Verification

1. Create the component file
2. The component should export a default function
3. Component should accept level, setpoint, maxHeight as props
4. SVG should have viewBox attribute for responsive scaling
5. Should include all required visual elements

To test rendering (in Task 22b):
```bash
cd frontend && npm run dev
```

Open http://localhost:3000, switch to Process tab, and verify tank graphic appears.

### Escalation Hints

**Escalate to Haiku if:**
- SVG coordinate system is confusing after reading MDN docs
- Unclear how to calculate liquid rectangle height
- Trouble with responsive scaling

**Search for these terms if stuck:**
- "React SVG component props"
- "SVG viewBox responsive"
- "CSS transition SVG attributes"

### Acceptance Criteria
- [ ] File created at `frontend/components/TankGraphic.tsx`
- [ ] Component accepts level, setpoint, maxHeight props
- [ ] SVG includes tank outline, liquid fill, scale marks, setpoint line
- [ ] Uses Tailwind classes for colors
- [ ] Includes transition-all for smooth animation
- [ ] Exports default function

---

## Task 22b: Integrate TankGraphic into ProcessView

**Phase:** Phase 5 - Process View  
**Prerequisites:** Task 22a  
**Estimated Time:** 15-20 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/ProcessView.tsx`

### Context and References

Replace the current data-only display with a two-column layout: tank graphic on left, live data on right.

**Reference existing code:**
- ProcessView already imports useSimulation hook
- Already has null check for state
- Already has data display grid

### Requirements

Modify ProcessView to use a two-column grid layout when state is available.

**Left column:**
- Import TankGraphic component
- Render `<TankGraphic level={state.tank_level} setpoint={state.setpoint} maxHeight={5.0} />`
- Constrain width to reasonable size (max-w-md)

**Right column:**
- Keep existing data display grid
- All current functionality preserved (time, levels, flows, valve position)

**Layout structure:**
```
When state !== null:
  <div className="grid grid-cols-1 lg:grid-cols-2 gap-8">
    {/* Left: Tank Graphic */}
    <div className="flex justify-center">
      <div className="w-full max-w-md">
        <TankGraphic ... />
      </div>
    </div>
    
    {/* Right: Data Display */}
    <div className="bg-gray-800 rounded-lg p-6 space-y-4">
      {/* Existing data grids */}
    </div>
  </div>
```

### Verification

Run the dev server:
```bash
cd frontend && npm run dev
```

Expected result:
- Process tab shows tank graphic on left, data on right
- Tank graphic renders with blue liquid
- Liquid level animates when simulation updates
- Layout responsive (stacks vertically on narrow screens due to lg: prefix)

### Escalation Hints

**Escalate if:**
- Import path errors persist after checking file location
- Tailwind grid layout not working as expected

**Search terms:**
- "Tailwind CSS grid two columns"
- "React component import error"

### Acceptance Criteria
- [ ] ProcessView imports TankGraphic component
- [ ] Two-column grid layout when state is available
- [ ] TankGraphic renders in left column with max-w-md constraint
- [ ] Existing data display preserved in right column
- [ ] Responsive layout (lg:grid-cols-2 for large screens)
- [ ] Dev server runs without errors

---

## Task 23a: Create SetpointControl Component

**Phase:** Phase 5 - Process View  
**Prerequisites:** Task 22b  
**Estimated Time:** 20-25 minutes  
**Files:** 1 file

### File to Create
- `frontend/components/SetpointControl.tsx`

### Context and References

Create a control panel input for operators to change the tank level setpoint. This is a critical SCADA control - it should be clear, immediate, and prevent invalid inputs.

**Design pattern:**
- Number input with +/- buttons for fine adjustment
- Direct text input for jumping to specific value
- Validation (0.0 to 5.0 meters)
- Visual feedback on current value vs. actual level

**Reference:**
- React controlled inputs: https://react.dev/reference/react-dom/components/input
- Search: "React number input validation" if needed

### Requirements

Create a component that accepts current setpoint, current level, and an onChange callback.

**Props interface:**
```
{
  currentSetpoint: number,
  currentLevel: number,
  onSetpointChange: (newValue: number) => void
}
```

**Component structure:**
- Display label "Setpoint Control"
- Show current setpoint value prominently
- Number input (type="number", min="0", max="5", step="0.1")
- Two buttons: Increment (+0.1) and Decrement (-0.1)
- Visual indicator showing error (setpoint - level)
- Disable buttons when at limits (can't go below 0 or above 5)

**Validation:**
- Clamp input to [0.0, 5.0] range
- Round to 1 decimal place
- Call onChange only with valid values

**Styling:**
- Dark theme consistent with existing components
- Input: bg-gray-700, text-white, border-gray-600
- Buttons: bg-blue-600 hover:bg-blue-700 disabled:bg-gray-700 disabled:cursor-not-allowed
- Error display: text-sm, text-gray-400, shows (SP - PV) value

### Verification

Component should export a default function accepting the three props.

To test (in Task 23b):
```bash
cd frontend && npm run dev
```

Visual check:
- Input displays current setpoint
- Buttons increment/decrement by 0.1
- Buttons disable at limits
- Error calculation shows correct value

### Escalation Hints

**Escalate if:**
- Controlled input pattern is unfamiliar after reading React docs
- Validation logic unclear
- Number formatting issues

**Search terms:**
- "React controlled input onChange"
- "JavaScript number toFixed"
- "React button disabled state"

### Acceptance Criteria
- [ ] File created at `frontend/components/SetpointControl.tsx`
- [ ] Component accepts currentSetpoint, currentLevel, onSetpointChange props
- [ ] Number input with min=0, max=5, step=0.1
- [ ] Increment and decrement buttons
- [ ] Buttons disabled at appropriate limits
- [ ] Error display (setpoint - level)
- [ ] Validation clamps to valid range
- [ ] Exports default function

---

## Task 23b: Integrate SetpointControl into ProcessView

**Phase:** Phase 5 - Process View  
**Prerequisites:** Task 23a  
**Estimated Time:** 15-20 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/ProcessView.tsx`

### Context and References

Add SetpointControl to the right column below the data display. Connect it to the WebSocket to send setpoint commands.

**Reference existing code:**
- ProcessView already has access to `state` from useSimulation
- Need to import useSimulation and get send function
- WebSocket message format defined in lib/types.ts

### Requirements

**Import changes:**
- Import SetpointControl component
- useSimulation hook already imported, but ensure we destructure `send` function

**Callback function:**
Create handleSetpointChange function:
```
Parameters: newValue (number)
Action: Send WebSocket message with type "setpoint" and value
Message format must match WebSocketMessage type from lib/types.ts
```

**Integration:**
Add SetpointControl below the existing data display grid in the right column.

Place it after the valve position section:
```
<div className="bg-gray-800 rounded-lg p-6 space-y-4">
  {/* Existing data display grids */}
  
  {/* NEW: Setpoint Control */}
  <div className="pt-4 border-t border-gray-700">
    <SetpointControl
      currentSetpoint={state.setpoint}
      currentLevel={state.tank_level}
      onSetpointChange={handleSetpointChange}
    />
  </div>
</div>
```

### Verification

Run dev server:
```bash
cd frontend && npm run dev
```

**Test steps:**
1. Open http://localhost:3000
2. Verify setpoint control appears in Process tab
3. Change setpoint value
4. Check browser console for WebSocket message sent
5. (If backend running) Verify tank level responds to new setpoint

Expected WebSocket message format:
```json
{"type": "setpoint", "value": 3.5}
```

### Escalation Hints

**Escalate if:**
- WebSocket send function undefined (check providers.tsx)
- Message format unclear despite types.ts reference
- Callback not triggering

**Search terms:**
- "React callback function props"
- "WebSocket send JSON"

### Acceptance Criteria
- [ ] SetpointControl imported in ProcessView
- [ ] handleSetpointChange function created
- [ ] Function sends correct WebSocket message format
- [ ] Component integrated below data display
- [ ] Border separator added (border-t border-gray-700)
- [ ] Dev server runs without errors
- [ ] Setpoint control visible and functional

---

## Task 24a: Create PIDTuningControl Component

**Phase:** Phase 5 - Process View  
**Prerequisites:** Task 23b  
**Estimated Time:** 25-30 minutes  
**Files:** 1 file

### File to Create
- `frontend/components/PIDTuningControl.tsx`

### Context and References

Create a control panel for operators to tune PID controller parameters: Kc (proportional gain), tau_I (integral time), tau_D (derivative time).

**Design considerations:**
- Three separate number inputs
- Labels explaining each parameter
- Validation ranges (Kc can be negative, tau_I/tau_D must be >= 0)
- Apply button to send all three values together
- Display current values prominently

**Reference:**
- PID control basics (if unfamiliar): Search "PID controller parameters meaning"
- React forms: https://react.dev/reference/react-dom/components/input

### Requirements

Create component accepting current PID gains and onChange callback.

**Props interface:**
```
{
  currentGains: { Kc: number, tau_I: number, tau_D: number },
  onGainsChange: (newGains: { Kc: number, tau_I: number, tau_D: number }) => void
}
```

**Component structure:**
- Heading: "PID Tuning"
- Three number inputs with labels:
  - Kc: "Proportional Gain" (no min, no max, step="0.1")
  - tau_I: "Integral Time (s)" (min="0", step="1.0")
  - tau_D: "Derivative Time (s)" (min="0", step="0.1")
- Apply button to submit all changes
- Local state to track pending changes before Apply clicked

**Behavior:**
- Initialize local state from currentGains prop
- Update local state as user types
- When Apply clicked, call onGainsChange with local state
- Validate: tau_I >= 0, tau_D >= 0 (Kc can be negative)

**Styling:**
- Consistent with SetpointControl
- Labels: text-xs font-semibold text-gray-400 uppercase
- Inputs: bg-gray-700 text-white border-gray-600
- Apply button: bg-green-600 hover:bg-green-700

### Verification

Component should export default function accepting props.

Visual structure:
```
PID Tuning
├── Kc input with label
├── tau_I input with label
├── tau_D input with label
└── Apply button
```

To test (in Task 24b):
```bash
cd frontend && npm run dev
```

### Escalation Hints

**Escalate if:**
- Managing local state for three inputs is unclear
- Validation logic confusing
- Form submission pattern unfamiliar

**Search terms:**
- "React multiple controlled inputs"
- "React form local state"
- "JavaScript object spread syntax"

### Acceptance Criteria
- [ ] File created at `frontend/components/PIDTuningControl.tsx`
- [ ] Component accepts currentGains and onGainsChange props
- [ ] Three number inputs (Kc, tau_I, tau_D)
- [ ] Local state tracks pending changes
- [ ] Apply button sends changes via callback
- [ ] Validation prevents negative tau_I and tau_D
- [ ] Labels clearly identify each parameter
- [ ] Exports default function

---

## Task 24b: Integrate PIDTuningControl into ProcessView

**Phase:** Phase 5 - Process View  
**Prerequisites:** Task 24a  
**Estimated Time:** 15-20 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/ProcessView.tsx`

### Context and References

Add PID tuning control to the controls section below setpoint control.

**Backend protocol:**
WebSocket message type "pid" with Kc, tau_I, tau_D fields (see lib/types.ts)

### Requirements

**Import:**
- Import PIDTuningControl component

**Callback function:**
Create handlePIDChange function:
```
Parameters: newGains object with Kc, tau_I, tau_D
Action: Send WebSocket message with type "pid"
```

**Integration:**
Add below SetpointControl in the controls section:
```
<div className="pt-4 border-t border-gray-700">
  <SetpointControl ... />
</div>

<div className="pt-4 border-t border-gray-700">
  <PIDTuningControl
    currentGains={{
      Kc: /* extract from state or config */,
      tau_I: /* extract from state or config */,
      tau_D: /* extract from state or config */
    }}
    onGainsChange={handlePIDChange}
  />
</div>
```

**Note on current gains:**
The SimulationState doesn't include current PID gains. For now, pass hardcoded initial values:
```
currentGains={{ Kc: 2.0, tau_I: 10.0, tau_D: 0.0 }}
```

This will be improved in Phase 7 when we add config fetching.

### Verification

Run dev server:
```bash
cd frontend && npm run dev
```

**Test:**
1. Open Process tab
2. Verify PID tuning section appears
3. Change Kc, tau_I, tau_D values
4. Click Apply button
5. Check console for WebSocket message

Expected message format:
```json
{"type": "pid", "Kc": 2.5, "tau_I": 15.0, "tau_D": 0.0}
```

### Escalation Hints

**Escalate if:**
- Unclear how to structure PID gains object
- WebSocket message format doesn't match types
- Component not rendering

**Search terms:**
- "React object destructuring props"
- "TypeScript interface object literal"

### Acceptance Criteria
- [ ] PIDTuningControl imported
- [ ] handlePIDChange function created
- [ ] Function sends correct WebSocket message (type: "pid")
- [ ] Component integrated below SetpointControl
- [ ] Border separator added
- [ ] Initial gains hardcoded (Kc: 2.0, tau_I: 10.0, tau_D: 0.0)
- [ ] Dev server runs without errors
- [ ] PID control visible and Apply button functional

---

## Task 25a: Create InletFlowControl Component

**Phase:** Phase 5 - Process View  
**Prerequisites:** Task 24b  
**Estimated Time:** 20-25 minutes  
**Files:** 1 file

### File to Create
- `frontend/components/InletFlowControl.tsx`

### Context and References

Create control for inlet flow rate with two modes:
1. **Constant mode:** Operator sets exact flow rate
2. **Brownian mode:** Random walk between min/max bounds

**Design:**
- Mode selector (radio buttons or toggle)
- In constant mode: single flow rate input
- In Brownian mode: min, max, variance inputs
- Apply button sends appropriate WebSocket command

**Reference:**
- Radio buttons: https://developer.mozilla.org/en-US/docs/Web/HTML/Element/input/radio
- Search: "React radio button controlled component"

### Requirements

Create component accepting current mode, current flow, and two callbacks.

**Props interface:**
```
{
  currentFlow: number,
  onFlowChange: (value: number) => void,
  onModeChange: (mode: "constant" | "brownian", config?: { min: number, max: number, variance: number }) => void
}
```

**Component structure:**
- Heading: "Inlet Flow Control"
- Mode selector: Radio buttons for "Constant" and "Brownian"
- Conditional rendering based on mode:
  - Constant: Single input for flow rate (0.0 to 2.0 m³/s)
  - Brownian: Three inputs for min, max, variance
- Apply button to submit

**Constant mode:**
- Flow rate input (min="0", max="2.0", step="0.1")
- Apply button calls onFlowChange(value)

**Brownian mode:**
- Min flow input (min="0", max="2.0", step="0.1", default="0.8")
- Max flow input (min="0", max="2.0", step="0.1", default="1.2")
- Variance input (min="0", max="1.0", step="0.01", default="0.05")
- Apply button calls onModeChange("brownian", {min, max, variance})
- Validation: max must be > min

**Local state:**
Track mode selection and current input values before Apply clicked.

### Verification

Component exports default function.

Structure by mode:
```
Constant mode:
├── Mode selector (Constant selected)
├── Flow rate input
└── Apply button

Brownian mode:
├── Mode selector (Brownian selected)
├── Min input
├── Max input
├── Variance input
└── Apply button
```

To test (in Task 25b):
```bash
cd frontend && npm run dev
```

### Escalation Hints

**Escalate if:**
- Conditional rendering based on radio selection unclear
- Validation for min/max relationship confusing
- Managing two different callback functions unclear

**Search terms:**
- "React conditional rendering"
- "React radio button onChange"
- "JavaScript Math.max Math.min validation"

### Acceptance Criteria
- [ ] File created at `frontend/components/InletFlowControl.tsx`
- [ ] Component accepts currentFlow, onFlowChange, onModeChange props
- [ ] Mode selector with "constant" and "brownian" options
- [ ] Conditional rendering based on selected mode
- [ ] Constant mode: single flow input
- [ ] Brownian mode: min, max, variance inputs
- [ ] Apply button triggers appropriate callback
- [ ] Validation ensures max > min in Brownian mode
- [ ] Exports default function

---

## Task 25b: Integrate InletFlowControl into ProcessView

**Phase:** Phase 5 - Process View  
**Prerequisites:** Task 25a  
**Estimated Time:** 20-25 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/ProcessView.tsx`

### Context and References

Add inlet flow control below PID tuning. This requires two separate callback functions for the two WebSocket message types.

**Backend protocol:**
- Message type "inlet_flow" with value field (constant mode)
- Message type "inlet_mode" with mode, min, max, variance fields (mode change)

Reference: lib/types.ts WebSocketMessage union type

### Requirements

**Import:**
- Import InletFlowControl component

**Callback functions:**

Create handleFlowChange:
```
Parameters: value (number)
Action: Send {"type": "inlet_flow", "value": value}
```

Create handleModeChange:
```
Parameters: mode (string), config (object or undefined)
Action: Send {"type": "inlet_mode", "mode": mode, ...config}
```

For constant mode: `{type: "inlet_mode", mode: "constant"}`
For brownian mode: `{type: "inlet_mode", mode: "brownian", min: 0.8, max: 1.2, variance: 0.05}`

**Integration:**
Add below PIDTuningControl:
```
<div className="pt-4 border-t border-gray-700">
  <InletFlowControl
    currentFlow={state.inlet_flow}
    onFlowChange={handleFlowChange}
    onModeChange={handleModeChange}
  />
</div>
```

### Verification

Run dev server:
```bash
cd frontend && npm run dev
```

**Test constant mode:**
1. Select "Constant" mode
2. Set flow to 1.5
3. Click Apply
4. Check console for: `{"type": "inlet_flow", "value": 1.5}`

**Test brownian mode:**
1. Select "Brownian" mode
2. Set min=0.8, max=1.2, variance=0.05
3. Click Apply
4. Check console for: `{"type": "inlet_mode", "mode": "brownian", "min": 0.8, "max": 1.2, "variance": 0.05}`

### Escalation Hints

**Escalate if:**
- Spread operator for config object unclear
- Message format doesn't match types.ts
- Two callbacks confusing

**Search terms:**
- "JavaScript spread operator object"
- "TypeScript union type narrowing"
- "React multiple callback props"

### Acceptance Criteria
- [ ] InletFlowControl imported
- [ ] handleFlowChange function sends "inlet_flow" message
- [ ] handleModeChange function sends "inlet_mode" message
- [ ] Messages match WebSocketMessage type from lib/types.ts
- [ ] Component integrated below PIDTuningControl
- [ ] Border separator added
- [ ] Both constant and brownian modes functional
- [ ] Dev server runs without errors

---

## Task 26: Add Flow Direction Indicators to TankGraphic

**Phase:** Phase 5 - Process View  
**Prerequisites:** Task 25b  
**Estimated Time:** 15-20 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/TankGraphic.tsx`

### Context and References

Enhance the tank graphic with animated flow direction arrows and flow rate labels.

**Visual enhancement:**
- Arrow on inlet pipe (pointing into tank)
- Arrow on outlet pipe (pointing out of tank)
- Flow rate text labels
- Animate arrows (pulse or flow animation)

**Reference:**
- SVG markers for arrows: Search "SVG arrow marker"
- CSS animations: Search "CSS keyframe animation SVG"

### Requirements

Add two props to TankGraphic component:
```
inletFlow: number
outletFlow: number
```

**SVG additions:**

1. **Arrow markers (define in <defs>):**
   - Create arrowhead marker
   - id="arrowhead"
   - Fill with appropriate color

2. **Inlet arrow:**
   - Line or path pointing downward into tank
   - Apply marker-end="#arrowhead"
   - Stroke color based on flow magnitude (blue for flow > 0, gray for 0)

3. **Outlet arrow:**
   - Line or path pointing downward out of tank
   - Apply marker-end="#arrowhead"
   - Stroke color based on flow magnitude

4. **Flow rate labels:**
   - Text element showing inlet_flow value with 2 decimal places
   - Text element showing outlet_flow value with 2 decimal places
   - Position near respective pipes
   - Include units: "m³/s"

**Animation (optional but recommended):**
Add CSS keyframe animation for arrow pulsing:
```
.animate-pulse-slow {
  animation: pulse 2s ease-in-out infinite;
}
```

### Verification

After modification:
```bash
cd frontend && npm run dev
```

**Visual checks:**
- Arrows appear on inlet and outlet pipes
- Flow rate numbers display next to arrows
- Arrows change color based on flow rate
- (If animated) Arrows pulse slowly

### Escalation Hints

**Escalate if:**
- SVG marker syntax unclear after searching docs
- Text positioning confusing
- Animation not working

**Search terms:**
- "SVG marker arrowhead"
- "SVG text element position"
- "Tailwind CSS custom animation"

### Acceptance Criteria
- [ ] Component accepts inletFlow and outletFlow props
- [ ] Arrow markers defined in SVG <defs>
- [ ] Inlet arrow displayed on inlet pipe
- [ ] Outlet arrow displayed on outlet pipe
- [ ] Flow rate labels positioned near arrows
- [ ] Colors change based on flow > 0
- [ ] (Optional) Animation added
- [ ] Props passed from ProcessView in Task 26b

---

## Task 26b: Pass Flow Data to TankGraphic

**Phase:** Phase 5 - Process View  
**Prerequisites:** Task 26  
**Estimated Time:** 10 minutes  
**Files:** 1 file

### File to Modify
- `frontend/components/ProcessView.tsx`

### Requirements

Update the TankGraphic usage to pass inlet and outlet flow rates.

**Change:**
```tsx
<TankGraphic
  level={state.tank_level}
  setpoint={state.setpoint}
  maxHeight={5.0}
  inletFlow={state.inlet_flow}
  outletFlow={state.outlet_flow}
/>
```

### Verification

```bash
cd frontend && npm run dev
```

**Check:**
- Flow arrows appear on tank graphic
- Flow rate numbers update in real-time
- No TypeScript errors

### Acceptance Criteria
- [ ] TankGraphic receives inletFlow and outletFlow props
- [ ] Flow indicators visible on tank graphic
- [ ] Real-time updates working
- [ ] No errors in console

---

## Task 27: Test Complete Process View

**Phase:** Phase 5 - Process View  
**Prerequisites:** All previous tasks  
**Estimated Time:** 20-30 minutes  
**Files:** None (testing task)

### Context and References

Comprehensive test of the complete Process View with all controls and visualization.

### Requirements

**Start full stack:**

1. Start backend (from project root):
```bash
cd api
uv run uvicorn main:app --reload
```

2. Start frontend (separate terminal):
```bash
cd frontend
npm run dev
```

3. Open http://localhost:3000 in browser

### Test Checklist

**Visual verification:**
- [ ] Process tab loads without errors
- [ ] Tank graphic displays with blue liquid
- [ ] Liquid level matches data display
- [ ] Setpoint line visible on tank
- [ ] Flow arrows and labels visible
- [ ] Connection status shows "Connected"

**Setpoint control:**
- [ ] Change setpoint to 3.0m
- [ ] Tank level begins moving toward 3.0m
- [ ] Liquid in graphic animates smoothly
- [ ] Error value updates (setpoint - level)

**PID tuning:**
- [ ] Change Kc to 3.0
- [ ] Change tau_I to 15.0
- [ ] Click Apply
- [ ] System responds with different control behavior
- [ ] Faster response or overshoot depending on gains

**Inlet flow control - Constant:**
- [ ] Select "Constant" mode
- [ ] Set flow to 1.5 m³/s
- [ ] Click Apply
- [ ] Inlet flow label updates to 1.50
- [ ] Tank level responds to change

**Inlet flow control - Brownian:**
- [ ] Select "Brownian" mode
- [ ] Set min=0.8, max=1.2, variance=0.05
- [ ] Click Apply
- [ ] Inlet flow begins random walk
- [ ] Flow stays within min/max bounds
- [ ] Tank level fluctuates

**Real-time updates:**
- [ ] All values update at ~1 Hz
- [ ] Smooth animations
- [ ] No lag or jitter
- [ ] Time counter increments steadily

**Responsive layout:**
- [ ] Resize browser window
- [ ] Layout adjusts (stacks vertically on narrow screen)
- [ ] All controls remain functional
- [ ] Tank graphic scales appropriately

### Troubleshooting

**Backend not connecting:**
- Check backend is running on http://localhost:8000
- Check WebSocket endpoint: ws://localhost:8000/ws
- Check browser console for connection errors

**Controls not responding:**
- Check browser console for WebSocket send errors
- Check backend logs for received messages
- Verify message format matches types.ts

**Tank graphic not animating:**
- Check CSS transitions applied (inspect element)
- Verify level value is changing (check data display)
- Check browser dev tools for SVG errors

### Escalation Hints

**Escalate if:**
- Backend integration not working after checking all connections
- WebSocket messages not being processed
- Visual glitches in tank graphic
- Controls sending incorrect message formats

**Commands to check:**
```bash
# Check backend is running
curl http://localhost:8000/api/config

# Check WebSocket (requires wscat or similar)
# npm install -g wscat
wscat -c ws://localhost:8000/ws
```

### Acceptance Criteria
- [ ] Full stack running (backend + frontend)
- [ ] Process View displays tank graphic and controls
- [ ] Setpoint control functional and responsive
- [ ] PID tuning control functional
- [ ] Inlet flow control functional in both modes
- [ ] Real-time updates working smoothly
- [ ] All visual elements rendering correctly
- [ ] No console errors
- [ ] Responsive layout working

---

## Summary of Phase 5 Micro-Tasks

### Completed Tasks
(None yet - starting fresh)

### Current Tasks (22a - 27)

| Task | Description | Files | Est. Time |
|------|-------------|-------|-----------|
| 22a | Create TankGraphic SVG component | TankGraphic.tsx | 20-30 min |
| 22b | Integrate TankGraphic into ProcessView | ProcessView.tsx | 15-20 min |
| 23a | Create SetpointControl component | SetpointControl.tsx | 20-25 min |
| 23b | Integrate SetpointControl | ProcessView.tsx | 15-20 min |
| 24a | Create PIDTuningControl component | PIDTuningControl.tsx | 25-30 min |
| 24b | Integrate PIDTuningControl | ProcessView.tsx | 15-20 min |
| 25a | Create InletFlowControl component | InletFlowControl.tsx | 20-25 min |
| 25b | Integrate InletFlowControl | ProcessView.tsx | 20-25 min |
| 26 | Add flow indicators to TankGraphic | TankGraphic.tsx | 15-20 min |
| 26b | Pass flow data to TankGraphic | ProcessView.tsx | 10 min |
| 27 | Test complete Process View | (testing) | 20-30 min |

**Total estimated time:** ~3.5-4.5 hours of focused implementation

---

## Upcoming Work (After Phase 5 Complete)

### Phase 6: Trends View
- Create chart components with Recharts
- Level vs Setpoint line chart
- Flow rates line chart
- Valve position line chart
- Time range selector
- Fetch historical data from backend
- Auto-scrolling with new data

### Phase 7: Integration and Polish
- Fetch and display actual PID gains from /api/config
- Error handling and user feedback
- Loading states
- Reconnection UI improvements
- E2E tests with Playwright
- Performance optimization
- Documentation updates

---

## Development Workflow Notes

### Context Preservation

After each task, optionally commit:
```bash
git add <files-changed>
git commit -m "Task Xa: Brief description"
```

This preserves progress and makes it easy to revert if needed.

### Testing Strategy

- **Per-component:** Verify component renders after creation (visual check)
- **Per-integration:** Verify component works when integrated (functional check)
- **End-to-end:** Task 27 validates entire Process View with backend

### Git Workflow

Working on branch: `phase5-process-view`

After all tasks complete and tested:
```bash
git add .
git commit -m "Phase 5 Complete: Process View with tank visualization and controls"
# Code review, then merge to main
```

### Common Patterns

**React component structure:**
```tsx
"use client"; // If uses hooks

import { useState } from "react";
// other imports

interface ComponentProps {
  // prop definitions
}

export default function ComponentName({ prop1, prop2 }: ComponentProps) {
  // state and handlers
  
  return (
    <div className="...">
      {/* JSX */}
    </div>
  );
}
```

**WebSocket message sending:**
```tsx
import { useSimulation } from "../app/providers";

const { send } = useSimulation();

const handleAction = (value: number) => {
  send({ type: "setpoint", value });
};
```

---

## Key Principles for Local LLM Success

### 1. Task Independence
Each task creates or modifies 1-2 files maximum. No task depends on understanding the entire system.

### 2. Reference-First Design
Every task links to relevant documentation. If a pattern is unfamiliar, guidance on where to learn it.

### 3. Escalation Clarity
Each task specifies exactly when to escalate and what to search for first.

### 4. Verification at Scale
Simple, one-command verification per task. No complex multi-step testing until final integration.

### 5. Structure Over Flexibility
Tasks provide detailed structure guidance (what sections, what order) rather than open-ended requirements.

---

## Notes on Architecture Decisions

### Why SVG for Tank Graphic?
- Scalable without pixelation
- Animatable with CSS
- Lightweight (no image assets)
- Accessible (can add ARIA labels)

### Why Separate Control Components?
- Reusable if we add more views later
- Easier to test in isolation
- Clear separation of concerns
- Smaller files = easier for local LLM

### Why Local State in Controls Before Apply?
- Prevents flooding WebSocket with every keystroke
- Gives user chance to review before submitting
- Standard SCADA pattern (set + apply)
- Allows validation before sending

### Why Hardcode Initial PID Gains?
- SimulationState doesn't include current gains
- Config endpoint exists but not yet integrated
- Will be fixed in Phase 7 when we add config fetching
- For now, operator can tune from known starting point

---

**Document created:** 2026-02-13  
**Senior Engineer:** Claude (Sonnet)  
**Target executor:** Local LLM (Qwen 2.5 Coder 32B) or Haiku  
**Branch:** phase5-process-view
