# Tank Dynamics Documentation Index

This directory contains comprehensive documentation for the Tank Dynamics Simulator project. Use this index to find what you need.

## Quick Navigation

### 🚀 Getting Started (Start Here!)

1. **[Project Status](STATUS.md)** - What's been completed, current progress, and next steps
2. **[README.md](../README.md)** - Project overview, quick start, and features
3. **[DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)** - Development environment setup and contribution guidelines

### 📚 Understanding the Architecture

1. **[Implementation Plan](plan.md)** - Complete system architecture and technology decisions
2. **[Specifications](specs.md)** - Feature requirements and acceptance criteria
3. **[Tank Dynamics Theory](TankDynamics.md)** - Physics equations and control theory background

### 💻 Development Resources

- **[API Reference](API_REFERENCE.md)** - Complete C++ class documentation with examples
- **[Next Tasks](next.md)** - Current implementation phase and what needs to be done

### 📖 Detailed Class Specifications

For implementation details of each component, see:

- **[Model Class Specification](Model%20Class.md)** - Stateless tank physics model
- **[PID Controller Class Specification](PID%20Controller%20Class.md)** - Control with anti-windup
- **[Stepper Class Specification](Stepper%20Class.md)** - GSL RK4 integrator wrapper
- **[Simulator Class Specification](Simulator%20Class.md)** - Master orchestrator (planned)

### 🔧 Workflow and Process

- **[CLAUDE.md](../CLAUDE.md)** - AI-assisted hybrid workflow configuration
- **[Workflow Reference](workflow.md)** - General project workflow notes

---

## Documentation by Audience

### For Project Managers / Stakeholders

1. Start with **[README.md](../README.md)** for overview
2. Check **[Project Status](STATUS.md)** for current progress
3. Review **[Implementation Plan](plan.md)** for timeline and phases

### For Developers Starting New Work

1. Read **[Project Status](STATUS.md)** - understand what's done
2. Read **[DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)** - set up environment
3. Check **[Next Tasks](next.md)** - see what needs implementation
4. Review relevant class specification (e.g., `Stepper Class.md`)
5. Use **[API Reference](API_REFERENCE.md)** for existing APIs

### For Code Reviewers

1. Check **[Project Status](STATUS.md)** - understand context
2. Review **[Implementation Plan](plan.md)** - understand design
3. Use **[API Reference](API_REFERENCE.md)** - verify interfaces
4. Check test files in `../tests/` for acceptance criteria

### For Understanding Process Dynamics

1. Read **[Tank Dynamics Theory](TankDynamics.md)** - physics background
2. Review **[Specifications](specs.md)** - feature requirements
3. Check **[Implementation Plan](plan.md)** - design decisions section

### For Contributing

1. Read **[CLAUDE.md](../CLAUDE.md)** - understand workflow
2. Read **[DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)** - development process
3. Check **[Next Tasks](next.md)** - available tasks
4. Follow the git workflow in DEVELOPER_GUIDE

---

## Document Descriptions

| Document | Purpose | Audience | Status |
|----------|---------|----------|--------|
| [README.md](../README.md) | Project overview and quick start | Everyone | ✅ Current |
| [STATUS.md](STATUS.md) | Detailed progress and completed work | Managers, Developers | ✅ Current |
| [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) | Setup, building, testing, workflow | Developers | ✅ New (2026-02-02) |
| [API_REFERENCE.md](API_REFERENCE.md) | Complete C++ API documentation | Developers | ✅ New (2026-02-02) |
| [plan.md](plan.md) | Architecture, design, and phases | Architects, Tech Leads | ✅ Current |
| [specs.md](specs.md) | Feature requirements | Product, Developers | ✅ Current |
| [next.md](next.md) | Implementation tasks for current phase | Developers | ✅ Current |
| [TankDynamics.md](TankDynamics.md) | Process physics and theory | Engineers, Scientists | ✅ Current |
| [Model Class.md](Model%20Class.md) | TankModel detailed spec | Developers | ✅ Current |
| [PID Controller Class.md](PID%20Controller%20Class.md) | PIDController detailed spec | Developers | ✅ Current |
| [Stepper Class.md](Stepper%20Class.md) | Stepper detailed spec | Developers | ✅ Current |
| [Simulator Class.md](Simulator%20Class.md) | Simulator detailed spec | Developers | ✅ Current |
| [CLAUDE.md](../CLAUDE.md) | AI workflow configuration | AI Agents, Tech Leads | ✅ Current |

---

## Current Project Phase

**Phase:** 1 - C++ Simulation Core  
**Progress:** 50% Complete (17/34 tasks)  
**Status:** On Track

### Completed Components

- ✅ Build system (CMake)
- ✅ TankModel physics class
- ✅ PIDController control class
- ✅ Comprehensive unit tests (17 passing)

### In Progress

- 🔄 Stepper GSL wrapper

### Planned

- ⏳ Simulator orchestrator
- ⏳ Simulator tests
- ⏳ Python bindings
- ⏳ FastAPI backend
- ⏳ Next.js frontend

For detailed progress, see [STATUS.md](STATUS.md).

---

## Key Design Decisions

The project follows several architectural principles:

1. **Component Separation:** Physics (Model), Control (PIDController), Integration (Stepper), Orchestration (Simulator)
2. **Stateless Physics:** TankModel computes derivatives without maintaining state
3. **Standard Methods:** RK4 integration via GSL, linear algebra via Eigen
4. **Tennessee Eastman Pattern:** Separation between derivative computation and integration
5. **Clean API:** Simple interfaces suitable for Python binding and web exposure

See [plan.md](plan.md) for complete architecture details.

---

## Getting Help

**Technical Questions:**
- Check the relevant class specification (e.g., `Stepper Class.md`)
- Review examples in [API_REFERENCE.md](API_REFERENCE.md)
- Look at existing tests in `../tests/`

**Development Process Questions:**
- See [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)
- Check [CLAUDE.md](../CLAUDE.md) for workflow

**Architecture/Design Questions:**
- Review [plan.md](plan.md)
- Check [TankDynamics.md](TankDynamics.md) for physics background

**Status/Progress Questions:**
- See [STATUS.md](STATUS.md)
- Check [next.md](next.md) for current tasks

---

## Building from Source

```bash
# Clone and enter directory
git clone <repo>
cd tank_dynamics

# Build (automatically downloads dependencies)
cmake -B build -S .
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure

# See DEVELOPER_GUIDE.md for more options
```

See [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) for detailed build instructions.

---

## Document Maintenance

**Last Updated:** 2026-02-02  
**Maintainer:** Documentation Writer role  
**Update Frequency:** After each completed task

If you find incomplete, outdated, or confusing documentation:
1. Note the specific issue
2. Open an issue on GitHub or contact the maintainer
3. See contribution guidelines in [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)

---

## Quick Reference: File Organization

```
tank_dynamics/
├── README.md                           ← Start here
├── CLAUDE.md                           ← Workflow configuration
│
├── docs/
│   ├── README.md                       ← This file (documentation index)
│   ├── STATUS.md                       ← Current project status
│   ├── DEVELOPER_GUIDE.md              ← Development setup
│   ├── API_REFERENCE.md                ← C++ API documentation
│   ├── plan.md                         ← Architecture plan
│   ├── specs.md                        ← Feature specifications
│   ├── next.md                         ← Implementation tasks
│   ├── TankDynamics.md                 ← Physics theory
│   ├── Model Class.md                  ← TankModel spec
│   ├── PID Controller Class.md         ← PIDController spec
│   ├── Stepper Class.md                ← Stepper spec
│   └── Simulator Class.md              ← Simulator spec
│
├── src/                                ← C++ source code
│   ├── tank_model.h/cpp
│   ├── pid_controller.h/cpp
│   ├── stepper.h/cpp
│   └── simulator.h/cpp (planned)
│
├── tests/                              ← Unit tests
│   ├── test_tank_model.cpp             ✅ 7 tests
│   ├── test_pid_controller.cpp         ✅ 10 tests
│   ├── test_stepper.cpp                🔄 In progress
│   └── test_simulator.cpp              ⏳ Planned
│
├── bindings/                           ← Python bindings (planned)
├── api/                                ← FastAPI backend (planned)
└── frontend/                           ← Next.js frontend (planned)
```

---

**Navigation:** [Home](../README.md) | [Status](STATUS.md) | [Next Tasks](next.md) | [API Reference](API_REFERENCE.md)

For questions, refer to the appropriate document above or consult [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md).
