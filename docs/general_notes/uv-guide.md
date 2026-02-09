# UV: Python Package Manager Guide

This guide covers everything you need to know about using `uv` for the Tank Dynamics Simulator project.

## What is uv?

`uv` is an extremely fast Python package and project manager written in Rust by Astral (the creators of Ruff). It's designed as a unified replacement for multiple Python tools:

- **pip** - Package installation
- **pip-tools** - Dependency resolution and lock files
- **virtualenv** - Virtual environment creation
- **pyenv** (partially) - Python version management
- **twine** (partially) - Package publishing

### Why uv?

**Performance:**
- 10-100x faster than traditional tools
- Installs dependencies in seconds instead of minutes
- Built in Rust for speed and reliability

**Simplicity:**
- Single tool instead of multiple
- Automatic virtual environment management
- Built-in dependency locking
- Drop-in replacement for pip commands

**Reliability:**
- Universal lock file works across platforms and Python versions
- Global cache for instant reinstalls of common packages
- Better dependency resolution than pip

**Modern Features:**
- Python version management
- Workspace support for monorepos
- PEP 735 dependency groups
- Automatic environment creation

## How uv is Configured in This Project

### pyproject.toml Configuration

```toml
[build-system]
requires = ["scikit-build-core>=0.8", "pybind11>=2.11"]
build-backend = "scikit_build_core.build"

[project]
name = "tank-sim"
version = "0.1.0"
description = "Real-time tank dynamics simulator with PID control"
requires-python = ">=3.10"
dependencies = [
    "numpy>=1.20",
]

[project.optional-dependencies]
dev = [
    "pytest>=7.0",
    "pytest-cov>=4.0",
    "pybind11>=2.11",
]
api = [
    "fastapi>=0.100",
    "uvicorn[standard]>=0.20",
    "websockets>=11.0",
    "pydantic>=2.0",
]
```

**Key points:**
- **Build dependencies** (`[build-system]`): Only needed during installation to compile C++ code
- **Runtime dependencies** (`dependencies`): Always installed (just numpy in this case)
- **Optional dependencies** (`[project.optional-dependencies]`): Installed on demand
  - `dev`: Development tools (testing, coverage)
  - `api`: FastAPI server dependencies

### uv.lock File

The `uv.lock` file is automatically managed by uv and contains:
- Exact versions of all dependencies (including transitive ones)
- SHA256 hashes for integrity verification
- Resolution for all Python versions and platforms
- Source URLs for each package

**Important:** Never edit `uv.lock` manually. It's automatically updated when you run `uv add`, `uv remove`, or `uv lock`.

## Essential Commands

### Project Setup

```bash
# Create a new virtual environment
uv venv

# Activate the virtual environment (Linux/macOS)
source .venv/bin/activate

# Install project dependencies
uv sync

# Install with optional dependency groups
uv sync --extra dev
uv sync --extra api
uv sync --extra dev --extra api  # Install both
```

### Managing Dependencies

```bash
# Add a new dependency
uv add numpy
uv add "requests>=2.28"

# Add to optional groups
uv add --optional dev pytest
uv add --optional api fastapi

# Remove a dependency
uv remove numpy

# Upgrade all dependencies
uv lock --upgrade

# Upgrade specific package
uv lock --upgrade-package numpy
```

### Installing Packages

```bash
# Install project in editable mode (for development)
uv pip install -e .

# Install with dev dependencies
uv pip install -e ".[dev]"

# Install with api dependencies
uv pip install ".[api]"

# Install with multiple groups
uv pip install -e ".[dev,api]"

# Install from lock file (exact versions)
uv sync
```

### Running Commands

```bash
# Run a command in the project environment
uv run python script.py
uv run pytest
uv run python -m tank_sim

# Run with automatic environment activation
uv run python -c "import tank_sim; print(tank_sim.__version__)"
```

### Locking Dependencies

```bash
# Generate/update uv.lock file
uv lock

# Lock without updating existing versions
uv lock --locked

# Refresh specific package
uv lock --upgrade-package fastapi
```

### Working with Virtual Environments

```bash
# Create virtual environment
uv venv

# Create with specific Python version
uv venv --python 3.11
uv venv --python 3.12

# Remove virtual environment
rm -rf .venv

# Show environment location
uv python find
```

### Cache Management

```bash
# Show cache location
uv cache dir

# Clean entire cache
uv cache clean

# Clean specific package
uv cache clean numpy

# Show cache size
uv cache prune --dry-run
```

## Workflow Patterns for This Project

### Initial Setup (New Developer)

```bash
# Clone the repository
git clone <repo-url>
cd tank_dynamics

# Create virtual environment and install dependencies
uv venv
source .venv/bin/activate
uv pip install -e ".[dev]"

# Or use uv sync for locked versions
uv sync --extra dev
```

### Daily Development

```bash
# Activate environment
source .venv/bin/activate

# Make changes to Python code
# (changes are immediately available with -e install)

# Make changes to C++ code
# (requires rebuild)
uv pip install -e .

# Run tests
uv run pytest

# Run with coverage
uv run pytest --cov=tank_sim
```

### Adding New Dependencies

```bash
# For runtime dependencies
uv add package-name
# This automatically updates pyproject.toml and uv.lock

# For dev dependencies
uv add --optional dev package-name

# For API dependencies
uv add --optional api package-name

# Install the newly added dependency
uv sync
```

### VPS Deployment

```bash
# On the VPS
cd /path/to/tank_dynamics
git pull

# Install with API dependencies only (no dev tools)
uv pip install ".[api]"

# Or use locked versions for reproducibility
uv sync --extra api --frozen

# Restart your FastAPI service
```

### Running Tests

```bash
# Run all tests
uv run pytest

# Run with coverage
uv run pytest --cov=tank_sim --cov-report=html

# Run specific test file
uv run pytest tests/python/test_pid.py

# Run with verbose output
uv run pytest -v
```

## Virtual Environment Management

### How uv Handles Virtual Environments

uv automatically manages virtual environments in the `.venv` directory:

1. **Automatic Creation**: uv creates `.venv` when you run `uv venv`, `uv sync`, or `uv run` for the first time
2. **Automatic Discovery**: uv looks for `.venv` in the current directory or parent directories
3. **Fast Setup**: Virtual environments are created in seconds using a global package cache

### Virtual Environment Location

Default location: `.venv` in your project root

You can customize this with the `UV_PROJECT_ENVIRONMENT` environment variable:

```bash
export UV_PROJECT_ENVIRONMENT=/path/to/custom/venv
```

### Activating Environments

```bash
# Manual activation (Linux/macOS)
source .venv/bin/activate

# Or use uv run (no activation needed)
uv run python script.py
```

**Pro tip:** Use `uv run` to avoid manual activation. It automatically uses the project environment.

### Managing Multiple Environments

```bash
# Create environment with specific Python version
uv venv --python 3.10
uv venv --python 3.11

# List available Python versions
uv python list

# Install a specific Python version
uv python install 3.12
```

## Understanding uv.lock

The `uv.lock` file ensures reproducible installations across all environments.

### Key Features

1. **Platform-Independent**: Contains resolutions for Linux, macOS, Windows
2. **Python-Version Independent**: Includes markers for different Python versions
3. **Complete Dependency Graph**: Lists all transitive dependencies
4. **Integrity Verification**: Includes SHA256 hashes for security

### Lock File Behavior

```bash
# Generate lock file
uv lock

# Install from lock file (exact versions)
uv sync

# Update lock file when pyproject.toml changes
uv lock

# Prevent any updates (CI/CD)
uv sync --frozen
```

**Important:** Commit `uv.lock` to version control for reproducible builds.

### When Lock File is Updated

uv automatically updates `uv.lock` when you:
- Run `uv add` or `uv remove`
- Run `uv lock`
- Run `uv sync` and dependencies have changed

## Comparison with pip and Poetry

### uv vs pip

| Feature | uv | pip |
|---------|----|----|
| **Speed** | 10-100x faster | Baseline |
| **Lock files** | Built-in (`uv.lock`) | Requires pip-tools |
| **Virtual envs** | Automatic | Manual with venv |
| **Dependency resolution** | Advanced (like Poetry) | Basic |
| **Python version mgmt** | Built-in | Requires pyenv |
| **Global cache** | Yes | Limited |

**Migration from pip:**
```bash
# pip commands
pip install requests
pip install -r requirements.txt
pip freeze > requirements.txt

# uv equivalents
uv add requests
uv sync
uv pip freeze > requirements.txt  # Not recommended, use uv.lock instead
```

### uv vs Poetry

| Feature | uv | Poetry |
|---------|----|----|
| **Speed** | Extremely fast | Moderate |
| **Maturity** | Newer (2023+) | Mature (2018+) |
| **Lock format** | Universal | Platform-specific |
| **Plugin ecosystem** | Growing | Established |
| **Publishing to PyPI** | Basic | Full-featured |
| **Workspace support** | Yes | Yes |

**When to choose uv:**
- Speed is critical (CI/CD, large projects)
- You want a single tool for everything
- You prefer modern tooling

**When to choose Poetry:**
- You need extensive plugin support
- Complex publishing workflows
- Team is already using Poetry

### Using uv with This Project

This project uses uv because:
1. **Fast rebuilds**: C++ compilation is slow; fast dependency resolution helps
2. **Simple tooling**: One tool instead of pip + venv + pip-tools
3. **Modern approach**: Follows Python packaging best practices
4. **Compatible with scikit-build-core**: Works seamlessly with CMake builds

## Dependency Groups and Optional Dependencies

### Optional Dependencies (Extras)

Defined in `[project.optional-dependencies]`:

```toml
[project.optional-dependencies]
dev = ["pytest>=7.0", "pytest-cov>=4.0"]
api = ["fastapi>=0.100", "uvicorn[standard]>=0.20"]
```

**Usage:**
```bash
# Install with extras
uv sync --extra dev
uv sync --extra api
uv pip install -e ".[dev,api]"
```

**When to use:**
- Public packages (published to PyPI)
- Optional features users can choose
- Different deployment configurations

### Dependency Groups (PEP 735)

Defined in `[dependency-groups]` (not used in this project yet):

```toml
[dependency-groups]
test = ["pytest>=7.0", "pytest-cov>=4.0"]
docs = ["sphinx>=4.0", "sphinx-rtd-theme"]
```

**Usage:**
```bash
uv sync --group test
uv sync --group docs
uv sync --all-groups
```

**Difference from extras:**
- Not exposed when package is installed as a dependency
- Local development only
- More flexible than extras

### Current Project Structure

This project uses **optional dependencies** (extras) instead of dependency groups:

```bash
# Development work
uv sync --extra dev

# VPS deployment
uv sync --extra api

# Full installation
uv sync --extra dev --extra api
```

## Working with C++ Extensions

This project combines Python with C++ code using pybind11 and CMake.

### Development Workflow

```bash
# Initial setup
uv venv
uv pip install -e ".[dev]"

# Edit Python files
# Changes are immediately available (editable install)

# Edit C++ files (src/*.cpp, bindings/*.cpp)
# Requires rebuild:
uv pip install -e .

# Run tests
uv run pytest
```

### Build Process

When you run `uv pip install -e .`:

1. uv reads `pyproject.toml`
2. Installs build dependencies (`scikit-build-core`, `pybind11`)
3. scikit-build-core invokes CMake
4. CMake compiles C++ code
5. Compiled extension (`.so` file) is placed in Python package
6. Package is installed in editable mode

### Editable Installs with C++

```bash
# Install in editable mode
uv pip install -e .

# Python changes: No rebuild needed
# C++ changes: Run uv pip install -e . again
```

**Why `-e` matters:**
- Without `-e`: Files are copied; must reinstall for any change
- With `-e`: Python files are linked; only C++ changes need rebuild

## Best Practices for This Project

### 1. Always Use the Lock File

```bash
# ✓ Good: Reproducible installs
uv sync --extra dev

# ✗ Avoid: Ignores lock file
uv pip install -e ".[dev]"
```

**Exception:** During development, `uv pip install -e .` is fine for quick iterations.

### 2. Commit uv.lock to Git

```bash
git add uv.lock pyproject.toml
git commit -m "Update dependencies"
```

This ensures all developers and deployments use identical versions.

### 3. Update Dependencies Regularly

```bash
# Check for updates
uv lock --upgrade --dry-run

# Update all dependencies
uv lock --upgrade

# Update specific package
uv lock --upgrade-package fastapi
```

### 4. Use uv run for Scripts

```bash
# ✓ Good: Automatic environment
uv run pytest
uv run python scripts/analyze.py

# ✗ Avoid: Manual activation
source .venv/bin/activate
pytest
```

### 5. Separate Dev and Production Dependencies

```bash
# Development
uv sync --extra dev

# Production (VPS)
uv sync --extra api --frozen
```

The `--frozen` flag prevents dependency updates in production.

### 6. Leverage the Global Cache

uv caches packages globally, making reinstalls instant:

```bash
# First install: Downloads packages
uv venv && uv sync

# Delete environment
rm -rf .venv

# Second install: Uses cache (instant)
uv venv && uv sync
```

### 7. Use Specific Python Versions

```bash
# Specify Python version in pyproject.toml
requires-python = ">=3.10"

# Create environment with specific version
uv venv --python 3.11
```

## Troubleshooting Common Issues

### Build Failures

**Problem:** C++ compilation fails

**Solutions:**
```bash
# Install build dependencies
sudo apt-get install cmake g++ libgsl-dev  # Ubuntu/Debian
sudo pacman -S cmake gcc gsl  # Arch Linux

# Check build backend logs
uv pip install -e . --verbose

# Try with no cache
uv pip install -e . --no-cache
```

### Cache Corruption

**Problem:** Errors like "Failed to deserialize cache entry"

**Solutions:**
```bash
# Clear entire cache
uv cache clean

# Clear specific package
uv cache clean numpy

# Force refresh
uv sync --refresh
uv pip install --refresh -e .
```

### Missing Dependencies

**Problem:** `ImportError` when running code

**Solutions:**
```bash
# Verify installation
uv run python -c "import tank_sim; print(tank_sim.__version__)"

# Check which packages are installed
uv pip list

# Reinstall with correct extras
uv sync --extra dev --extra api
```

### Wrong Python Version

**Problem:** Project requires Python 3.10+ but using 3.9

**Solutions:**
```bash
# Install correct Python version
uv python install 3.11

# Recreate environment with correct version
rm -rf .venv
uv venv --python 3.11
uv sync
```

### Lock File Conflicts

**Problem:** `uv.lock` has merge conflicts after git pull

**Solutions:**
```bash
# Accept their version
git checkout --theirs uv.lock

# Regenerate lock file
uv lock

# Or accept your version and regenerate
git checkout --ours uv.lock
uv lock
```

### Permission Errors

**Problem:** Permission denied when accessing cache

**Solutions:**
```bash
# Check cache location
uv cache dir

# Fix permissions
sudo chown -R $USER ~/.cache/uv

# Or use custom cache location
export UV_CACHE_DIR=~/.local/share/uv-cache
```

### Slow Network Issues

**Problem:** Package downloads timeout or fail

**Solutions:**
```bash
# Increase timeout
export UV_HTTP_TIMEOUT=300

# Retry failed downloads
uv sync --refresh

# Use a different index
export UV_INDEX_URL=https://pypi.org/simple
```

### Editable Install Not Working

**Problem:** Changes to code not reflected after editable install

**Solutions:**
```bash
# For Python changes: Should work immediately
# Verify editable install
uv pip show tank-sim

# For C++ changes: Rebuild required
uv pip install -e . --force-reinstall

# Check if module is using cached bytecode
python -B -c "import tank_sim"  # -B = don't write .pyc files
```

## Advanced Usage

### Custom Index URLs

```bash
# Use a private PyPI server
export UV_INDEX_URL=https://pypi.mycompany.com/simple

# Add extra index (in addition to PyPI)
export UV_EXTRA_INDEX_URL=https://pypi.mycompany.com/simple
```

### Offline Installation

```bash
# Build bundle with all dependencies
uv export --all-extras > requirements.txt
uv pip download -r requirements.txt -d ./packages

# Install offline
uv pip install --no-index --find-links ./packages -e .
```

### CI/CD Integration

```yaml
# GitHub Actions example
name: Test

on: [push]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      
      - name: Set up uv
        run: curl -LsSf https://astral.sh/uv/install.sh | sh
      
      - name: Install dependencies
        run: uv sync --extra dev --frozen
      
      - name: Run tests
        run: uv run pytest
```

### Custom Build Configuration

For this project's C++ extensions, you can pass CMake flags:

```bash
# Set CMake build type
export CMAKE_BUILD_TYPE=Debug
uv pip install -e .

# Pass additional CMake arguments
export CMAKE_ARGS="-DENABLE_TESTS=ON"
uv pip install -e .
```

## Migration Guide

### From pip + requirements.txt

```bash
# Old workflow
pip install -r requirements.txt
pip install -r requirements-dev.txt

# New workflow
uv sync --extra dev
```

**Convert requirements.txt to pyproject.toml:**
```bash
# Use uv to generate from existing requirements
uv add $(cat requirements.txt | grep -v '^#' | tr '\n' ' ')
```

### From Poetry

```bash
# Poetry uses poetry.lock and pyproject.toml
# uv can read the same pyproject.toml format

# Convert Poetry project
poetry export -f requirements.txt --output requirements.txt
uv add $(cat requirements.txt | cut -d= -f1)
rm poetry.lock requirements.txt

# Use uv going forward
uv sync
```

### From Conda

```bash
# Export conda environment
conda env export > environment.yml

# Extract pip packages (manual process)
# Add to pyproject.toml

# Use uv
uv sync
```

## Quick Reference

### Most Common Commands

```bash
# Project setup
uv venv                          # Create virtual environment
uv sync --extra dev              # Install all dependencies

# Development
uv pip install -e .              # Install project in editable mode
uv run pytest                    # Run tests
uv run python script.py          # Run script in environment

# Dependency management
uv add package                   # Add new dependency
uv add --optional dev package    # Add dev dependency
uv remove package                # Remove dependency
uv lock --upgrade                # Update all dependencies

# Environment management
source .venv/bin/activate        # Activate environment (manual)
uv run <command>                 # Run command (auto-activate)
rm -rf .venv && uv venv          # Reset environment

# Cache and troubleshooting
uv cache clean                   # Clear cache
uv sync --refresh                # Force refresh packages
uv pip install -e . --no-cache   # Install without cache
```

### File Checklist

- `pyproject.toml` - Project configuration (edit manually)
- `uv.lock` - Lock file (auto-generated, commit to git)
- `.venv/` - Virtual environment (in .gitignore)
- `.python-version` - Optional: specify Python version

### Environment Variables

```bash
UV_CACHE_DIR             # Cache location
UV_PROJECT_ENVIRONMENT   # Custom venv location
UV_INDEX_URL            # PyPI mirror
UV_HTTP_TIMEOUT         # Download timeout
UV_PYTHON              # Python version to use
```

## Resources

### Official Documentation
- [uv Documentation](https://docs.astral.sh/uv/) - Complete official docs
- [uv GitHub Repository](https://github.com/astral-sh/uv) - Source code and issues
- [Python Packaging Guide](https://packaging.python.org/) - Python packaging standards

### Tutorials and Guides
- [DataCamp: Python UV Tutorial](https://www.datacamp.com/tutorial/python-uv) - Comprehensive beginner guide
- [Real Python: Managing Projects with uv](https://realpython.com/python-uv/) - In-depth tutorial
- [SaaS Pegasus: uv Deep Dive](https://www.saaspegasus.com/guides/uv-deep-dive/) - Advanced guide
- [uv Cheat Sheet](https://mathspp.com/blog/uv-cheatsheet) - Quick reference

### Community Resources
- [uv Quick Reference](https://uv.pydevtools.com/) - Comprehensive command reference
- [Enhanced uv Cheat Sheet](https://ricketyrick.github.io/uv_cheat_sheet/uv-cheat-sheet.html) - Visual guide

### Related Project Documentation
- [Python Packaging for C++ Extensions](./python-packaging.md) - Project-specific packaging guide
- [scikit-build-core](https://scikit-build-core.readthedocs.io/) - Build backend documentation
- [pybind11](https://pybind11.readthedocs.io/) - C++ binding documentation

---

**Last Updated:** 2026-02-04  
**uv Version:** 0.6.x+  
**Project:** Tank Dynamics Simulator
