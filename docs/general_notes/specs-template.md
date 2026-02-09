# Project Specification

## Overview

This is a simulation of a tank.  It will look like a scada screen that a process operator would use.  The user will be able to monitor process conditions and adjust varibles (PID) settings to tune the process.

## Goals

- This project will develop a dynamic model of a tank with variable inlet flow and and outlet flow controlled by a PID loop controlling tank level.
- We build the logic for the ODEs and algerbric equations in cpp.
- We will use the RK4 ODE solver provided by the GNU scientific library with fixed time step
- We will use Eigen for matricies and matrix manipulation
- The state vector consists of one variable - h or height of fluid in the tank.
- The manipulated variable vector consists of severak variables - qin, the liquid flowrate into the tank and x, the position of the valve, and the PID settings.  The tank level setpoint.
- We will need to store the manipulated varibles and state variables.  For display in the front end as well as for use later in a machine learing library as data.

## Non-Goals

- No scope drift.  This needs to be a proof of concept before taking on furher work.

## Users

1. A process operator using the tank SCADA screen to experiment with tank control and PID settings.

## Features

### Feature 1: SCADA Interface

**Description:** Allows the user to control the simulation.  It looks like a screen you would see in a control room.  A tank will be represented in the screen and all measured process variables will be visible.  One would normally be able to make custom plots, but this simulation will have a second screen that shows a pre-define set of plots.  The PID seetings will be availble on the screen to change.

**User Story:** As a conrol room operator I want to use the simuator so I can understand how these pocesses work better.

**Requirements:**
- A screen represntig the process with all process varibles present.
- All manipulated variables on the screen so the operator can contorl them.
- A second screen that allows the opertor to view process trends.
  - A tank level against setpont plot 
  - Inlet flow vs outlet flow
  - any others
  - Written as a next.js app.
  
**Acceptance Criteria:**
- [ ] The user can see the state of the process
- [ ] The user can manipulate the process with the tools at his disposal
- [ ] The user can see the process history by looking at plots.

### Feature 2: Process Simuator

**Description** Is the backend for the application.  Runs the process and communicates with the frontend.

**User Story** The user will not see this as it is the backend.

**Requirements**
- Should be similar to the Tenesee Eastman Fortran Sructure as much as possible.  See reference docs
- Written in cpp with pybind11 exposing an API to frontend.
- Uses GNU scientific library for ODE solving
- Uses Eigen for matricies

## Technical Requirements

### Performance
- Ideally we will be stepping through the simulation at 60fps.

### Security
- This is a proof of concept that will only run on my machine.  It will be pushed to github for others to use but security is their concern.

### Compatibility
- This will be compiled for the platform it is running on and will communcate to a react app in the browser.

### Dependencies
- Eigen
- GNU sceintific library
- CMake to build
- GoogleTest to test
- NodeJS

## Constraints

None at this stage

## Open Questions



1. There is nothign in out process for testing. We need to add testing and a testing engineer prompt.  This should be added to our workflow.  Preferably the local llm will write tests as the senior engineer insruction.

## References

The technical background is in docs/TankDynamics
Tenesse Eastman backgroud is in docs/Tennessee_Eastman_Process_Equations.md

---

## Notes for Architect

When creating the plan, please address:

1. [Specific concern or question]
2. [Another area needing attention]

Preferred technologies (if any):
- [Technology preferences]

Timeline considerations:
- [Any deadline or time constraints]
