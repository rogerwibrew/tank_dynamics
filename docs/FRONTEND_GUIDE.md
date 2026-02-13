# Frontend Guide - Tank Dynamics Simulator

A comprehensive guide to the Next.js frontend for the Tank Dynamics Simulator. This document covers architecture, component structure, development setup, and usage patterns.

## Table of Contents

1. [Overview](#overview)
2. [Quick Start](#quick-start)
3. [Architecture](#architecture)
4. [Components](#components)
5. [Hooks and Utilities](#hooks-and-utilities)
6. [Development](#development)
7. [Deployment](#deployment)
8. [Troubleshooting](#troubleshooting)

---

## Overview

The frontend is a modern Next.js 16 application with the App Router, providing a SCADA-style interface for real-time process monitoring and control. It communicates with the FastAPI backend via WebSocket for live updates (1 Hz) and REST endpoints for historical data.

### Key Features

- **Real-time Monitoring**: Live tank level, flow rates, and valve position via WebSocket
- **Process Control**: Adjust PID gains, setpoints, and inlet flow in real-time
- **Tabbed Interface**: Process View for control and Trends View for historical analysis
- **Dark Theme**: SCADA-style dark interface with Tailwind CSS
- **Responsive Design**: Works on desktop and tablet screens
- **Connection Status**: Visual indicator of backend connectivity
- **Type Safety**: Full TypeScript support with strict mode

### Technology Stack

- **Framework**: Next.js 16 (App Router)
- **Language**: TypeScript 5
- **Styling**: Tailwind CSS v4 with dark theme configuration
- **Charting**: Recharts 3.7 for time-series visualization
- **Build Tool**: Webpack (built into Next.js)

---

## Quick Start

### Prerequisites

- Node.js 18+ (check with `node --version`)
- Backend running on `localhost:8000` (or configure `NEXT_PUBLIC_WS_URL`)
- npm or your preferred package manager

### Installation

```bash
# Navigate to frontend directory
cd frontend

# Install dependencies
npm install

# Start development server
npm run dev
```

The application will be available at `http://localhost:3000`

### Configuration

Set the backend URL via environment variable:

```bash
# .env.local or shell
export NEXT_PUBLIC_WS_URL=ws://your-backend:8000/ws
npm run dev
```

If not set, defaults to `ws://localhost:8000/ws`

### Build for Production

```bash
# Build optimized production bundle
npm run build

# Start production server
npm start
```

---

## Architecture

### Component Hierarchy

```
App (Root Layout)
├── Header (Title + Connection Status)
├── TabNavigation
│   ├── "Process" Tab
│   └── "Trends" Tab
└── Main Content (activeTab dependent)
    ├── ProcessView (Control Interface)
    │   ├── Tank Visualization
    │   ├── PID Control Panel
    │   ├── Flow Controls
    │   └── Status Indicators
    └── TrendsView (Historical Data)
        ├── Level vs Setpoint Chart
        ├── Flow Rate Chart
        └── Valve Position Chart
```

### Data Flow

```
┌─────────────────────────────────────────────────────┐
│            SimulationProvider Context               │
│  • Manages WebSocket connection                     │
│  • Broadcasts state updates to all components       │
│  • Provides command methods (setSetpoint, etc)      │
└─────────────────────────────────┬───────────────────┘
                                  │
                    ┌─────────────┴──────────────┐
                    │                            │
              WebSocket (1 Hz)            React Components
            (Real-time updates)           (useContext hook)
                    │                            │
                    └─────────────┬──────────────┘
                                  │
                    ┌─────────────┴──────────────┐
                    │                            │
              ProcessView                  TrendsView
              (Controls)                  (Charts)
```

### State Management

State is managed at the application level through React Context (`SimulationProvider`):

- **Global State**: Current simulation state (tank level, flows, etc)
- **Local State**: UI state (active tab, form inputs)
- **Connection Status**: Managed by `useWebSocket` hook

This approach avoids prop drilling while keeping complexity low for a single-view application.

---

## Components

### Root Layout (`app/layout.tsx`)

The root layout component wraps the entire application with the `SimulationProvider` and establishes the overall page structure.

**Responsibilities:**
- Global styling (Tailwind CSS dark theme)
- Provider setup for Context
- Font and metadata configuration
- Document structure (html, body tags)

**Key Exports:**
- None (used by Next.js automatically)

### Home Page (`app/page.tsx`)

The main application page with tabs for Process and Trends views.

**Features:**
- Header with title and connection status indicator
- Tab navigation for switching between views
- Content area that switches based on active tab
- Client-side rendered ("use client")

**State:**
```typescript
const [activeTab, setActiveTab] = useState("process");
```

### ConnectionStatus Component (`components/ConnectionStatus.tsx`)

Status indicator showing backend connectivity.

**Props:** None (uses `useWebSocket` context internally)

**States:**
- `connected`: Green indicator, "Connected"
- `disconnected`: Gray indicator, "Disconnected"
- `error`: Red indicator, "Connection Error"

**Visual Feedback:**
- Color-coded status with animated dot (when connecting)
- Tooltip shows last update time

### TabNavigation Component (`components/TabNavigation.tsx`)

Tab selector for switching between Process and Trends views.

**Props:**
```typescript
{
  activeTab: "process" | "trends";
  onTabChange: (tab: "process" | "trends") => void;
}
```

**Features:**
- Two tabs with visual active state
- Smooth transitions
- Keyboard accessible

### ProcessView Component (`components/ProcessView.tsx`)

Real-time control interface showing tank visualization and control inputs.

**Props:** None (uses context for state and callbacks)

**Sections:**
1. **Tank Visualization**
   - SVG graphic showing current tank level
   - Color changes based on level (green safe, yellow warning, red critical)
   - Displays current level percentage

2. **Control Panel**
   - **Setpoint Control**: Slider and numeric input for target level
   - **PID Gains**: Three inputs for Kc, tau_I, tau_D
   - **Manual Inlet Flow**: Input field for manual inlet flow rate
   - **Inlet Mode**: Selector for manual/Brownian with variance control

3. **Status Indicators**
   - Current level (m)
   - Inlet flow (m³/s)
   - Outlet flow (m³/s)
   - Valve position (%)
   - Error (current - setpoint)

**Controls:**
- Sliders update state immediately on input
- Numeric fields validate and debounce updates
- Mode selector enables/disables related controls

### TrendsView Component (`components/TrendsView.tsx`)

Historical data visualization with time-series charts.

**Props:** None (uses context for state)

**Sub-Components:**
1. **LevelChart** (`components/LevelChart.tsx`)
   - LineChart showing actual tank level vs setpoint
   - Blue line: actual level, Red dashed line: setpoint
   - Dual Y-axes for flexible scaling
   - Interactive tooltips with values at hover point

2. **FlowsChart** (`components/FlowsChart.tsx`)
   - LineChart comparing inlet vs outlet flow rates
   - Blue line: inlet flow, Green line: outlet flow
   - Single Y-axis for direct comparison
   - Helps identify flow imbalances

3. **ValveChart** (`components/ValveChart.tsx`)
   - AreaChart showing valve position history
   - Filled area gradient (gray closed → blue open)
   - Y-axis: 0-100% valve opening
   - Visual representation of valve control

**Features:**
- Time-domain X-axis (HH:MM:SS format)
- Responsive grid layout (1-2 columns depending on screen)
- Real-time data streaming (updates every 1 second)
- Time range selector (30m, 1h, custom range)
- Interactive legend (click to toggle series visibility)
- Custom tooltips showing precise values
- Cross-filtering between related metrics
- Smooth animations on data updates
- Error handling for API failures

---

## Hooks and Utilities

### useHistory Hook (`hooks/useHistory.ts`)

Custom React hook that fetches historical simulation data from the backend.

**Parameters:**
```typescript
{
  duration?: number;  // Duration in seconds (default: 3600 = 1 hour)
}
```

**Returns:**
```typescript
{
  data: HistoryDataPoint[];  // Array of timestamped state snapshots
  loading: boolean;          // True while fetching data
  error: string | null;      // Error message if fetch failed
  refetch: () => void;       // Manually trigger data refetch
}
```

**Data Format:**
Each point contains:
```typescript
{
  timestamp: number;        // Unix timestamp (seconds)
  tank_level: number;       // Tank level in meters
  setpoint: number;         // Target level in meters
  inlet_flow: number;       // Inlet flow rate (m³/s)
  outlet_flow: number;      // Outlet flow rate (m³/s)
  valve_position: number;   // Valve opening (0-1 scale)
  error: number;            // Control error (setpoint - level)
}
```

**Features:**
- Automatic data fetching on mount
- Manual refetch capability for updating data
- Loading state for UI feedback
- Error handling with user-friendly messages
- Duration parameter for flexible time ranges

**Example Usage:**
```typescript
const { data, loading, error, refetch } = useHistory({ duration: 1800 }); // 30 minutes

if (loading) return <div>Loading history...</div>;
if (error) return <div>Error: {error}</div>;

return (
  <div>
    <button onClick={refetch}>Refresh Data</button>
    {/* Use data in charts */}
  </div>
);
```

**Implementation Notes:**
- Fetches from `GET /api/history?duration={seconds}`
- Caches data in component state (no global cache)
- Updates when duration prop changes
- Suitable for time range selector in TrendsView

---

### useWebSocket Hook (`hooks/useWebSocket.ts`)

Custom React hook that manages WebSocket connection and exposes simulation control methods.

**Returns:**
```typescript
{
  state: SimulationState | null;           // Current simulation state
  connectionStatus: ConnectionStatus;      // "connected" | "disconnected" | "error"
  error: string | null;                    // Error message if any
  setSetpoint: (value: number) => void;    // Command: set target level
  setPIDGains: (Kc, tau_I, tau_D) => void; // Command: update PID parameters
  setInletFlow: (value: number) => void;   // Command: set manual inlet flow
  setInletMode: (mode, min, max, var) => void; // Command: set inlet mode
  reconnect: () => void;                   // Attempt to reconnect
}
```

**Lifecycle:**
- On mount: Creates WebSocket client and listeners
- On unmount: Cleans up listeners and closes connection
- Automatic reconnection with exponential backoff (5s, 10s, 20s, 30s)

**Example Usage:**
```typescript
const { state, setSetpoint, connectionStatus } = useWebSocket();

// Update setpoint in real-time
const handleSetpointChange = (value: number) => {
  setSetpoint(value);
};
```

### WebSocket Client (`lib/websocket.ts`)

Low-level WebSocket client with event-based interface and reconnection logic.

**Class: WebSocketClient**

**Constructor:**
```typescript
new WebSocketClient(url: string)
```

**Methods:**
- `connect()`: Establish connection to server
- `disconnect()`: Close connection
- `send(message: object)`: Send JSON message to server
- `on(event, callback)`: Subscribe to event (returns unsubscribe function)

**Events:**
- `connect`: Connection established
- `disconnect`: Connection closed
- `error`: Connection error occurred
- `message`: Message received from server

**Reconnection:**
- Automatic reconnection with exponential backoff
- Max reconnection interval: 30 seconds
- Triggers `reconnect` event before attempting

**Example:**
```typescript
const client = new WebSocketClient("ws://localhost:8000/ws");
client.on("message", (data) => console.log("Received:", data));
client.connect();
```

### Type Definitions (`lib/types.ts`)

TypeScript interfaces matching the backend API models.

**Key Types:**

```typescript
// Main simulation state
interface SimulationState {
  timestamp: number;        // Unix timestamp
  tank_level: number;       // Current tank level (m)
  setpoint: number;         // Target tank level (m)
  inlet_flow: number;       // Inlet volumetric flow (m³/s)
  outlet_flow: number;      // Outlet volumetric flow (m³/s)
  valve_position: number;   // Valve opening (0-1)
  error: number;            // Control error (setpoint - level)
}

// PID controller state
interface PIDState {
  Kc: number;      // Proportional gain
  tau_I: number;   // Integral time (s)
  tau_D: number;   // Derivative time (s)
}
```

### Utility Functions (`lib/utils.ts`)

Helper functions for common operations.

**Available Functions:**
- `formatNumber(value, decimals)`: Format numbers with fixed decimal places
- `calculatePercentage(value, min, max)`: Calculate percentage between bounds
- `clampValue(value, min, max)`: Restrict value to range
- `getStatusColor(level, warning, critical)`: Get color for status display

---

## Development

### File Structure

```
frontend/
├── app/
│   ├── layout.tsx              # Root layout with providers
│   ├── page.tsx                # Home page with tabs
│   └── providers.tsx           # SimulationProvider context
├── components/
│   ├── ConnectionStatus.tsx    # Connection indicator
│   ├── ProcessView.tsx         # Control interface
│   ├── TabNavigation.tsx       # Tab selector
│   ├── TrendsView.tsx          # Historical charts container
│   ├── LevelChart.tsx          # Level vs setpoint chart
│   ├── FlowsChart.tsx          # Inlet/outlet flow chart
│   ├── ValveChart.tsx          # Valve position chart
│   ├── TankGraphic.tsx         # SVG tank visualization
│   ├── PIDControlPanel.tsx     # PID tuning interface
│   └── InletFlowControl.tsx    # Inlet manipulation controls
├── hooks/
│   ├── useWebSocket.ts         # WebSocket management hook
│   └── useHistory.ts           # Historical data fetching hook
├── lib/
│   ├── types.ts                # TypeScript type definitions
│   ├── utils.ts                # Utility functions
│   └── websocket.ts            # WebSocket client class
├── public/                     # Static assets
├── package.json                # Dependencies and scripts
├── tsconfig.json               # TypeScript configuration
├── next.config.ts              # Next.js configuration
└── tailwind.config.ts          # Tailwind CSS configuration
```

### Development Server

```bash
# Start with hot reload
npm run dev

# Run on different port
npm run dev -- -p 3001

# With verbose logging
npm run dev -- --verbose
```

The dev server watches for file changes and automatically reloads. Page errors appear in browser console.

### TypeScript

Full strict mode enabled for type safety:

```bash
# Type checking without build
npx tsc --noEmit

# Errors prevent build in CI
npm run build  # Fails if type errors exist
```

### Linting

ESLint checks code style:

```bash
# Check code
npx eslint .

# Fix common issues automatically
npx eslint . --fix
```

### Adding New Dependencies

Use npm to add packages:

```bash
# Add production dependency
npm install package-name

# Add dev dependency
npm install --save-dev package-name

# Update all dependencies
npm update
```

### Common Development Tasks

**Add a new component:**

1. Create file: `components/MyComponent.tsx`
2. Export as React component
3. Import and use in other components

**Add a new page:**

1. Create file: `app/mypage/page.tsx`
2. Export default component
3. Next.js automatically creates route `/mypage`

**Add a new utility function:**

1. Add to `lib/utils.ts` or create new file `lib/my-utils.ts`
2. Export function
3. Import where needed

---

## Deployment

### Production Build

```bash
# Create optimized build
npm run build

# Verify build size
du -sh .next
```

Build output is in `.next/` directory.

### Environment Variables

Set `NEXT_PUBLIC_WS_URL` for production:

```bash
# .env.production.local
NEXT_PUBLIC_WS_URL=wss://simulator.example.com/ws
```

Or pass at build time:

```bash
NEXT_PUBLIC_WS_URL=wss://api.example.com/ws npm run build
```

### Running Production Server

```bash
# Build and start
npm run build
npm start

# Or use process manager (pm2 example)
pm2 start "npm start" --name "tank-simulator-frontend"
```

### Docker Deployment

Example Dockerfile:

```dockerfile
FROM node:18-alpine
WORKDIR /app
COPY package*.json ./
RUN npm ci --only=production
COPY .next .next
COPY public public
EXPOSE 3000
CMD ["npm", "start"]
```

Build and run:

```bash
docker build -t tank-simulator-frontend .
docker run -p 3000:3000 \
  -e NEXT_PUBLIC_WS_URL=wss://api.example.com/ws \
  tank-simulator-frontend
```

### Nginx Reverse Proxy

```nginx
upstream nextjs_backend {
    server localhost:3000;
}

server {
    listen 80;
    server_name simulator.example.com;

    location / {
        proxy_pass http://nextjs_backend;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }
}
```

### SSL/TLS Setup

For secure WebSocket (wss://), enable HTTPS:

```nginx
server {
    listen 443 ssl;
    server_name simulator.example.com;

    ssl_certificate /etc/letsencrypt/live/simulator.example.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/simulator.example.com/privkey.pem;

    # ... rest of proxy config
}
```

Then update frontend:

```bash
NEXT_PUBLIC_WS_URL=wss://simulator.example.com/ws npm run build
```

---

## Troubleshooting

### WebSocket Connection Fails

**Problem:** "WebSocket connection error" message displayed

**Solutions:**

1. **Check backend is running:**
   ```bash
   # On backend machine
   curl http://localhost:8000/api/health
   ```

2. **Verify WebSocket URL:**
   ```bash
   # Check environment variable
   echo $NEXT_PUBLIC_WS_URL
   ```

3. **Check CORS/firewall:**
   - Browser console shows CORS error → backend doesn't allow origin
   - Network tab shows connection refused → check firewall/port

4. **Reset WebSocket:**
   - Reload page (Ctrl+R or Cmd+R)
   - Check browser console for detailed error messages

**Example error messages and fixes:**

| Error | Cause | Fix |
|-------|-------|-----|
| `Failed to construct 'WebSocket'` | Invalid URL format | Check `NEXT_PUBLIC_WS_URL` syntax |
| `WebSocket is closed` | Connection dropped | Check backend is running |
| `net::ERR_CONNECTION_REFUSED` | Can't reach backend | Check IP/port, firewall rules |
| `403 Forbidden` | CORS policy blocking | Add frontend origin to backend CORS config |

### Components Not Updating

**Problem:** Values in ProcessView or TrendsView don't change

**Solutions:**

1. **Check connection status:** 
   - Should show "Connected" in top-right
   - If "Disconnected" or "Error", see WebSocket troubleshooting above

2. **Check network tab:**
   - Open DevTools → Network → WS filter
   - Should see WebSocket connection active
   - Messages should arrive ~1 per second

3. **Check backend logs:**
   - Backend should log WebSocket connections
   - Should see state messages being sent

### Build Errors

**Problem:** `npm run build` fails with TypeScript errors

**Solutions:**

1. **Run type check:**
   ```bash
   npx tsc --noEmit
   ```

2. **Fix type errors:**
   - Error message shows file and line number
   - Usually missing type annotations or wrong types

3. **Check Node.js version:**
   ```bash
   node --version  # Should be 18+
   ```

### Performance Issues

**Problem:** UI feels sluggish or laggy

**Solutions:**

1. **Check WebSocket message rate:**
   - Open DevTools → Network → WS tab
   - Should see messages arriving at ~1 Hz (not faster)

2. **Check chart data points:**
   - Too many data points → chart becomes slow
   - Limit history to ~120 entries (2 minutes at 1 Hz)

3. **Profile React rendering:**
   - DevTools → Profiler tab
   - Record and look for slow component renders
   - Check if components re-render unnecessarily

### Styling Issues

**Problem:** Dark theme not applied or colors look wrong

**Solutions:**

1. **Clear Next.js cache:**
   ```bash
   rm -rf .next
   npm run dev
   ```

2. **Check Tailwind CSS:**
   - Verify `tailwind.config.ts` exists
   - Check theme values in config match expectation

3. **Clear browser cache:**
   - Hard refresh: Ctrl+Shift+R (Windows) or Cmd+Shift+R (Mac)
   - Or clear cache in DevTools → Application → Clear storage

---

## Integration with Backend

### Expected WebSocket Message Format

The frontend expects state messages from the backend in this format:

```json
{
  "type": "state",
  "data": {
    "timestamp": 1707700000,
    "tank_level": 2.5,
    "setpoint": 3.0,
    "inlet_flow": 1.0,
    "outlet_flow": 0.9,
    "valve_position": 0.85,
    "error": 0.5
  }
}
```

### Sending Commands to Backend

The frontend sends control commands as JSON:

```json
{"type": "setpoint", "value": 3.5}
{"type": "pid", "Kc": 1.5, "tau_I": 8.0, "tau_D": 2.0}
{"type": "inlet_flow", "value": 1.2}
{"type": "inlet_mode", "mode": "brownian", "min": 0.8, "max": 1.2, "variance": 0.1}
```

See backend `api/README.md` for complete protocol documentation.

---

## Performance Optimization

### Build Size

Check production bundle size:

```bash
npm run build
# Check output for "page size" information
```

Current bundle is optimized with:
- Code splitting (page-specific bundles)
- CSS purging (unused styles removed)
- React optimizations (strict mode in dev)

### Runtime Performance

For improved performance:

1. **Memoize expensive components:**
   ```typescript
   import { memo } from 'react';
   export const MyChart = memo(function MyChart(props) { ... });
   ```

2. **Limit state updates:**
   - Use `useCallback` for stable function references
   - Avoid unnecessary re-renders with proper dependency arrays

3. **Optimize charts:**
   - Limit data points displayed
   - Use responsive container sizing
   - Consider debouncing updates

---

## Future Enhancements

Potential improvements for future phases:

1. **Advanced Analytics**
   - Export data to CSV
   - Scenario comparison
   - PID tuning recommendations

2. **Multiple Views**
   - Real-time alerts/alarms
   - Process simulation log viewer
   - Configuration management

3. **Performance**
   - Server-side rendering (SSR) for initial load
   - Service Worker for offline mode
   - Data compression for large history

4. **User Experience**
   - Touch-optimized controls for tablets
   - Keyboard shortcuts for power users
   - Custom chart time ranges

---

## Resources

### Next.js Documentation
- [Next.js App Router](https://nextjs.org/docs/app)
- [Next.js API Routes](https://nextjs.org/docs/app/building-your-application/routing)
- [Next.js Deployment](https://nextjs.org/docs/deployment)

### React Documentation
- [React Hooks](https://react.dev/reference/react)
- [Context API](https://react.dev/reference/react/useContext)
- [Performance](https://react.dev/reference/react/useMemo)

### Tailwind CSS
- [Tailwind Docs](https://tailwindcss.com/docs)
- [Dark Mode](https://tailwindcss.com/docs/dark-mode)
- [Responsive Design](https://tailwindcss.com/docs/responsive-design)

### Recharts
- [Recharts Documentation](https://recharts.org)
- [API Reference](https://recharts.org/api)
- [Examples](https://recharts.org/examples)

---

**Last Updated:** 2026-02-13
**Phase:** 6 - Trends View Enhancement (Complete)
**Status:** Full SCADA interface with real-time controls and historical trend analysis
