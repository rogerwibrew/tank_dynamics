#!/bin/bash

# Build script for TankDynamics project using Ninja
# Uses all available CPU cores for parallel compilation

set -e  # Exit on error

# Color output for better readability
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="build"
NUM_CORES=$(nproc)
CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Debug}"

# Parse command line arguments
CLEAN_BUILD=false
RUN_TESTS=false
RUN_SIMULATOR_VERIFY=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --test)
            RUN_TESTS=true
            shift
            ;;
        --verify)
            RUN_TESTS=true
            shift
            ;;
        --verify-simulator)
            RUN_SIMULATOR_VERIFY=true
            shift
            ;;
        --release)
            CMAKE_BUILD_TYPE="Release"
            shift
            ;;
        --debug)
            CMAKE_BUILD_TYPE="Debug"
            shift
            ;;
        -h|--help)
            echo "Usage: ./build.sh [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --clean              Clean build directory before building"
            echo "  --test               Run tests after building"
            echo "  --verify             Run verification programs after building"
            echo "  --verify-simulator   Run simulator verification after building"
            echo "  --debug              Build with debug symbols (default)"
            echo "  --release            Build with optimizations"
            echo "  -h, --help           Show this help message"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

echo -e "${YELLOW}=== TankDynamics Build Script ===${NC}"
echo "Build type: $CMAKE_BUILD_TYPE"
echo "Using $NUM_CORES cores"
echo ""

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

# Change to build directory
cd "$BUILD_DIR"

# Configure with CMake using Ninja
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake \
    -G Ninja \
    -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    ..

# Build using Ninja with all cores
echo -e "${YELLOW}Building with Ninja ($NUM_CORES cores)...${NC}"
ninja -j "$NUM_CORES"

# Check build status
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Build successful${NC}"
else
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi

# Create symlink to compile_commands.json in project root
if [ -f compile_commands.json ]; then
    ln -sf "$BUILD_DIR/compile_commands.json" ../compile_commands.json
    echo -e "${GREEN}✓ Created compile_commands.json symlink${NC}"
fi

# Run tests if requested
if [ "$RUN_TESTS" = true ]; then
    echo ""
    echo -e "${YELLOW}Running tests...${NC}"
    ctest --output-on-failure
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ All tests passed${NC}"
    else
        echo -e "${RED}✗ Some tests failed${NC}"
        exit 1
    fi
fi

# Run verification programs
echo ""
echo -e "${YELLOW}Running verification programs...${NC}"

# Stepper verification program
if [ -f "./bindings/stepper_verify" ]; then
    echo -e "${YELLOW}Running stepper_verify...${NC}"
    ./bindings/stepper_verify
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Stepper verification passed${NC}"
    else
        echo -e "${RED}✗ Stepper verification failed${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}⊙ stepper_verify not built${NC}"
fi

# Simulator verification program (if requested)
if [ "$RUN_SIMULATOR_VERIFY" = true ]; then
    if [ -f "./bindings/simulator_verify" ]; then
        echo -e "${YELLOW}Running simulator_verify...${NC}"
        ./bindings/simulator_verify
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ Simulator verification passed${NC}"
        else
            echo -e "${RED}✗ Simulator verification failed${NC}"
            exit 1
        fi
    else
        echo -e "${RED}✗ simulator_verify not built${NC}"
        exit 1
    fi
fi

echo ""
echo -e "${GREEN}Build directory: ${BUILD_DIR}${NC}"
echo -e "${GREEN}Build complete!${NC}"
