# Python Packaging for C++ Extension Projects

This document explains Python packaging concepts relevant to the Tank Dynamics Simulator, which combines C++ code with Python bindings.

## The Problem We're Solving

You have a C++ simulation library that you want to use from Python. This means:

1. C++ code must be compiled into a shared library (`.so` on Linux)
2. That library must be placed where Python can find it
3. Python must know how to load and call the C++ functions

Python's packaging system handles all of this, but the terminology can be confusing.

## Key Concepts

### Source Distribution vs. Binary Distribution

**Source Distribution (sdist):**
- A tarball (`.tar.gz`) containing your source code
- When someone installs it, they must compile everything on their machine
- Requires the recipient to have: compiler, CMake, GSL, Python headers, etc.
- Portable across platforms, but slow to install

**Binary Distribution (wheel):**
- A `.whl` file containing pre-compiled code
- Installs instantly—just unzips files into the right places
- Platform-specific: a wheel built on Ubuntu won't work on macOS
- No compiler needed by the recipient

### What is a Wheel?

A wheel is just a ZIP file with a `.whl` extension and a specific naming convention:

```
tank_sim-0.1.0-cp311-cp311-linux_x86_64.whl
   │       │     │     │        │
   │       │     │     │        └── Platform (Linux, 64-bit)
   │       │     │     └── Python ABI (CPython 3.11)
   │       │     └── Python version (CPython 3.11)
   │       └── Package version
   └── Package name
```

If you unzip it, you'll find:
```
tank_sim/
    __init__.py
    _tank_sim.cpython-311-x86_64-linux-gnu.so   # The compiled C++ code
tank_sim-0.1.0.dist-info/
    METADATA
    WHEEL
    RECORD
```

The `.so` file is your C++ code compiled into a Python extension module.

### Why Wheels Matter for Your Project

**For development (your laptop):**
- You don't need wheels—`pip install -e .` compiles on the fly
- Editable installs let you modify code without reinstalling

**For your VPS:**
- Option A: Install from source (`pip install .`)—compiles on the VPS
- Option B: Build a wheel locally, copy it, install the wheel

Option A is simpler for a single VPS. Option B matters when:
- You're distributing to others who may not have compilers
- You want faster deployments (no compilation step)
- You're publishing to PyPI

**For this project:** Just use `pip install .` on your VPS. Building wheels is unnecessary complexity for a single-deployment portfolio project.

## The Modern Python Packaging Stack

### pyproject.toml

The single configuration file for Python projects (replaced setup.py, setup.cfg, requirements.txt for many use cases):

```toml
[build-system]
requires = ["scikit-build-core", "pybind11"]  # Build-time dependencies
build-backend = "scikit_build_core.build"      # What builds the package

[project]
name = "tank-sim"
version = "0.1.0"
dependencies = ["numpy"]                        # Runtime dependencies

[project.optional-dependencies]
dev = ["pytest"]                                # pip install ".[dev]"
api = ["fastapi", "uvicorn"]                    # pip install ".[api]"
```

**Key insight:** `[build-system]` dependencies are only needed during installation. Users of your wheel never need scikit-build-core or pybind11—those are baked into the compiled `.so` file.

### Build Backends

The build backend is the tool that actually creates packages. Common options:

| Backend | Use Case |
|---------|----------|
| `setuptools` | Pure Python packages (traditional) |
| `hatchling` | Pure Python packages (modern) |
| `scikit-build-core` | CMake-based C++ extensions |
| `meson-python` | Meson-based C++ extensions |
| `maturin` | Rust extensions |

We use `scikit-build-core` because:
- Your project already has a working CMakeLists.txt
- It automatically finds and runs CMake
- It handles pybind11 integration
- It places the compiled `.so` in the right location

### Package Manager: uv

`uv` is a fast, modern Python package manager written in Rust. It replaces pip, pip-tools, and virtualenv with a single tool.

**Why uv:**
- 10-100x faster than pip
- Built-in virtual environment management
- Reproducible installs
- Drop-in replacement for pip commands

**Build tools** (create packages):
- `scikit-build-core` - Build backend for CMake projects (used automatically)

**What happens when you run `uv pip install .`:**
```
uv pip install .
    │
    └── Calls build backend (scikit-build-core)
            │
            └── Calls CMake
                    │
                    └── Compiles C++ code
```

## Virtual Environments

A virtual environment is an isolated Python installation. Each venv has its own:
- `site-packages/` directory (installed packages)
- `pip`/`uv` installation
- Python interpreter symlink

**Why use them:**
- Project A needs numpy 1.20, Project B needs numpy 2.0—no conflict
- You can delete and recreate them easily
- Your system Python stays clean

**Creating with uv:**
```bash
uv venv              # Creates .venv/
source .venv/bin/activate
uv pip install .     # Installs into .venv/
```

## Editable Installs

```bash
uv pip install -e ".[dev]"
#              ^^
#              Editable flag
```

An editable install creates a link from the venv to your source code instead of copying files. This means:

- Changes to Python files take effect immediately (no reinstall)
- Changes to C++ files require rebuilding: `uv pip install -e .`

**The `-e` workflow:**
1. Edit `tank_sim/__init__.py` → Changes work immediately
2. Edit `bindings/bindings.cpp` → Must run `uv pip install -e .` again

**Note:** uv also supports `uv sync` with a lockfile for even more reproducible installs, but `uv pip install` is sufficient for this project.

## Dependency Groups

```toml
[project]
dependencies = ["numpy"]           # Always installed

[project.optional-dependencies]
dev = ["pytest", "pytest-cov"]     # pip install ".[dev]"
api = ["fastapi", "uvicorn"]       # pip install ".[api]"
```

**Usage:**
```bash
# Development: get pytest
uv pip install -e ".[dev]"

# VPS deployment: get FastAPI
uv pip install ".[api]"

# Both:
uv pip install ".[dev,api]"
```

This keeps your production deployment lean—no pytest on the VPS.

## What Happens When You Run `uv pip install .`

Step by step:

1. **uv reads pyproject.toml** and sees the build backend is `scikit_build_core`

2. **uv installs build dependencies** (`scikit-build-core`, `pybind11`) in a temporary environment

3. **scikit-build-core runs CMake:**
   ```
   cmake -B _skbuild/build -S .
   cmake --build _skbuild/build
   ```

4. **CMake compiles everything:**
   - Your C++ library (`tank_sim_core`)
   - pybind11 bindings (`_tank_sim.so`)

5. **scikit-build-core collects files:**
   - `tank_sim/__init__.py`
   - `_tank_sim.cpython-311-x86_64-linux-gnu.so`

6. **uv installs into site-packages:**
   ```
   .venv/lib/python3.11/site-packages/
       tank_sim/
           __init__.py
           _tank_sim.cpython-311-x86_64-linux-gnu.so
   ```

7. **Cleanup:** Temporary build environment is deleted

Now `import tank_sim` works!

## For Your VPS Deployment

Given that this is a portfolio project deployed to a single VPS:

**Recommended approach:**
```bash
# On VPS
ssh your-vps
cd /path/to/tank_dynamics
git pull
uv pip install ".[api]"
# Restart your FastAPI server
```

**You don't need to:**
- Build wheels locally and transfer them
- Publish to PyPI
- Set up CI/CD to build wheels for multiple platforms

**You do need:**
- Build tools on the VPS (cmake, g++, libgsl-dev)
- The source code on the VPS

If compilation becomes slow or problematic, you can always add wheel building later. Start simple.

## Glossary

| Term | Meaning |
|------|---------|
| **wheel** | Pre-compiled package (`.whl` file) |
| **sdist** | Source distribution (`.tar.gz` of source code) |
| **build backend** | Tool that creates packages (scikit-build-core) |
| **editable install** | `uv pip install -e .`—links rather than copies |
| **venv** | Isolated Python environment |
| **site-packages** | Directory where packages are installed |
| **extension module** | Compiled code (`.so`/`.pyd`) callable from Python |
| **pyproject.toml** | Modern Python project configuration file |
| **uv** | Fast Python package manager (replaces pip) |

## Further Reading

- [Python Packaging User Guide](https://packaging.python.org/)
- [scikit-build-core documentation](https://scikit-build-core.readthedocs.io/)
- [pybind11 documentation](https://pybind11.readthedocs.io/)
- [uv documentation](https://docs.astral.sh/uv/)
