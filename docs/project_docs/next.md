# Next Tasks - Tank Dynamics Simulator

## Current Phase: Phase 4 - Next.js Frontend (Micro-Task Breakdown)

**Phase Status:** Task 19a complete, continuing Phase 4 with corrected task breakdown
**Branch:** phase4-initial

**IMPORTANT UPDATE (2026-02-10):** Task 19a discovered that Next.js 16 and Tailwind v4 have different output than documented. This file has been updated to reflect actual behavior.

**What Changed:**
- Next.js 16 uses `next.config.ts` (TypeScript) instead of `next.config.js`
- ESLint uses flat config format (`eslint.config.mjs`) instead of `.eslintrc.json`
- Tailwind CSS v4 is NOT installed by default (requires separate installation)
- Tailwind v4 uses CSS-based configuration (@import, @theme) instead of `tailwind.config.ts`
- Task 19ba added for Tailwind installation
- Task 19g merged into 19d (single CSS configuration file)

**Context:** Phases 1-3 are complete with all tests passing. The C++ simulation core, Python bindings, and FastAPI backend are fully operational with comprehensive test coverage. Phase 3 delivered a production-ready API with WebSocket real-time updates at 1 Hz, REST endpoints, Brownian inlet flow mode, and complete documentation.

**Phase 4 Goals:** Build a modern SCADA-style web interface using Next.js 16 with App Router, Tailwind CSS v4, and Recharts. The interface will provide real-time process visualization, control inputs, and historical trend plotting.

**Micro-Task Strategy:** This phase is broken into 1-2 file tasks taking 15-30 minutes each. Each task is independently testable and suitable for local LLMs or Haiku. See LESSONS_LEARNED.md for the rationale behind this approach.

---

## Task 19a: Initialize Next.js Project with App Router

**Phase:** 4 - Next.js Frontend
**Prerequisites:** None - Phase 3 backend operational
**Estimated Time:** 15-30 minutes
**Files:** Command execution only

### Context and References

If unfamiliar with Next.js initialization:
- Reference: https://nextjs.org/docs/getting-started/installation
- Search keywords: "Next.js create-next-app setup" or "Next.js 14 quickstart"
- Escalation: Not needed for this task (straightforward command)

### Requirements

Initialize a new Next.js project in the `frontend/` directory at the root of the repository using the official create-next-app command.

The initialization should:
- Create a directory called `frontend` at `/home/roger/dev/tank_dynamics/frontend/`
- Use Next.js 14 or later with the App Router (not Pages Router)
- Enable TypeScript support with strict mode
- Include ESLint configuration for code quality
- Enable Tailwind CSS for styling
- Keep the app directory at root level (not in src/)
- Skip the example app directory (use manual structure)

### Exact Command to Execute

From the tank_dynamics directory, run:

```bash
npx create-next-app@latest frontend --typescript --tailwind --app --no-src-dir --eslint --skip-install
```

This creates the project structure without installing dependencies (we'll handle that in the next task).

### Verification

Run this command to verify the project structure:

```bash
ls -la /home/roger/dev/tank_dynamics/frontend/
```

Expected output should show:
- `app/` directory (for App Router)
- `public/` directory (for static assets)
- `package.json` (project manifest)
- `tsconfig.json` (TypeScript configuration)
- `next.config.ts` (Next.js configuration in TypeScript)
- `next-env.d.ts` (Next.js TypeScript declarations)
- `postcss.config.mjs` (PostCSS configuration)
- `eslint.config.mjs` (ESLint flat config format)
- `.gitignore` (git ignore patterns)
- `README.md` (Next.js readme)

Note: As of Next.js 15+, create-next-app does NOT install Tailwind CSS by default anymore, even with the --tailwind flag. Task 19ba below will handle Tailwind installation separately.

### Escalation Hints

**Escalate to Haiku if:**
- The create-next-app command fails due to Node version incompatibility
- The project structure looks significantly different than expected

**Search for these terms if stuck:**
- "Next.js Node version requirements"
- "npm create-next-app troubleshooting"

### Acceptance Criteria
- [ ] Next.js project initialized in `frontend/` directory
- [ ] App Router is configured (not Pages Router)
- [ ] TypeScript support is enabled
- [ ] Tailwind CSS is included
- [ ] Project structure contains app/, public/, and config files

---

## Task 19ba: Install Tailwind CSS

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 19a (Next.js project initialized)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/tailwind.config.ts`, `frontend/app/globals.css` (created by init command)

### Context and References

Tailwind CSS v4+ has a new installation process. As of Next.js 16+, Tailwind is NOT installed by default.
- Reference: https://tailwindcss.com/docs/installation/framework-guides
- Search keywords: "Tailwind CSS Next.js installation" or "Tailwind v4 setup"
- Escalation: If installation fails with unclear errors, escalate to Haiku

### Requirements

Install Tailwind CSS in the Next.js project. The latest create-next-app no longer includes Tailwind by default, so we need to add it manually.

Note: The package.json already shows `@tailwindcss/postcss` and `tailwindcss` version 4 in devDependencies from the create-next-app initialization, but we need to verify the configuration files are properly set up.

### Exact Commands to Execute

From the frontend directory:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npm install
```

This installs all dependencies including Tailwind CSS v4.

### Verification

Check that Tailwind is installed:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npm list tailwindcss
```

Expected output: Should show tailwindcss version 4.x in the dependency tree.

Check that node_modules exists:

```bash
ls -la /home/roger/dev/tank_dynamics/frontend/ | grep node_modules
```

Should show the node_modules directory.

Check PostCSS config mentions Tailwind:

```bash
cat /home/roger/dev/tank_dynamics/frontend/postcss.config.mjs
```

Should show `@tailwindcss/postcss` plugin.

### Escalation Hints

**Escalate to Haiku if:**
- npm install fails with dependency conflicts
- Tailwind CSS not found after installation
- Node version compatibility issues

**Search for these terms if stuck:**
- "Tailwind CSS v4 installation Next.js 16"
- "npm install troubleshooting"

### Acceptance Criteria
- [ ] All npm dependencies installed successfully
- [ ] node_modules directory created
- [ ] tailwindcss package installed (version 4.x)
- [ ] @tailwindcss/postcss installed
- [ ] No error messages during installation
- [ ] PostCSS config references Tailwind plugin

---

## Task 19b: Install Additional Frontend Dependencies

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 19ba (Tailwind CSS installed)
**Estimated Time:** 15-30 minutes
**Files:** frontend/package.json (modified to add recharts)

### Context and References

For the frontend, we use npm (not uv). The backend uses uv for Python packages.
- Reference: https://docs.npmjs.com/cli/v10/commands/npm-install
- Search keywords: "npm install package" or "npm add dependency"
- Escalation: Not needed (straightforward command)

### Requirements

Install additional UI dependencies for the Next.js frontend project. The base dependencies (Next.js, React, TypeScript, Tailwind) are already in package.json from create-next-app and Task 19ba.

**Additional UI Dependencies to Install:**
- recharts (^3.7.0) for time-series charting and trend visualization

**Important:** Recharts v3.0 introduced breaking changes from v2.x:
- CartesianGrid now requires `xAxisId`/`yAxisId` props (defaults may not render)
- Internal state management completely rewritten
- New React hooks available for accessing chart state
- Refer to [Recharts 3.0 migration guide](https://github.com/recharts/recharts/wiki/3.0-migration-guide) if needed

### Exact Commands to Execute

Navigate to the frontend directory and add Recharts:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npm install recharts
```

This adds Recharts to package.json dependencies and installs it to node_modules.

### Verification

Verify Recharts installation:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npm list recharts
```

Expected output should show recharts version 3.7.0 or higher (3.x series).

Also verify it's in package.json:

```bash
grep "recharts" /home/roger/dev/tank_dynamics/frontend/package.json
```

Should show recharts in dependencies section.

### Escalation Hints

**Escalate to Haiku if:**
- npm install fails with dependency conflicts
- Version conflicts preventing installation
- Network errors downloading packages

**Search for these terms if stuck:**
- "npm install recharts troubleshooting"
- "npm dependency resolution"

### Acceptance Criteria
- [ ] recharts package installed successfully
- [ ] recharts appears in package.json dependencies
- [ ] recharts appears in node_modules directory
- [ ] No error messages during installation

---

## Task 19c: Configure TypeScript with Strict Mode

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 19a (Next.js project initialized)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/tsconfig.json` (modify)

### Context and References

TypeScript is a programming language that adds type safety to JavaScript. If unfamiliar:
- Reference: https://www.typescriptlang.org/docs/handbook/
- Search keywords: "TypeScript strict mode setup" or "tsconfig.json guide"
- Escalation: Not needed for this task (straightforward configuration)

### Requirements

Modify the TypeScript configuration file to enable strict type checking and configure path aliases for cleaner imports.

The tsconfig.json file should have:

**Compiler Options Section:**
- `strict: true` - Enable strict type checking mode
- `module: "esnext"` - Use ES modules
- `lib: ["ES2022", "DOM", "DOM.Iterable"]` - Include ES2022 and DOM APIs
- `moduleResolution: "bundler"` - Use bundler resolution (recommended for Next.js)
- `target: "ES2022"` - Compile to ES2022 JavaScript
- `jsx: "preserve"` - Let Next.js handle JSX (important for App Router)
- `jsxImportSource: "react"` - Import React for JSX

**Path Aliases Section (under compilerOptions.paths):**
Configure these aliases for convenience:
- `@/*` maps to `./` (import from @/components, @/lib, @/hooks, etc.)

**Include and Exclude Sections:**
- include: `["next-env.d.ts", "**/*.ts", "**/*.tsx"]`
- exclude: `["node_modules"]`

The result should allow imports like `import { SimulationState } from '@/lib/types'` instead of relative paths.

### Verification

Verify the TypeScript configuration by running the TypeScript compiler check:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit
```

Expected result: No errors (output should be empty or only show compilation success).

Check that strict mode is enabled:

```bash
grep '"strict"' /home/roger/dev/tank_dynamics/frontend/tsconfig.json
```

Should output: `"strict": true`

### Escalation Hints

**Escalate to Haiku if:**
- TypeScript compilation fails with unclear errors
- Path aliases don't resolve in your editor

**Search for these terms if stuck:**
- "TypeScript path aliases configuration"
- "Next.js tsconfig.json strict mode"

### Acceptance Criteria
- [ ] `strict: true` enabled in compiler options
- [ ] Path aliases configured (@/* maps to ./)
- [ ] moduleResolution set to "bundler"
- [ ] target set to "ES2022"
- [ ] TypeScript compilation passes without errors
- [ ] Path alias imports would work (verified by configuration)

---

## Task 19d: Configure Tailwind CSS for SCADA Dark Theme

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 19ba (Tailwind CSS installed)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/app/globals.css` (modify to add Tailwind directives and custom theme)

### Context and References

Tailwind CSS v4+ uses a new CSS-first configuration approach. Instead of tailwind.config.ts, we configure Tailwind directly in CSS using @theme directive.
- Reference: https://tailwindcss.com/docs/v4-beta
- Search keywords: "Tailwind CSS v4 configuration" or "Tailwind v4 @theme directive"
- Escalation: If v4 configuration seems unclear, escalate to Haiku

### Requirements

Configure Tailwind CSS with custom colors for the SCADA dark theme. With Tailwind v4, configuration is done in the CSS file using the @theme directive, not in a separate config file.

**Tailwind v4 Configuration in globals.css:**

The globals.css file should be structured with these sections:

**Section 1: Tailwind Import**
Import Tailwind CSS at the top:
```
@import "tailwindcss";
```

**Section 2: Custom Theme Variables (using @theme)**
Define custom colors using the @theme directive:

Process State Colors:
- `--color-process-normal`: green (#10b981) for normal operation
- `--color-process-warning`: amber (#f59e0b) for warning state
- `--color-process-alarm`: red (#ef4444) for alarm state

Tank Visualization Colors:
- `--color-tank-liquid`: blue (#3b82f6) for liquid fill
- `--color-tank-empty`: dark gray (#374151) for empty space
- `--color-tank-border`: medium gray (#6b7280) for container borders

SCADA UI Colors:
- `--color-scada-dark`: very dark gray (#111827) for main background
- `--color-scada-card`: dark gray (#1f2937) for cards/panels
- `--color-scada-text`: white (#ffffff) for primary text

These can be used in Tailwind classes as: `bg-[--color-process-normal]` or similar.

**Section 3: Base Styles**
Define base HTML element styling:
- Dark background on body
- Light text color
- System font stack
- Smooth scrolling behavior

**Note on Tailwind v4:**
Tailwind v4 doesn't use traditional config files. Configuration is done directly in CSS using @theme and @layer directives. The @import "tailwindcss" brings in all of Tailwind's utilities.

### Verification

Verify the globals.css file has Tailwind import:

```bash
head -5 /home/roger/dev/tank_dynamics/frontend/app/globals.css
```

Should show `@import "tailwindcss";` at or near the top.

Check for custom color variables:

```bash
grep "color-process\|color-tank\|color-scada" /home/roger/dev/tank_dynamics/frontend/app/globals.css
```

Should show the custom color variable definitions.

Test that styles compile without errors:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npm run dev
```

Should start dev server without CSS compilation errors.

### Escalation Hints

**Escalate to Haiku if:**
- Tailwind v4 @theme syntax seems unclear
- CSS custom properties pattern is confusing
- Want recommendations on color psychology for industrial UI

**Search for these terms if stuck:**
- "Tailwind CSS v4 custom colors"
- "Tailwind CSS v4 @theme directive"
- "CSS custom properties with Tailwind"

### Acceptance Criteria
- [ ] globals.css imports Tailwind with @import "tailwindcss"
- [ ] Custom color variables defined (process states, tank colors, SCADA UI)
- [ ] Base styles defined for body and HTML elements
- [ ] File is syntactically valid CSS
- [ ] Dev server starts without CSS compilation errors

---

## Task 19e: Create Type Definitions Matching API Models

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 19a (TypeScript configured)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/lib/types.ts` (create new)

### Context and References

TypeScript interfaces define the shape of objects. If unfamiliar:
- Reference: https://www.typescriptlang.org/docs/handbook/interfaces.html
- Search keywords: "TypeScript interface tutorial" or "TypeScript types guide"
- Escalation: If unsure about field mappings, check `/home/roger/dev/tank_dynamics/docs/project_docs/FASTAPI_API_REFERENCE.md`

### Requirements

Create a new file `frontend/lib/types.ts` containing TypeScript interfaces that match the Pydantic models from the FastAPI backend. This ensures type safety when receiving WebSocket messages and API responses.

**Interface 1: SimulationState**
This represents the real-time simulation snapshot sent via WebSocket every second.

Fields should include:
- `time`: number (simulation time in seconds)
- `level`: number (current tank level in meters)
- `setpoint`: number (target level in meters)
- `error`: number (difference between setpoint and level)
- `inlet_flow`: number (current inlet flow rate in m³/s)
- `outlet_flow`: number (current outlet flow rate in m³/s)
- `valve_position`: number (valve opening fraction, 0 to 1)
- `inlet_mode`: string (either "constant" or "brownian")
- `inlet_config`: optional object with min, max, variance fields

**Interface 2: ConfigResponse**
This represents the simulation configuration returned by GET /api/config.

Fields should include:
- `tank_height`: number (maximum tank height in meters)
- `tank_area`: number (cross-sectional area in m²)
- `valve_coefficient`: number (k_v value)
- `initial_level`: number (starting level)
- `initial_setpoint`: number (starting setpoint)
- `pid_gains`: object with properties Kc, tau_I, tau_D (all numbers)
- `timestep`: number (simulation dt)
- `history_capacity`: number (maximum history size)
- `history_size`: number (current history entries)

**Interface 3: HistoryPoint**
A single historical data point, with same fields as SimulationState.

**Interface 4: WebSocketMessage**
Union type for messages sent from client to server. Should support multiple message types with different payloads:
- setpoint: with value field (number)
- pid: with Kc, tau_I, tau_D fields (numbers)
- inlet_flow: with value field (number)
- inlet_mode: with mode field (string), plus min, max, variance for brownian mode

Use TypeScript union types or a discriminated union pattern for type safety.

### Structure Guidance

The file should have:
- Import statements at top (if using any types from external libraries)
- Interface definitions in logical order (data types first, then API response types)
- Export each interface for use in other files
- Brief comments describing the purpose of each interface

### Verification

Verify TypeScript compilation:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit lib/types.ts
```

Should complete without errors.

Check that the file exists and is syntactically valid:

```bash
grep "interface\|type" /home/roger/dev/tank_dynamics/frontend/lib/types.ts
```

Should show multiple interface/type definitions.

### Escalation Hints

**Escalate to Haiku if:**
- Unsure about exact field names or types from the backend API
- Need clarification on optional vs required fields

**Search for these terms if stuck:**
- "TypeScript interface optional properties"
- Check `/home/roger/dev/tank_dynamics/docs/project_docs/FASTAPI_API_REFERENCE.md` for exact API field names

### Acceptance Criteria
- [ ] File created at `frontend/lib/types.ts`
- [ ] SimulationState interface defined with all required fields
- [ ] ConfigResponse interface defined with all required fields
- [ ] HistoryPoint interface defined
- [ ] WebSocketMessage union type defined with all message types
- [ ] All interfaces are exported
- [ ] TypeScript compilation passes without errors

---

## Task 19f: Create Utility Helper Functions

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 19e (types.ts created)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/lib/utils.ts` (create new)

### Context and References

Utility functions are reusable helper functions used throughout the application.
- Reference: https://tailwindcss.com/docs/adding-custom-styles#using-css-and-tailwind-together
- Search keywords: "JavaScript utility functions" or "clsx classname merging"
- Escalation: If unsure about specific formatting requirements, escalate to Haiku

### Requirements

Create a new file `frontend/lib/utils.ts` containing utility helper functions for the frontend.

**Function 1: cn() - Class Name Merger**
Purpose: Conditionally combine Tailwind CSS class names for component styling.

Behavior:
- Accept variable number of class name arguments (strings, arrays, objects, or undefined)
- Filter out falsy values (undefined, null, false, empty strings)
- Concatenate remaining class names with spaces
- Return a single string of class names

Implementation hint: Use the `clsx` library or implement simple concatenation with filtering.

**Function 2: formatLevel(value: number) -> string**
Purpose: Format tank level values for display.

Behavior:
- Accept a number representing tank level in meters
- Return formatted string with 2 decimal places
- Example: 1.234 becomes "1.23"
- Handle null/undefined by returning "N/A"

**Function 3: formatFlowRate(value: number) -> string**
Purpose: Format flow rate values for display.

Behavior:
- Accept a number representing flow rate in m³/s
- Return formatted string with 3 decimal places
- Example: 0.00456 becomes "0.005"
- Handle null/undefined by returning "N/A"

**Function 4: formatValvePosition(value: number) -> string**
Purpose: Format valve position as percentage.

Behavior:
- Accept a number between 0 and 1 representing valve opening
- Return formatted string as percentage with 1 decimal place
- Example: 0.75 becomes "75.0%"
- Handle null/undefined by returning "N/A"

**Function 5: formatTime(seconds: number) -> string**
Purpose: Format simulation time in seconds to human-readable format.

Behavior:
- Accept a number of seconds
- Return formatted time string in MM:SS format for times under 60 minutes
- Return HH:MM:SS format for times 60 minutes or longer
- Example: 125 seconds becomes "02:05", 3661 seconds becomes "01:01:01"
- Handle null/undefined by returning "N/A"

**Function 6: clampValue(value: number, min: number, max: number) -> number**
Purpose: Constrain a value within a valid range.

Behavior:
- Accept a value and minimum/maximum bounds
- Return the value if within bounds
- Return min if value is below min
- Return max if value is above max
- Used for input validation before sending commands to backend

### Verification

Verify TypeScript compilation:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit lib/utils.ts
```

Should complete without errors.

Test a few functions manually in a Node REPL or check syntax:

```bash
grep "export function\|export const" /home/roger/dev/tank_dynamics/frontend/lib/utils.ts
```

Should show multiple exported functions.

### Escalation Hints

**Escalate to Haiku if:**
- Unsure about formatting precision for specific values
- Need clarification on rounding behavior

**Search for these terms if stuck:**
- "JavaScript number formatting"
- "TypeScript utility function patterns"

### Acceptance Criteria
- [ ] File created at `frontend/lib/utils.ts`
- [ ] cn() function exports (class name merger)
- [ ] formatLevel() function exports
- [ ] formatFlowRate() function exports
- [ ] formatValvePosition() function exports
- [ ] formatTime() function exports
- [ ] clampValue() function exports
- [ ] All functions are exported
- [ ] TypeScript compilation passes without errors

---

## Task 19g: ~~Create Global CSS Styles~~ [MERGED WITH TASK 19d]

**Note:** This task has been merged with Task 19d. Tailwind v4 configuration and global styles are now configured together in `frontend/app/globals.css` using the @import directive and @theme syntax. Task 19d handles both Tailwind setup and custom SCADA theme colors.

---

## Task 19h: Create Root Layout Component

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 19d (Tailwind and globals.css configured)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/app/layout.tsx` (create/modify)

### Context and References

Layout components in Next.js wrap all pages. If unfamiliar:
- Reference: https://nextjs.org/docs/app/building-your-application/routing/layouts-and-templates
- Search keywords: "Next.js App Router layout" or "Next.js RootLayout"
- Escalation: If React JSX syntax unclear, escalate to Haiku

### Requirements

Create the root layout component that wraps the entire application and establishes the HTML structure.

**Structure of layout.tsx:**

**Section 1: Import Statements**
- Import React types (ReactNode)
- Import metadata (Metadata type from Next.js)
- Import global CSS file

**Section 2: Metadata Export**
Define metadata for the page:
- title: "Tank Dynamics Simulator"
- description: "Real-time SCADA interface for tank level control"

**Section 3: RootLayout Component**
This is an async server component that accepts `children` prop.

Implement:
- HTML doctype and lang attribute (lang="en")
- Head section (auto-managed by Next.js, but can include metadata)
- Body element with:
  - `dark` class for dark theme (Tailwind dark mode)
  - `bg-gray-950` class for very dark background
  - `text-gray-100` class for light text
  - `overflow-x-hidden` to prevent horizontal scrolling
  - Children rendered inside body

**Section 4: Font Configuration (Optional but Recommended)**
If using custom fonts, import them here. For now, using system fonts is acceptable:
- Font stack: `-apple-system, BlinkMacSystemFont, "Segoe UI", Roboto`

### Verification

Verify TypeScript syntax and component structure:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit app/layout.tsx
```

Should complete without errors.

Check that file contains RootLayout export:

```bash
grep "export.*RootLayout\|function RootLayout" /home/roger/dev/tank_dynamics/frontend/app/layout.tsx
```

Should show the RootLayout component definition.

### Escalation Hints

**Escalate to Haiku if:**
- React JSX syntax is unclear
- Need help understanding Next.js layout component pattern

**Search for these terms if stuck:**
- "Next.js App Router RootLayout example"
- "React component JSX syntax"

### Acceptance Criteria
- [ ] File `frontend/app/layout.tsx` exists
- [ ] RootLayout component exported as default
- [ ] HTML structure with proper doctype and lang attribute
- [ ] Dark theme class applied to body
- [ ] Metadata export with title and description
- [ ] Global CSS imported
- [ ] Children rendered in body
- [ ] TypeScript compilation passes without errors

---

## Task 19i: Create Home Page Placeholder

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 19h (layout.tsx created)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/app/page.tsx` (create/modify)

### Context and References

Page components render specific routes in Next.js App Router.
- Reference: https://nextjs.org/docs/app/building-your-application/routing/pages
- Search keywords: "Next.js App Router page component"
- Escalation: If React component syntax unclear, escalate to Haiku

### Requirements

Create the home page component that serves as the root page (/) of the application.

For now, this is a placeholder that demonstrates the dark theme is working and provides a foundation for tabs and views (which will be added in Task 21).

**Content of page.tsx:**

**Section 1: Imports**
- Import React types if needed
- No external dependencies required yet

**Section 2: Page Component**
A React client component that renders:
- A main container with full viewport height and dark background
- Heading section with:
  - Application title: "Tank Dynamics Simulator"
  - Subtitle: "Real-time SCADA Interface"
- Content section with:
  - Brief description text
  - Status text that says "Status: Waiting for connection setup in next tasks"
  - Tailwind classes for spacing, typography, and colors

**Section 3: Styling with Tailwind**
Use Tailwind utility classes for:
- Full viewport height: `h-screen`
- Flexbox centering: `flex flex-col items-center justify-center`
- Dark background: `bg-gray-900` or `bg-gray-950`
- Light text: `text-white` or `text-gray-100`
- Padding: `p-8` or `p-10`
- Font sizes: `text-4xl` for title, `text-xl` for subtitle
- Spacing between elements: `gap-4` or `mb-6`

### Structure

The page should have a simple, centered layout:
```
┌─────────────────────────────────┐
│                                 │
│      Tank Dynamics Simulator    │
│     Real-time SCADA Interface   │
│                                 │
│   Description of the system     │
│   Status: Waiting for setup     │
│                                 │
└─────────────────────────────────┘
```

### Verification

Verify TypeScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit app/page.tsx
```

Should complete without errors.

Check file exists:

```bash
ls -la /home/roger/dev/tank_dynamics/frontend/app/page.tsx
```

### Escalation Hints

**Escalate to Haiku if:**
- React component syntax is unclear
- Need help with Tailwind class combinations

**Search for these terms if stuck:**
- "React functional component tutorial"
- "Tailwind CSS flexbox centering"

### Acceptance Criteria
- [ ] File `frontend/app/page.tsx` exists
- [ ] Exports default React component
- [ ] Page renders title, subtitle, and description
- [ ] Uses Tailwind classes for dark theme styling
- [ ] Centered layout with appropriate spacing
- [ ] TypeScript compilation passes without errors

---

## Task 19j: Configure Next.js Build Settings

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 19a (Next.js project initialized)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/next.config.js` (modify)

### Context and References

Next.js configuration controls build behavior and optimization.
- Reference: https://nextjs.org/docs/app/api-reference/next-config-js
- Search keywords: "next.config.js configuration" or "Next.js build settings"
- Escalation: Not needed for basic configuration

### Requirements

Modify the Next.js configuration file to set up build and development settings.

**Configuration Settings:**

**Section 1: React Strict Mode**
Enable React strict mode to catch potential bugs:
- `reactStrictMode: true`

This enables additional development checks (like double-mounting in development).

**Section 2: Dev Server Rewrites (Optional)**
Add rewrites to proxy API requests during development to avoid CORS issues:

Although the FastAPI backend already has CORS configured, this is good practice for future flexibility:
- Configure a rewrite rule that forwards `/api/*` requests to `http://localhost:8000/api/*` during development
- This allows frontend to call `fetch('/api/config')` instead of full URL

**Section 3: Experimental Features (Not Required)**
Leave experimental features disabled for stability unless specifically needed.

**Section 4: Environment Variables (Optional)**
The WebSocket URL will be configurable via `NEXT_PUBLIC_WS_URL` environment variable in .env.local file (created separately).

### Verification

Verify JavaScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
node -c next.config.js
```

Should complete without syntax errors.

Check file exists:

```bash
ls -la /home/roger/dev/tank_dynamics/frontend/next.config.js
```

### Escalation Hints

**Escalate to Haiku if:**
- Unsure about specific configuration options needed

**Search for these terms if stuck:**
- "Next.js next.config.js examples"

### Acceptance Criteria
- [ ] File `frontend/next.config.js` exists
- [ ] React strict mode enabled
- [ ] Rewrites configured for API proxy (optional)
- [ ] No syntax errors in configuration
- [ ] Configuration is valid JavaScript

---

## Task 19k: Test Project Initialization with Dev Server

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Tasks 19a-19j (all project setup complete)
**Estimated Time:** 15-30 minutes
**Files:** None - testing only

### Context and References

This task verifies that the entire project setup is working correctly before moving to component development.

### Requirements

Test that the Next.js development server can start and the application renders correctly.

**Steps to Execute:**

1. Navigate to frontend directory:
```bash
cd /home/roger/dev/tank_dynamics/frontend
```

2. Start the development server:
```bash
npm run dev
```

The server should start on http://localhost:3000 (or next available port if 3000 is in use).

3. Open browser to http://localhost:3000 and verify:
   - Page loads without errors
   - Dark theme is applied (dark background)
   - Title "Tank Dynamics Simulator" is visible
   - Subtitle "Real-time SCADA Interface" is visible
   - No console errors in browser DevTools

4. Test hot reload:
   - Edit `app/page.tsx` to change a text string
   - Save file
   - Browser should auto-refresh (or hot-reload) with updated content

5. Test TypeScript checking:
   - Run `npm run lint` in another terminal
   - Should complete without errors

6. Stop the dev server:
   - Press Ctrl+C in the terminal

### Verification

Expected outcomes:
- Dev server starts without errors
- Page renders with dark theme
- All content visible and readable
- Browser DevTools console has no errors
- Hot reload works (changes appear without manual refresh)
- Linting passes

### Troubleshooting

If the development server fails to start:
- Verify Node.js version is 18.17 or later: `node --version`
- Check if port 3000 is in use: `lsof -i :3000`
- Delete `node_modules` and `package-lock.json`, then reinstall: `uv sync`

If styling doesn't work:
- Verify Tailwind config scans the right files
- Check that `globals.css` is imported in layout.tsx
- Verify `dark` class is applied to body element

### Escalation Hints

**Escalate to Haiku if:**
- Dev server fails to start with unclear error
- Port 3000 issues that can't be resolved
- Styling completely broken (no colors applied)

**Search for these terms if stuck:**
- "Next.js dev server troubleshooting"
- "Tailwind CSS not working Next.js"

### Acceptance Criteria
- [ ] Dev server starts without errors
- [ ] Application renders at http://localhost:3000
- [ ] Dark theme is visually applied
- [ ] Title and subtitle are visible
- [ ] Browser console shows no errors
- [ ] Hot reload works (file changes appear immediately)
- [ ] ESLint passing (`npm run lint`)
- [ ] TypeScript compilation passing (`npx tsc --noEmit`)

---

## Task 20a: Create WebSocket Client Class - Basic Connection

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 19 complete (project structure)
**Estimated Time:** 15-30 minutes

**Files:** `frontend/lib/websocket.ts` (create)

### Context and References

WebSocket is a browser API for real-time bidirectional communication.
- Reference: https://developer.mozilla.org/en-US/docs/Web/API/WebSocket
- Search keywords: "JavaScript WebSocket client" or "WebSocket event handling"
- Escalation: If WebSocket API is unfamiliar, escalate to Haiku

### Requirements

Create a WebSocket client class that handles connection to the FastAPI backend at `ws://localhost:8000/ws`.

This task focuses on basic connection/disconnection logic. Reconnection logic will be added in Task 20b.

**Class Structure:**

Create a class called `WebSocketClient` with:

**Constructor:**
- Accept a URL parameter (string) for the WebSocket endpoint
- Store URL for later use
- Initialize a `connectionStatus` property (tracking: "connecting", "connected", "disconnected", "error")
- Initialize a `websocket` property to store the WebSocket instance (can be null)
- Initialize a callbacks object to store registered message handlers
- Set initial status to "disconnected"

**Public Methods:**

**connect() method:**
- Create a new WebSocket instance with the stored URL
- Set status to "connecting"
- Add event listeners:
  - `open` event: Set status to "connected", call any registered "connect" callbacks
  - `message` event: Parse JSON, call any registered "message" callbacks
  - `close` event: Set status to "disconnected", call any registered "disconnect" callbacks
  - `error` event: Set status to "error", log error, call any registered "error" callbacks
- Handle errors during initialization gracefully

**disconnect() method:**
- If WebSocket exists and is open, close it with code 1000 (normal closure)
- Set status to "disconnected"
- Clear event listeners
- Set websocket property to null

**on(event: string, callback: function) method:**
- Register callback for events: "connect", "message", "disconnect", "error"
- Store in callbacks object
- Return callback (or provide unsubscribe method) for cleanup

**Status Getter:**
- Property or method to get current connection status
- Return current status without modification

### Implementation Notes

- Do NOT include reconnection logic yet (that's Task 20b)
- Use native browser WebSocket API (no external libraries needed)
- Error handling: Log errors but don't throw (let component handle error state)
- Event handlers: Store callbacks in a way that allows multiple handlers per event
- TypeScript: Use proper types for callbacks (function types with parameters)

### Verification

Verify TypeScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit lib/websocket.ts
```

Should complete without errors.

Check that class exists:

```bash
grep "class WebSocketClient\|constructor(" /home/roger/dev/tank_dynamics/frontend/lib/websocket.ts
```

### Escalation Hints

**Escalate to Haiku if:**
- WebSocket event handling seems unclear
- Need help with callback registration pattern

**Search for these terms if stuck:**
- "WebSocket event listeners JavaScript"
- "WebSocket connection example MDN"

### Acceptance Criteria
- [ ] File `frontend/lib/websocket.ts` created
- [ ] WebSocketClient class defined
- [ ] Constructor accepts URL parameter
- [ ] connect() method creates WebSocket and adds event listeners
- [ ] disconnect() method closes WebSocket
- [ ] on() method allows registering callbacks
- [ ] Status tracking property/getter exists
- [ ] TypeScript compilation passes without errors
- [ ] No reconnection logic yet (added in next task)

---

## Task 20b: Add Message Sending Methods to WebSocket Class

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 20a (WebSocket class basic structure)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/lib/websocket.ts` (modify)

### Context and References

This task adds methods to send commands to the WebSocket server.
- Reference: Task 19e (types.ts) defines WebSocketMessage type
- Search keywords: "WebSocket send JSON" or "JSON serialization JavaScript"
- Escalation: Not needed (straightforward method additions)

### Requirements

Add public methods to the WebSocket client class for sending different command types to the server.

**Public Methods to Add:**

**sendSetpoint(value: number) method:**
- Accept a number representing desired tank level (setpoint)
- Validate: value should be between 0 and tank_height (use clampValue from utils)
- Create message object with:
  - type: "setpoint"
  - value: the validated value
- Serialize to JSON and send via WebSocket
- Handle if WebSocket not connected: log warning, don't crash

**sendPIDGains(Kc: number, tau_I: number, tau_D: number) method:**
- Accept three numbers for PID parameters
- Validate: all should be positive numbers
- Create message object with:
  - type: "pid"
  - Kc, tau_I, tau_D fields
- Serialize to JSON and send via WebSocket

**sendInletFlow(value: number) method:**
- Accept a number for inlet flow rate
- Validate: should be positive
- Create message object with:
  - type: "inlet_flow"
  - value: the validated value
- Serialize to JSON and send

**sendInletMode(mode: string, min: number, max: number, variance: number) method:**
- Accept mode ("constant" or "brownian") and parameters
- Validate: mode should be "constant" or "brownian"
- If "constant", only send type and mode (ignore other params)
- If "brownian", include min, max, variance params
- Create message object with type: "inlet_mode" plus appropriate fields
- Serialize to JSON and send

**Private Helper: sendMessage(data: object) method:**
- Helper method that all send methods use
- Check if WebSocket is connected: if not, log warning and return
- Serialize data to JSON
- Call `websocket.send(jsonString)`
- Handle errors gracefully (don't throw)

**Input Validation Helper:**
- Use clampValue utility from lib/utils.ts
- Or implement simple min/max checking
- Log warnings if input is outside expected range

### Implementation Notes

- All send methods should be defensive (handle disconnected state gracefully)
- Don't throw errors - log warnings and return
- Use TypeScript types from lib/types.ts for WebSocketMessage
- Each method should check connection status before sending
- Consider adding optional logging for debugging

### Verification

Verify TypeScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit lib/websocket.ts
```

Should complete without errors.

Check that send methods exist:

```bash
grep "send[A-Z]" /home/roger/dev/tank_dynamics/frontend/lib/websocket.ts
```

Should show multiple send methods.

### Escalation Hints

**Escalate to Haiku if:**
- Unsure about validation ranges for specific parameters

**Search for these terms if stuck:**
- "JavaScript JSON.stringify"
- "TypeScript method parameters and types"

### Acceptance Criteria
- [ ] sendSetpoint() method added
- [ ] sendPIDGains() method added
- [ ] sendInletFlow() method added
- [ ] sendInletMode() method added
- [ ] All send methods validate input ranges
- [ ] All send methods check connection status
- [ ] All send methods serialize to JSON and send via WebSocket
- [ ] TypeScript compilation passes without errors

---

## Task 20c: Add Reconnection Logic to WebSocket Class

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 20b (send methods added)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/lib/websocket.ts` (modify)

### Context and References

Reconnection logic provides resilience when connections drop.
- Search keywords: "exponential backoff algorithm" or "automatic reconnection pattern"
- Escalation: If exponential backoff math seems unclear, escalate to Haiku

### Requirements

Add automatic reconnection logic to the WebSocket class.

**Reconnection Behavior:**

**Exponential Backoff:**
- First reconnection attempt: 1 second delay
- Second attempt: 2 seconds delay
- Third attempt: 4 seconds delay
- Fourth attempt: 8 seconds delay
- Continue doubling until reaching maximum
- Maximum backoff delay: 30 seconds
- After reaching max, continue retrying at 30-second intervals

**Reconnection Conditions:**
- Only reconnect if the disconnect was unintentional (not from manual disconnect)
- Add a flag to track if disconnect was intentional
- Don't reconnect if user called disconnect() manually
- Do reconnect if connection drops unexpectedly (network issue)

**Implementation:**

Add to WebSocket class:

**Private Fields:**
- `reconnectAttempts`: counter for number of reconnection attempts
- `reconnectDelay`: current delay between reconnection attempts
- `reconnectTimer`: ID of pending reconnection timeout
- `intentionalDisconnect`: flag to track if disconnect was intentional

**Private Methods:**

**scheduleReconnect() method:**
- Calculate current backoff delay based on attempt count
- Set a timeout to call connect() after the delay
- Update reconnectDelay for next attempt
- Call "reconnecting" callback with delay information

**resetReconnect() method:**
- Called when connection succeeds
- Reset attempts to 0
- Reset delay to 1 second
- Clear any pending reconnect timeout

**Modify disconnect() method:**
- Accept optional parameter `intentional: boolean = true`
- If intentional, set flag to prevent reconnection
- Clear any pending reconnect timeout

**Modify close event handler:**
- Check if disconnect was intentional
- If not intentional, call scheduleReconnect()
- If intentional, don't schedule reconnection

### Verification

Verify TypeScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit lib/websocket.ts
```

Should complete without errors.

### Escalation Hints

**Escalate to Haiku if:**
- Exponential backoff calculation seems unclear
- Need help understanding the reconnection state machine

**Search for these terms if stuck:**
- "exponential backoff implementation"
- "JavaScript setTimeout/clearTimeout"

### Acceptance Criteria
- [ ] Exponential backoff logic implemented (1s, 2s, 4s, 8s, max 30s)
- [ ] Reconnection attempts counter working
- [ ] Only reconnects on unintentional disconnects
- [ ] Reconnection timeout properly scheduled
- [ ] Backoff resets on successful connection
- [ ] Manual disconnect prevents reconnection
- [ ] TypeScript compilation passes without errors

---

## Task 20d: Create useWebSocket React Hook

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 20c (WebSocket class complete), Task 19e (types.ts)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/hooks/useWebSocket.ts` (create)

### Context and References

React hooks integrate functionality with component lifecycle.
- Reference: https://react.dev/reference/react/useEffect and https://react.dev/reference/react/useState
- Search keywords: "React custom hooks" or "useEffect cleanup pattern"
- Escalation: If React hooks are unfamiliar, escalate to Haiku

### Requirements

Create a custom React hook that wraps the WebSocket client class and integrates with React lifecycle.

**Hook Signature:**

```
function useWebSocket(): {
  state: SimulationState | null;
  connectionStatus: 'connecting' | 'connected' | 'disconnected' | 'error';
  error: string | null;
  setSetpoint: (value: number) => void;
  setPIDGains: (Kc: number, tau_I: number, tau_D: number) => void;
  setInletFlow: (value: number) => void;
  setInletMode: (mode: string, min: number, max: number, variance: number) => void;
  reconnect: () => void;
}
```

**Implementation Requirements:**

**"use client" Directive:**
- Add `"use client"` at top of file (this is a client-side hook)

**Import Statements:**
- Import React hooks: useState, useEffect, useRef, useCallback
- Import WebSocketClient class
- Import types from lib/types.ts
- Import SimulationState type

**State Management:**
- Use useState for simulation state (initial value: null)
- Use useState for connection status (initial: "disconnected")
- Use useState for error (initial: null)

**Instance Management:**
- Use useRef to store WebSocket instance (persists across renders)
- Initialize lazily on first render (inside useEffect)

**Effect Hooks:**

**Effect 1: Initialize Connection**
- Run on mount only (empty dependency array)
- Create WebSocket instance with URL from environment: `process.env.NEXT_PUBLIC_WS_URL || 'ws://localhost:8000/ws'`
- Register callbacks for connect, message, disconnect, error
- Return cleanup function that disconnects on unmount

**Effect 2: Handle Messages**
- Register callback for "message" events
- When message received:
  - Validate it matches SimulationState type
  - Update state using setState callback
- This effect might be part of the same useEffect as connection setup

**Callback Functions:**

Use useCallback to create stable functions:

**setSetpoint callback:**
- Accept value: number
- Call websocket.sendSetpoint(value)
- Catch any errors and update error state

**setPIDGains callback:**
- Accept Kc, tau_I, tau_D numbers
- Call websocket.sendPIDGains(Kc, tau_I, tau_D)

**setInletFlow callback:**
- Accept value: number
- Call websocket.sendInletFlow(value)

**setInletMode callback:**
- Accept mode, min, max, variance
- Call websocket.sendInletMode(mode, min, max, variance)

**reconnect callback:**
- Call websocket.connect()

**Return Value:**
- Return object with all state and callback functions

### Verification

Verify TypeScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit hooks/useWebSocket.ts
```

Should complete without errors.

Check that hook is exported:

```bash
grep "export.*useWebSocket" /home/roger/dev/tank_dynamics/frontend/hooks/useWebSocket.ts
```

### Escalation Hints

**Escalate to Haiku if:**
- React useEffect cleanup is unclear
- Need help understanding hook dependencies
- useCallback vs useState seems confusing

**Search for these terms if stuck:**
- "React useEffect cleanup"
- "React custom hooks tutorial"

### Acceptance Criteria
- [ ] File `frontend/hooks/useWebSocket.ts` created
- [ ] "use client" directive at top
- [ ] Hook uses useState for connection state, error, simulation state
- [ ] Hook uses useRef for WebSocket instance
- [ ] useEffect handles mount/unmount lifecycle
- [ ] Message callbacks update simulation state
- [ ] All send methods wrapped as useCallback functions
- [ ] Hook returns correct shape with all required properties
- [ ] TypeScript compilation passes without errors

---

## Task 20e: Create SimulationProvider Context

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 20d (useWebSocket hook), Task 19 (project structure)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/app/providers.tsx` (create)

### Context and References

React Context provides a way to pass data through the component tree without props.
- Reference: https://react.dev/reference/react/createContext
- Search keywords: "React Context provider pattern" or "useContext hook"
- Escalation: If Context pattern is unclear, escalate to Haiku

### Requirements

Create a Context provider that makes simulation state available to all components in the app.

**File Structure:**

**Part 1: Create SimulationContext**
- Use `React.createContext()` to create context
- Context value type matches useWebSocket return type (all state and callbacks)
- Provide default values (null state, disconnected, no-op functions)

**Part 2: Create SimulationProvider Component**
- Accepts children prop (React.ReactNode)
- Calls useWebSocket hook internally (single instance for entire app)
- Wraps children with context provider, passing hook return as value

**Part 3: Create useSimulation Hook**
- Calls useContext(SimulationContext)
- Checks if context is used inside provider
- Throw error if used outside provider: "useSimulation must be used within SimulationProvider"
- Return context value

### Implementation Notes

- SimulationProvider should be marked with "use client" (uses hooks)
- Provider should wrap entire app (in layout.tsx)
- useSimulation hook makes context available to child components
- Default context values allow IDE intellisense even outside provider
- Error check prevents silent bugs from using hook outside provider

### Integration Step (Not Required for This Task)

Later (Task 21), we'll update app/layout.tsx to wrap the app with SimulationProvider.

### Verification

Verify TypeScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit app/providers.tsx
```

Should complete without errors.

Check that exports exist:

```bash
grep "export.*SimulationProvider\|export.*useSimulation" /home/roger/dev/tank_dynamics/frontend/app/providers.tsx
```

Should show both exports.

### Escalation Hints

**Escalate to Haiku if:**
- React Context pattern is confusing
- TypeScript types for context value seem unclear

**Search for these terms if stuck:**
- "React Context example"
- "useContext hook tutorial"

### Acceptance Criteria
- [ ] File `frontend/app/providers.tsx` created
- [ ] "use client" directive at top
- [ ] SimulationContext created with createContext
- [ ] SimulationProvider component wraps children with provider
- [ ] useSimulation hook exports context consumer
- [ ] Error thrown if useSimulation used outside provider
- [ ] Context value type matches useWebSocket return
- [ ] TypeScript compilation passes without errors

---

## Task 20f: Update Root Layout to Use SimulationProvider

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 20e (SimulationProvider created), Task 19h (layout.tsx exists)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/app/layout.tsx` (modify)

### Context and References

This task integrates the Context provider into the app root layout.
- Reference: Task 20e for provider structure
- Search keywords: "React component composition" or "Next.js layout structure"
- Escalation: Not needed (straightforward integration)

### Requirements

Modify the root layout component to wrap the app content with SimulationProvider.

**Changes to layout.tsx:**

**Add Import:**
- Import SimulationProvider from `./providers`

**Update RootLayout Component Body:**
- Wrap the children element with `<SimulationProvider>`
- Keep other elements (html, body, etc.) unchanged
- Structure should be:
  ```
  <html lang="en">
    <body className="...">
      <SimulationProvider>
        {children}
      </SimulationProvider>
    </body>
  </html>
  ```

**Effect:**
- Now all child components can call `useSimulation()` hook
- Single WebSocket connection for entire app
- Context provides state and callbacks to all components

### Verification

Verify TypeScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit app/layout.tsx
```

Should complete without errors.

Check that import exists:

```bash
grep "SimulationProvider" /home/roger/dev/tank_dynamics/frontend/app/layout.tsx
```

### Escalation Hints

**Escalate to Haiku if:**
- Unsure about component composition order
- Need clarification on how provider wraps children

**Search for these terms if stuck:**
- "React component composition patterns"
- "Next.js layout.tsx provider"

### Acceptance Criteria
- [ ] Layout.tsx imports SimulationProvider
- [ ] Children wrapped with SimulationProvider in JSX
- [ ] Provider is inside body element
- [ ] Other layout structure unchanged
- [ ] TypeScript compilation passes without errors

---

## Task 21a: Create TabNavigation Component

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 19 (Tailwind configured), Task 20 (WebSocket working)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/components/TabNavigation.tsx` (create)

### Context and References

Tab navigation is a common UI pattern for switching views.
- Reference: https://www.w3.org/WAI/ARIA/apg/patterns/tabs/
- Search keywords: "React tab component" or "accessible tabs ARIA"
- Escalation: If accessibility attributes seem unclear, escalate to Haiku

### Requirements

Create a tab navigation component that switches between views.

**Props Interface:**

The component accepts:
- `activeTab`: string ("process" or "trends")
- `onTabChange`: callback function receiving tab name string

**Tabs:**
- Tab 1: label "Process"
- Tab 2: label "Trends"

**Visual Design:**

Using Tailwind classes:
- Container with flex layout, horizontal direction
- Each tab button with padding (px-6, py-3)
- Active tab: different background color (e.g., bg-blue-600)
- Inactive tabs: lighter color (e.g., bg-gray-800)
- Hover state on inactive tabs
- Border-bottom on active tab (optional, for visual clarity)
- Smooth transition animation on tab click

**Keyboard Navigation:**

Add accessibility attributes:
- Container: `role="tablist"`
- Each tab button: `role="tab"`, `aria-selected={true/false}`, `aria-controls={tabPanelId}`
- Keyboard support: arrow keys to navigate tabs (optional but good practice)

**Implementation Notes:**

- Component is presentational only (doesn't manage state)
- Parent component (page.tsx in Task 21) manages activeTab state
- Component just renders UI and calls onTabChange callback
- Should use Tailwind classes exclusively for styling

### Verification

Verify TypeScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit components/TabNavigation.tsx
```

Should complete without errors.

Check file exists:

```bash
ls -la /home/roger/dev/tank_dynamics/frontend/components/TabNavigation.tsx
```

### Escalation Hints

**Escalate to Haiku if:**
- ARIA attributes seem unclear
- Need help with keyboard event handling

**Search for these terms if stuck:**
- "ARIA tabs pattern W3C"
- "React button onClick handler"

### Acceptance Criteria
- [ ] Component created in `frontend/components/TabNavigation.tsx`
- [ ] Accepts activeTab and onTabChange props
- [ ] Renders two tabs: "Process" and "Trends"
- [ ] Active tab is visually distinct
- [ ] Click handler calls onTabChange with correct tab name
- [ ] ARIA attributes present for accessibility
- [ ] Uses Tailwind classes for styling
- [ ] TypeScript compilation passes without errors

---

## Task 21b: Create ConnectionStatus Indicator Component

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 20e (useSimulation hook available), Task 21a (TabNavigation created)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/components/ConnectionStatus.tsx` (create)

### Context and References

This component displays the WebSocket connection state to the user.
- Reference: Task 20d (useSimulation hook) provides connection status
- Search keywords: "React status indicator" or "real-time status display"
- Escalation: Not needed (straightforward component)

### Requirements

Create a component that displays the WebSocket connection status.

**Functionality:**

**Consume Simulation State:**
- Use `useSimulation()` hook to get connectionStatus and error
- Component re-renders when connection status changes

**Display States:**

- **"connected"**: Green dot + "Connected" text
- **"connecting"**: Yellow/amber dot + "Connecting..." text
- **"disconnected"**: Red dot + "Disconnected" text
- **"error"**: Red dot + "Connection Error" text + show error message

**Visual Indicators:**

- Colored dot using semantic colors:
  - Green: bg-green-500 (connected)
  - Amber: bg-yellow-500 (connecting)
  - Red: bg-red-500 (disconnected/error)
  - Dot size: 12px diameter (w-3 h-3)
- Text label next to dot
- Optional: status details (error message if error state)
- Position: top-right corner of screen or near tabs

**CSS Classes:**

Use Tailwind for:
- Flex layout with gap (flex items-center gap-2)
- Absolute positioning (fixed or absolute)
- Padding and margin
- Text sizing (text-sm)
- Color classes for backgrounds and text

**Optional Enhancements:**

- Pulse animation for "connecting" state (use Tailwind animate-pulse)
- Tooltip showing last update time (optional)
- Expandable error details section (optional)

### Verification

Verify TypeScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit components/ConnectionStatus.tsx
```

Should complete without errors.

Check file exists:

```bash
ls -la /home/roger/dev/tank_dynamics/frontend/components/ConnectionStatus.tsx
```

### Escalation Hints

**Escalate to Haiku if:**
- useSimulation hook behavior is unclear
- Need help with Tailwind animation classes

**Search for these terms if stuck:**
- "Tailwind CSS colors semantic"
- "Tailwind animate-pulse animation"

### Acceptance Criteria
- [ ] Component created in `frontend/components/ConnectionStatus.tsx`
- [ ] Calls useSimulation() hook to get connection status
- [ ] Renders colored indicator dot based on status
- [ ] Shows appropriate status text
- [ ] Styled with Tailwind classes
- [ ] Uses semantic colors (green, amber, red)
- [ ] TypeScript compilation passes without errors
- [ ] Component updates when connectionStatus changes

---

## Task 21c: Create ProcessView Placeholder Component

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 20d (useSimulation hook), Task 19f (utility functions), Task 21b (connection status)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/components/ProcessView.tsx` (create)

### Context and References

This is a placeholder view that demonstrates WebSocket data flow.
- Reference: Task 20d (useSimulation) and Task 19f (formatLevel, formatFlowRate, etc.)
- Search keywords: "React data display component" or "real-time dashboard"
- Escalation: Not needed for placeholder (straightforward display)

### Requirements

Create a placeholder component for the Process View that displays current simulation state.

**Functionality:**

**Consume Simulation State:**
- Use `useSimulation()` hook
- Access: state, connectionStatus
- Update in real-time as state changes (1 Hz)

**Content:**

- Heading: "Process View"
- Subheading: "Tank visualization and real-time controls (coming next)"
- Connection Status component (import and include)
- Data section showing current values:
  - Simulation Time: [formatted time from state.time]
  - Tank Level: [formatted level from state.level]
  - Setpoint: [formatted from state.setpoint]
  - Inlet Flow: [formatted from state.inlet_flow]
  - Outlet Flow: [formatted from state.outlet_flow]
  - Valve Position: [formatted as percentage from state.valve_position]
- Waiting message: "Waiting for WebSocket connection..." if state is null

**Formatting:**

Use utility functions from `lib/utils.ts`:
- formatLevel() for tank level and setpoint
- formatFlowRate() for inlet/outlet flows
- formatValvePosition() for valve position
- formatTime() for simulation time

**Layout:**

Using Tailwind:
- Card/panel styling with dark background (bg-gray-800)
- Padding and margin for spacing
- Data displayed as key-value pairs (grid or list)
- Monospace font for numeric values (font-mono)
- Heading hierarchy with appropriate sizes

**Visual Hierarchy:**

- Prominent heading at top
- Connection status in top-right
- Data values clearly readable
- Use spacing to organize information

### Verification

Verify TypeScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit components/ProcessView.tsx
```

Should complete without errors.

Check file exists:

```bash
ls -la /home/roger/dev/tank_dynamics/frontend/components/ProcessView.tsx
```

### Escalation Hints

**Escalate to Haiku if:**
- useSimulation hook not providing expected data
- Need help with conditional rendering (if state is null)

**Search for these terms if stuck:**
- "React conditional rendering"
- "React component data display"

### Acceptance Criteria
- [ ] Component created in `frontend/components/ProcessView.tsx`
- [ ] Calls useSimulation() hook
- [ ] Displays heading and description
- [ ] Shows current simulation values when connected
- [ ] Uses utility functions for formatting
- [ ] Shows waiting message when disconnected
- [ ] Includes ConnectionStatus component
- [ ] Data updates in real-time (1 Hz)
- [ ] Styled with Tailwind classes
- [ ] TypeScript compilation passes without errors

---

## Task 21d: Create TrendsView Placeholder Component

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Task 20d (useSimulation hook), Task 21c (ProcessView created)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/components/TrendsView.tsx` (create)

### Context and References

This is a placeholder for future charting features.
- Reference: Task 20d (useSimulation) for state access
- Search keywords: "React data history" or "real-time data buffer"
- Escalation: Not needed (placeholder with simple display)

### Requirements

Create a placeholder component for the Trends View.

**Functionality:**

**Consume Simulation State:**
- Use `useSimulation()` hook
- Store recent state updates in component state
- Keep rolling history of last 10-20 state snapshots

**Content:**

- Heading: "Trends View"
- Subheading: "Historical process trends and analytics"
- Message: "Trend charts will be implemented in Phase 4 continued"
- Recent Data Display: Show last 10 state updates as simple list or table
  - Display: Time, Level, Setpoint, Inlet Flow, Outlet Flow
  - Format numbers appropriately
  - Show newest data first (reverse chronological)

**State Management:**

- Use useState hook to maintain history array
- Use useEffect to subscribe to state updates
- When new state arrives, add to beginning of array
- Keep only last 10 items (trim oldest if exceeds 10)

**Layout:**

Using Tailwind:
- Card/panel styling
- Heading at top
- Data table or list below
- Use monospace font for numeric data
- Light text on dark background
- Scrollable if data exceeds viewport

**Placeholder Style:**

- Make it clear this is a placeholder
- Include note about future enhancements
- Still show data to prove WebSocket is working

### Verification

Verify TypeScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit components/TrendsView.tsx
```

Should complete without errors.

Check file exists:

```bash
ls -la /home/roger/dev/tank_dynamics/frontend/components/TrendsView.tsx
```

### Escalation Hints

**Escalate to Haiku if:**
- useState for managing history array seems unclear
- Need help with array trimming logic

**Search for these terms if stuck:**
- "React useState array management"
- "JavaScript array slice/trim"

### Acceptance Criteria
- [ ] Component created in `frontend/components/TrendsView.tsx`
- [ ] Calls useSimulation() hook
- [ ] Displays heading and placeholder message
- [ ] Shows recent state history (last 10 updates)
- [ ] Updates in real-time as new states arrive
- [ ] Data displayed in reverse chronological order
- [ ] Numbers formatted appropriately
- [ ] Styled with Tailwind classes
- [ ] TypeScript compilation passes without errors

---

## Task 21e: Update Home Page with Tab Navigation and Views

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Tasks 21a-21d (all components created)
**Estimated Time:** 15-30 minutes
**Files:** `frontend/app/page.tsx` (modify)

### Context and References

This task integrates all the components into the main page layout.
- Reference: Tasks 21a-21d for component imports
- Search keywords: "React conditional rendering" or "component composition"
- Escalation: Not needed (straightforward integration)

### Requirements

Update the home page component to include the tab navigation and both view components.

**Structure:**

The page should have this layout:

```
┌─────────────────────────────────────────┐
│  Header                                 │
│  Tank Dynamics Simulator                │
│  Real-time SCADA Interface              │
│                              [Status]   │
├─────────────────────────────────────────┤
│ [Process]  [Trends]                     │
├─────────────────────────────────────────┤
│                                         │
│   Current Active View                   │
│   (ProcessView or TrendsView)           │
│                                         │
└─────────────────────────────────────────┘
```

**Implementation:**

**State Management:**
- Use useState to track activeTab (default: "process")
- Initialize as "process" tab

**Sections:**

**Section 1: Header**
- Application title: "Tank Dynamics Simulator"
- Subtitle: "Real-time SCADA Interface"
- ConnectionStatus component in top-right
- Use Tailwind for layout and styling

**Section 2: TabNavigation**
- Pass activeTab and onTabChange to TabNavigation component
- onTabChange updates the activeTab state

**Section 3: Main Content Area**
- Conditionally render ProcessView if activeTab === "process"
- Conditionally render TrendsView if activeTab === "trends"
- Both components stay mounted (just hidden), or unmount on switch (either approach is fine)
- Fill remaining vertical space

**Styling:**

Using Tailwind:
- Full viewport height (h-screen)
- Flexbox column layout
- Dark background and light text
- Header with border-bottom separator
- Main content area fills available space
- Padding and gaps for spacing

**Optional Enhancements:**

- Smooth transition animation when switching tabs
- Keep tab state in URL query param (for page refresh persistence)

### Verification

Verify TypeScript syntax:

```bash
cd /home/roger/dev/tank_dynamics/frontend
npx tsc --noEmit app/page.tsx
```

Should complete without errors.

Check imports:

```bash
grep "import.*View\|import.*Tab" /home/roger/dev/tank_dynamics/frontend/app/page.tsx
```

### Escalation Hints

**Escalate to Haiku if:**
- Conditional rendering logic seems unclear
- Component import paths not resolving

**Search for these terms if stuck:**
- "React conditional rendering {condition ? true : false}"
- "React component import and usage"

### Acceptance Criteria
- [ ] page.tsx imports TabNavigation, ProcessView, TrendsView
- [ ] useState tracks activeTab state
- [ ] Header with title, subtitle, ConnectionStatus component
- [ ] TabNavigation component displays with activeTab and onTabChange
- [ ] Conditional rendering shows correct view based on activeTab
- [ ] Both views render their content properly
- [ ] Full viewport height layout with proper spacing
- [ ] Switching tabs updates displayed view immediately
- [ ] Dark theme styling applied
- [ ] TypeScript compilation passes without errors

---

## Task 21f: Test Complete Frontend Application

**Phase:** 4 - Next.js Frontend
**Prerequisites:** Tasks 19-21 complete (entire frontend built)
**Estimated Time:** 15-30 minutes
**Files:** None - testing only

### Context and References

This is the integration test for all frontend components with the backend.

### Requirements

Test the complete frontend application with both dev server and backend running.

**Setup:**

1. Ensure FastAPI backend is running:
```bash
cd /home/roger/dev/tank_dynamics
uv run python -m tank_dynamics.api.server
```

Should show: "Uvicorn running on http://127.0.0.1:8000"

2. Start Next.js dev server:
```bash
cd /home/roger/dev/tank_dynamics/frontend
npm run dev
```

Should show: "Ready in Xs on http://localhost:3000"

**Visual Tests:**

1. Open browser to http://localhost:3000
2. Verify page loads without errors
3. Verify dark theme is applied
4. Verify title and subtitle visible
5. Verify connection status indicator is visible (should show green "Connected")
6. Verify two tabs visible: "Process" and "Trends"
7. Verify "Process" tab is active by default (visually distinct)
8. Verify ProcessView shows current values:
   - Tank Level
   - Setpoint
   - Inlet Flow
   - Outlet Flow
   - Valve Position
   - Simulation Time
9. Verify values update approximately once per second (WebSocket receiving at 1 Hz)

**Interaction Tests:**

1. Click "Trends" tab
2. Verify tab switches (visual change)
3. Verify TrendsView displays
4. Verify recent data history shown
5. Click "Process" tab
6. Verify tab switches back
7. Verify ProcessView displays again
8. Repeat tab switching several times (should be smooth)

**WebSocket Connection Tests:**

1. Open browser DevTools, Network tab, filter for WS
2. Should see WebSocket connection to ws://localhost:8000/ws
3. Should see messages flowing (one per second)
4. Each message should contain simulation state data

**Reconnection Test:**

1. Leave frontend running
2. Stop backend server (Ctrl+C)
3. Verify connection status changes to "Disconnected" or "Error"
4. Verify data display shows last received values or "Waiting for connection"
5. Restart backend server
6. Verify connection status returns to "Connected" within ~30 seconds
7. Verify data updates resume

**Browser Console:**

1. Open browser DevTools Console tab
2. Should be NO error messages
3. Should be NO warning messages related to React or WebSocket
4. Should see "Connected" or connection status logs (if logging implemented)

### Verification

If all tests pass:
- Frontend application is fully functional
- WebSocket connection works
- Tab navigation works
- Components display correctly
- Real-time data updates working
- Reconnection working

### Troubleshooting

If frontend shows "Disconnected":
- Verify backend is running on port 8000
- Check browser console for errors
- Check backend logs for connection errors
- Verify firewall not blocking WebSocket

If styling looks broken:
- Check Tailwind configuration
- Verify globals.css imported in layout.tsx
- Clear browser cache (Ctrl+Shift+Delete)
- Restart dev server

If WebSocket shows errors:
- Check WebSocket URL in environment variables
- Verify backend CORS configuration
- Check browser console for specific error messages
- Verify backend WebSocket endpoint exists at /ws

### Escalation Hints

**Escalate to Haiku if:**
- Unexplained errors in multiple areas
- WebSocket connection completely not working
- Dark theme completely missing

**Search for these terms if stuck:**
- "Next.js WebSocket development"
- "CORS WebSocket issue"

### Acceptance Criteria
- [ ] Frontend dev server starts without errors
- [ ] Application loads at http://localhost:3000
- [ ] Dark theme applied correctly
- [ ] WebSocket connection established (green indicator)
- [ ] ProcessView shows simulation values
- [ ] Values update approximately every 1 second
- [ ] Tab switching works smoothly
- [ ] TrendsView displays recent data history
- [ ] Reconnection works after backend restart
- [ ] No errors in browser console
- [ ] No errors in terminal output
- [ ] All components render correctly

---

## Summary of Phase 4 Micro-Tasks

| Task | File(s) | Focus | Status |
|------|---------|-------|--------|
| 19a | - (command) | Initialize Next.js project | **COMPLETE** ✓ |
| 19ba | - (command) | Install Tailwind CSS v4 | Ready to start |
| 19b | package.json | Install recharts dependency | Depends on 19ba |
| 19c | tsconfig.json | TypeScript configuration | Ready to start |
| 19d | app/globals.css | Tailwind v4 config + dark theme | Depends on 19ba |
| 19e | lib/types.ts | API type definitions | Ready to start |
| 19f | lib/utils.ts | Utility helper functions | Ready to start |
| 19g | ~~app/globals.css~~ | **MERGED WITH 19d** | N/A |
| 19h | app/layout.tsx | Root layout component | Depends on 19d |
| 19i | app/page.tsx | Home page placeholder | Depends on 19h |
| 19j | next.config.ts | Next.js configuration | Ready to start |
| 19k | - (testing) | Test dev server and build | Depends on 19a-19j |
| 20a | lib/websocket.ts | WebSocket basic connection | Ready to start |
| 20b | lib/websocket.ts | WebSocket message sending | Depends on 20a |
| 20c | lib/websocket.ts | WebSocket reconnection logic | Depends on 20b |
| 20d | hooks/useWebSocket.ts | React hook wrapping WebSocket | Depends on 20c |
| 20e | app/providers.tsx | Context provider for state | Depends on 20d |
| 20f | app/layout.tsx | Integrate provider in layout | Depends on 20e |
| 21a | components/TabNavigation.tsx | Tab UI component | Depends on 19d |
| 21b | components/ConnectionStatus.tsx | Connection indicator | Depends on 20d |
| 21c | components/ProcessView.tsx | Process view placeholder | Depends on 20d |
| 21d | components/TrendsView.tsx | Trends view placeholder | Depends on 20d |
| 21e | app/page.tsx | Integrate all components | Depends on 21a-21d |
| 21f | - (testing) | End-to-end integration test | Depends on all above |

**Total micro-tasks: 23 (19ba added, 19g merged into 19d)**
**Estimated time: 5-7 hours total (15-30 min per task)**

**Key Changes from Original Plan:**
- Task 19a is complete - Next.js 16 project initialized
- Task 19ba added - Tailwind CSS v4 requires separate installation
- Task 19g merged into 19d - Tailwind v4 uses CSS-based configuration
- All file references updated to match actual Next.js 16 structure (next.config.ts, eslint.config.mjs)

---

## Upcoming Work (After Phase 4 Foundation Complete)

### Phase 4 Continued: Advanced Features

**Task 22: Process View - Tank Visualization and Basic Controls**
- SVG tank graphic with animated fill
- Flow indicators with arrows
- Basic input controls for setpoint
- Integration with WebSocket commands

**Task 23: Process View - PID Control Panel**
- PID gains input controls
- Current PID state display
- Preset tuning configurations

**Task 24: Process View - Inlet Mode Controls**
- Mode selector (constant vs. Brownian)
- Parameter inputs for Brownian mode
- Mode-specific help text

**Task 25: Trends View - Recharts Integration**
- Historical data fetching
- Multi-plot trend charts (LineChart component)
- Responsive sizing with ResponsiveContainer
- **Important for v3:** CartesianGrid requires explicit `xAxisId`/`yAxisId` props
- Reference: [Recharts 3.0 API examples](https://github.com/recharts/recharts)
- Use new React hooks: `useChartWidth`, `useChartHeight` for custom components

**Task 26: Trends View - Time Range Selector**
- Dropdown for time range selection
- Dynamic chart updates

**Task 27: UI Polish and Production Ready**
- Loading spinners
- Error boundaries
- Toast notifications
- Production build optimization

---

## Development Workflow Notes

### Context Preservation

**Between Tasks:**
- Each task is designed to be completable independently
- Clear prerequisites specify what must be done before
- If switching to a different task, save progress with git commit
- Clear context by closing IDE/REPL between different tasks

### Dependency Management

**uv vs npm:**
- Use `uv` for Python packages in the backend
- Use `npm` for frontend dependencies (uv syncs lock file)
- Never mix pip and uv
- Never mix npm and yarn

### Testing Strategy

**Per-Task Verification:**
- Each task includes a simple command to verify completion
- Don't wait until end to test - test each task immediately
- If test fails, fix before moving to next task

**Integration Tests:**
- Task 21f is the integration test combining all components
- Only reach Task 21f after all prior tasks complete
- If integration test fails, check each component individually

### Git Workflow

After each task:
```bash
git add <files-modified>
git commit -m "Task Xz: Brief description"
```

For example:
```bash
git add frontend/app/layout.tsx
git commit -m "Task 19h: Create root layout component with dark theme"
```

### Common Patterns

**When importing from lib/:**
```typescript
import { formatLevel, formatFlowRate } from '@/lib/utils'
```

**When using hooks:**
```typescript
'use client'
import { useSimulation } from '@/app/providers'

export default function MyComponent() {
  const { state, connectionStatus } = useSimulation()
  // component code
}
```

**When creating Tailwind variants:**
- Define in tailwind.config.ts extend.colors
- Use in components: `className="bg-process-normal"`
- No manual CSS needed

---

## Key Principles for Local LLM Success

### 1. Task Independence
Each task can be completed without reading others. Prerequisites list what's needed.

### 2. Reference-First Design
Tasks include links to relevant documentation and search keywords. If stuck, search first before asking for help.

### 3. Escalation Clarity
Each task specifies exactly when to escalate to Haiku/Sonnet. This prevents getting stuck while preserving resources.

### 4. Verification at Scale
Simple one-command verification for each task prevents accumulated errors.

### 5. Structure Over Flexibility
Exact file paths, exact prop names, exact behavior specifications. Leaves no room for ambiguity.

---

## Notes on Architecture Decisions

### Why Context Instead of Props?

The entire app needs access to simulation state. Props would require drilling through many levels. Context is cleaner for this use case, even though it's slightly more setup initially.

### Why Separate WebSocket Client and Hook?

The WebSocket client is a plain TypeScript class that could be used anywhere (tests, workers, other frameworks). The React hook is the adapter for React components. Separating concerns keeps code testable and reusable.

### Why 1 Hz Update Rate?

- Matches typical SCADA systems (human operators, not high-frequency trading)
- Makes debugging easier (changes visible to human observation)
- Reduces network traffic and processing
- Simulation physics don't require faster updates for learning

### Why Tailwind Over CSS-in-JS?

- Faster development iteration
- Smaller bundle size than styled-components
- Better IDE support
- Dark theme is built-in and easy to configure

### Why Next.js Over React SPA?

- Built-in optimization and code splitting
- App Router provides better organization
- Server components reduce client-side JavaScript
- Better for SCADA (static-first, progressive enhancement)

---

**Status: Phase 4 Foundation Ready for Implementation**

The micro-task breakdown is complete and ready for local LLM execution. Each task is independently testable and suitable for models with limited context windows. Start with Task 19a and proceed sequentially.

For questions about specific tasks, refer to the task details and reference links. For pattern clarification not covered in task details, search the provided keywords before escalating.
