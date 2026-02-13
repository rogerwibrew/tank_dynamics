#!/bin/bash

# dev.sh - Start backend and frontend dev servers
#
# Usage: ./scripts/dev.sh
#
# Starts:
#   - Backend:  uvicorn on port 8000 (with hot reload scoped to api/ and tank_sim/)
#   - Frontend: Next.js dev server on port 3000
#
# Press Ctrl+C to stop both servers.
#
# Lessons encoded in this script:
#   1. Use .venv/bin/uvicorn directly, NOT "uv run". uv run triggers a full
#      C++ rebuild of the tank_sim extension every time (~60s). The extension
#      is already installed in the venv via "uv pip install -e ."
#   2. Scope uvicorn --reload-dir to api/ and tank_sim/ only. Without this,
#      uvicorn watches the entire project including frontend/.next/, which
#      has thousands of rapidly-changing files. The two file watchers (uvicorn
#      and Next.js) conflict and cause the frontend to hang on refresh.
#   3. Clear the entire .next/ cache before starting. Turbopack's persistent
#      cache can become corrupted (causing panics), and stale lock files from
#      crashed dev servers prevent Next.js from starting.

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Resolve project root (parent of scripts/)
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
FRONTEND_DIR="$PROJECT_ROOT/frontend"
VENV_BIN="$PROJECT_ROOT/.venv/bin"

# --- Preflight checks ---

if [ ! -f "$VENV_BIN/uvicorn" ]; then
    echo -e "${RED}Error: uvicorn not found in .venv/bin/${NC}"
    echo "Run: uv sync --extra api --extra dev"
    exit 1
fi

if [ ! -d "$FRONTEND_DIR/node_modules" ]; then
    echo -e "${RED}Error: frontend/node_modules not found${NC}"
    echo "Run: cd frontend && npm install"
    exit 1
fi

# Check that tank_sim C++ extension is installed
if ! "$VENV_BIN/python" -c "import tank_sim" 2>/dev/null; then
    echo -e "${YELLOW}Warning: tank_sim not installed. Building (this takes ~60s)...${NC}"
    cd "$PROJECT_ROOT" && uv pip install -e .
fi

# --- Cleanup ---

# Kill any existing servers on our ports and wait for release.
# Uses ss (not lsof) because lsof misses processes listening on wildcard (*:port).
for port in 3000 8000; do
    pids=$(ss -tlnp "sport = :$port" 2>/dev/null | grep -oP 'pid=\K[0-9]+' | sort -u || true)
    if [ -n "$pids" ]; then
        echo -e "${YELLOW}Killing existing process(es) on port $port (PIDs: $pids)${NC}"
        echo "$pids" | xargs kill -9 2>/dev/null || true
        # Wait until the port is actually free (up to 5s)
        for i in $(seq 1 50); do
            if ! ss -tlnp "sport = :$port" 2>/dev/null | grep -q "pid="; then
                break
            fi
            sleep 0.1
        done
    fi
done

# Clear Next.js cache (Turbopack cache corruption causes panics on startup)
if [ -d "$FRONTEND_DIR/.next" ]; then
    echo -e "${YELLOW}Clearing Next.js cache...${NC}"
    rm -rf "$FRONTEND_DIR/.next"
fi

# --- Start servers ---

# Trap Ctrl+C to kill both background processes
cleanup() {
    echo ""
    echo -e "${YELLOW}Shutting down...${NC}"
    kill $BACKEND_PID $FRONTEND_PID 2>/dev/null || true
    wait $BACKEND_PID $FRONTEND_PID 2>/dev/null || true
    echo -e "${GREEN}Done.${NC}"
    exit 0
}
trap cleanup SIGINT SIGTERM

# Start backend
echo -e "${GREEN}Starting backend on http://localhost:8000${NC}"
cd "$PROJECT_ROOT"
"$VENV_BIN/uvicorn" api.main:app \
    --reload \
    --reload-dir api \
    --reload-dir tank_sim \
    --host 0.0.0.0 \
    --port 8000 &
BACKEND_PID=$!

# Start frontend (explicit --port 3000 to fail fast if port is still busy)
echo -e "${GREEN}Starting frontend on http://localhost:3000${NC}"
cd "$FRONTEND_DIR"
npm run dev -- --port 3000 &
FRONTEND_PID=$!

# Wait for both
echo ""
echo -e "${GREEN}Both servers starting. Press Ctrl+C to stop.${NC}"
echo ""
wait $BACKEND_PID $FRONTEND_PID
