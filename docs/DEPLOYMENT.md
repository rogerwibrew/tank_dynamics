# Tank Dynamics API Deployment Guide

This guide provides step-by-step instructions for deploying the Tank Dynamics API in production environments.

## Prerequisites

### System Requirements

**Operating System:**
- Linux (Ubuntu 20.04+, Debian 10+, CentOS 7+, Arch Linux)
- macOS 10.14+
- Windows (WSL2) - untested but theoretically supported

**Python:**
- Python 3.10 or later

**System Packages:**

Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential libgsl-dev python3-dev git
```

Arch Linux:
```bash
sudo pacman -S cmake gsl base-devel python git
```

macOS:
```bash
brew install cmake gsl python@3.11 git
```

**C++ Compiler:**
- GCC 9+ or Clang 10+ (C++17 support required)
- MSVC 2019+ (Windows)

**Network:**
- Port 8000 (API server, configurable)
- Port 443 (HTTPS, if using TLS)
- Firewall rules to allow HTTP/WebSocket traffic

**Disk Space:**
- ~500 MB for dependencies and compilation
- ~50 MB for running application and history buffer

**Memory:**
- Minimum 512 MB
- Recommended 1+ GB for comfortable operation

### Dependency Installation

The C++ library requires external dependencies that will be automatically downloaded during build:
- Eigen (linear algebra)
- GSL (numerical integration)
- pybind11 (Python bindings)

These are fetched automatically by CMake if not found on the system.

---

## Installation

### Step 1: Clone Repository

```bash
git clone https://github.com/your-org/tank_dynamics.git
cd tank_dynamics
```

### Step 2: Build C++ Simulation Library

```bash
# Configure CMake (downloads Eigen, GSL, pybind11 automatically)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# Compile library and bindings
cmake --build build --config Release

# Verify C++ tests pass (optional but recommended)
ctest --test-dir build --output-on-failure
```

**Troubleshooting build errors:**

If CMake fails to download dependencies:
```bash
# Clear cache and retry
rm -rf build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
```

If GSL is not found:
```bash
# Try specifying GSL location
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DGSL_ROOT_DIR=/usr/local
```

### Step 3: Install Python Package and Dependencies

```bash
# Install the tank_sim Python bindings
pip install -e .

# Install API dependencies
pip install -r api/requirements.txt
```

If using `uv` (recommended, faster):
```bash
uv pip install -e .
uv sync --extra dev
```

### Step 4: Verify Installation

```bash
# Test Python bindings
python -c "import tank_sim; print('tank_sim module loaded successfully')"

# Test API imports
python -c "from api.main import app; print('FastAPI app loaded successfully')"

# Run Python tests (optional)
pytest api/tests/ -v
```

---

## Configuration

### Environment Variables

Create a `.env` file in the project root:

```bash
# API Server Configuration
API_HOST=0.0.0.0
API_PORT=8000
API_WORKERS=1

# Logging
LOG_LEVEL=INFO
# LOG_LEVEL options: DEBUG, INFO, WARNING, ERROR, CRITICAL

# CORS Configuration (comma-separated origins)
CORS_ORIGINS=https://example.com,https://www.example.com

# TLS/SSL (if using HTTPS)
TLS_CERT_PATH=/etc/tank-sim/cert.pem
TLS_KEY_PATH=/etc/tank-sim/key.pem
```

**Important:** In production, always use `API_WORKERS=1` to ensure a single simulation instance.

### CORS Configuration

For production, update `api/main.py` to restrict CORS to trusted origins:

```python
# In api/main.py, modify the CORSMiddleware configuration:
app.add_middleware(
    CORSMiddleware,
    allow_origins=[
        "https://your-frontend-domain.com",
        "https://www.your-frontend-domain.com",
    ],
    allow_credentials=True,
    allow_methods=["GET", "POST"],
    allow_headers=["*"],
)
```

Or use environment variable:

```python
import os

allowed_origins = os.getenv(
    "CORS_ORIGINS",
    "http://localhost:3000,http://localhost:5173"
).split(",")

app.add_middleware(
    CORSMiddleware,
    allow_origins=allowed_origins,
    allow_credentials=True,
    allow_methods=["GET", "POST"],
    allow_headers=["*"],
)
```

### Logging Configuration

Logging is configured in `api/main.py`. By default, logs go to stdout. For production, configure log rotation:

```python
import logging
from logging.handlers import RotatingFileHandler

# In api/main.py, add file handler:
handler = RotatingFileHandler(
    '/var/log/tank-sim-api.log',
    maxBytes=10485760,  # 10 MB
    backupCount=5
)
handler.setFormatter(logging.Formatter(
    "%(asctime)s - %(name)s - %(levelname)s - %(message)s"
))
logger.addHandler(handler)
```

---

## Running in Development

For local development and testing:

```bash
# From project root
python -m uvicorn api.main:app --reload --host 0.0.0.0 --port 8000
```

**Flags:**
- `--reload`: Auto-restart server when files change (development only, disable in production)
- `--host 0.0.0.0`: Listen on all interfaces (or `127.0.0.1` for localhost only)
- `--port 8000`: Port to listen on

The API is now accessible at `http://localhost:8000`. Access Swagger UI at `http://localhost:8000/docs`.

---

## Running in Production

### Direct Execution (Simple Deployments)

For production with a single machine:

```bash
# WITHOUT --reload flag
uvicorn api.main:app \
  --host 0.0.0.0 \
  --port 8000 \
  --workers 1
```

**Critical:** Always use `--workers 1` to ensure a single simulation instance. Multiple workers will cause state divergence.

### Systemd Service (Recommended)

Create `/etc/systemd/system/tank-sim-api.service`:

```ini
[Unit]
Description=Tank Dynamics Simulator API
Documentation=file:///path/to/tank_dynamics/docs/DEPLOYMENT.md
After=network.target

[Service]
Type=notify
User=tank-sim
Group=tank-sim
WorkingDirectory=/home/tank-sim/tank_dynamics
ExecStart=/usr/bin/python3 -m uvicorn api.main:app \
  --host 0.0.0.0 \
  --port 8000 \
  --workers 1 \
  --log-level info

# Restart on failure
Restart=on-failure
RestartSec=5s
StartLimitInterval=60s
StartLimitBurst=3

# Security
PrivateTmp=yes
NoNewPrivileges=yes

# Logging
StandardOutput=journal
StandardError=journal
SyslogIdentifier=tank-sim-api

[Install]
WantedBy=multi-user.target
```

**Setup:**

```bash
# Create service user and group
sudo useradd -r -s /bin/false tank-sim

# Set directory ownership
sudo chown -R tank-sim:tank-sim /path/to/tank_dynamics
sudo chmod -R 750 /path/to/tank_dynamics

# Enable and start service
sudo systemctl daemon-reload
sudo systemctl enable tank-sim-api
sudo systemctl start tank-sim-api

# Check status
sudo systemctl status tank-sim-api

# View logs
journalctl -u tank-sim-api -f
```

**Stopping/Restarting:**

```bash
sudo systemctl stop tank-sim-api
sudo systemctl restart tank-sim-api
```

### Docker Deployment (Optional)

For containerized deployments, create a `Dockerfile`:

```dockerfile
FROM python:3.11-slim

# Install system dependencies
RUN apt-get update && apt-get install -y \
    cmake build-essential libgsl-dev git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source
COPY . .

# Build C++ library
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --config Release

# Install Python package and dependencies
RUN pip install --no-cache-dir -e . && \
    pip install --no-cache-dir -r api/requirements.txt

# Expose port
EXPOSE 8000

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD python -c "import requests; requests.get('http://localhost:8000/api/health')"

# Run API
CMD ["uvicorn", "api.main:app", "--host", "0.0.0.0", "--port", "8000", "--workers", "1"]
```

Build and run:

```bash
docker build -t tank-sim-api .
docker run -p 8000:8000 tank-sim-api
```

---

## Reverse Proxy Setup

### Nginx (Recommended)

Create `/etc/nginx/sites-available/tank-sim`:

```nginx
upstream tank_sim_backend {
    server 127.0.0.1:8000;
}

server {
    listen 80;
    server_name api.example.com;
    
    # Redirect to HTTPS
    return 301 https://$server_name$request_uri;
}

server {
    listen 443 ssl http2;
    server_name api.example.com;
    
    # TLS certificates (use Let's Encrypt for free certs)
    ssl_certificate /etc/letsencrypt/live/api.example.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/api.example.com/privkey.pem;
    
    # TLS configuration
    ssl_protocols TLSv1.2 TLSv1.3;
    ssl_ciphers HIGH:!aNULL:!MD5;
    ssl_prefer_server_ciphers on;
    
    # Security headers
    add_header Strict-Transport-Security "max-age=31536000; includeSubDomains" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header X-Frame-Options "DENY" always;
    add_header X-XSS-Protection "1; mode=block" always;
    
    # Logging
    access_log /var/log/nginx/tank-sim-access.log;
    error_log /var/log/nginx/tank-sim-error.log;
    
    # Proxy REST endpoints
    location /api/ {
        proxy_pass http://tank_sim_backend;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        
        # Timeouts
        proxy_connect_timeout 60s;
        proxy_send_timeout 60s;
        proxy_read_timeout 60s;
    }
    
    # Proxy WebSocket endpoint
    location /ws {
        proxy_pass http://tank_sim_backend;
        
        # WebSocket upgrade headers (CRITICAL)
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        
        # Standard headers
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        
        # Keep-alive for long-lived connections
        proxy_read_timeout 86400;
        proxy_send_timeout 86400;
    }
    
    # Health check endpoint (used by load balancers)
    location /api/health {
        proxy_pass http://tank_sim_backend;
        access_log off;
    }
}
```

**Enable site:**

```bash
sudo ln -s /etc/nginx/sites-available/tank-sim /etc/nginx/sites-enabled/
sudo nginx -t  # Test configuration
sudo systemctl restart nginx
```

**Setup TLS with Let's Encrypt:**

```bash
sudo apt-get install certbot python3-certbot-nginx
sudo certbot certonly --nginx -d api.example.com
```

### Apache Configuration (Alternative)

```apache
<VirtualHost *:443>
    ServerName api.example.com
    
    # TLS
    SSLEngine on
    SSLCertificateFile /etc/letsencrypt/live/api.example.com/fullchain.pem
    SSLCertificateKeyFile /etc/letsencrypt/live/api.example.com/privkey.pem
    
    # Enable required modules
    # a2enmod proxy proxy_http proxy_wstunnel rewrite headers
    
    # Proxy WebSocket
    ProxyPreserveHost On
    ProxyPass /ws ws://127.0.0.1:8000/ws
    ProxyPassReverse /ws ws://127.0.0.1:8000/ws
    
    # Proxy REST
    ProxyPass /api http://127.0.0.1:8000/api
    ProxyPassReverse /api http://127.0.0.1:8000/api
    
    # Keep-alive for WebSocket
    ProxyTimeout 3600
    
    # Logging
    ErrorLog ${APACHE_LOG_DIR}/tank-sim-error.log
    CustomLog ${APACHE_LOG_DIR}/tank-sim-access.log combined
</VirtualHost>

<VirtualHost *:80>
    ServerName api.example.com
    Redirect permanent / https://api.example.com/
</VirtualHost>
```

---

## Monitoring

### Health Check Endpoint

Use the `/api/health` endpoint for monitoring:

```bash
# Manual check
curl http://localhost:8000/api/health

# Continuous monitoring
watch -n 5 curl http://localhost:8000/api/health
```

### System Logs

**With systemd:**

```bash
# View recent logs
journalctl -u tank-sim-api -n 50

# Follow logs in real-time
journalctl -u tank-sim-api -f

# View logs since last boot
journalctl -u tank-sim-api -b

# View specific time range
journalctl -u tank-sim-api --since "2 hours ago"
```

**Direct stdout (if running in foreground):**

All log messages are printed to console (also captured by systemd journal).

### Prometheus Metrics (Optional)

For production monitoring with Prometheus and Grafana, consider adding metrics middleware to `api/main.py`:

```python
from prometheus_client import Counter, Histogram, generate_latest
import time

request_count = Counter('tank_sim_requests_total', 'Total requests')
request_duration = Histogram('tank_sim_request_duration_seconds', 'Request duration')

@app.get("/metrics")
async def metrics():
    return generate_latest()
```

### API Monitoring Script

```bash
#!/bin/bash
# Simple health check script

API_URL="http://localhost:8000"
ALERT_EMAIL="admin@example.com"

while true; do
    if ! curl -f "$API_URL/api/health" > /dev/null 2>&1; then
        echo "API DOWN at $(date)" | mail -s "Tank Sim API Down" "$ALERT_EMAIL"
        systemctl restart tank-sim-api
    fi
    sleep 30
done
```

---

## Security Considerations

### Current Status

⚠️ **Important:** The current implementation has NO authentication. It is suitable only for:
- Trusted networks (e.g., internal company network)
- Local testing and development
- Machines behind a firewall

### For Public Deployment

**DO NOT expose this API to the internet without additional security measures.**

If you need public access, implement authentication:

#### Option 1: API Keys (Simple)

```python
# In api/main.py
from fastapi import Header, HTTPException

async def verify_api_key(x_api_key: str = Header(...)):
    valid_keys = os.getenv("API_KEYS", "").split(",")
    if x_api_key not in valid_keys:
        raise HTTPException(status_code=403, detail="Invalid API key")
    return x_api_key

# Apply to all endpoints
@app.get("/api/health", dependencies=[Depends(verify_api_key)])
async def health_check():
    ...
```

#### Option 2: OAuth2 (Recommended for Web Apps)

```python
from fastapi.security import OAuth2PasswordBearer, OAuth2PasswordRequestForm
from jose import JWTError, jwt

# See FastAPI documentation for full implementation
```

#### Option 3: Reverse Proxy Auth (Easiest with nginx)

```nginx
location /api/ {
    auth_basic "Tank Simulator API";
    auth_basic_user_file /etc/nginx/.htpasswd;
    
    proxy_pass http://tank_sim_backend;
}
```

Create password file:

```bash
sudo apt-get install apache2-utils
sudo htpasswd -c /etc/nginx/.htpasswd username
```

### Additional Security Measures

1. **Firewall:** Restrict access by IP/port
   ```bash
   sudo ufw allow 443/tcp
   sudo ufw allow 80/tcp
   sudo ufw deny 8000/tcp  # Block direct access to uvicorn
   ```

2. **HTTPS/TLS:** Always use in production
   - Use Let's Encrypt certificates (free)
   - Redirect HTTP → HTTPS

3. **Input Validation:** Already implemented via Pydantic models

4. **Rate Limiting:** Implement at reverse proxy level (nginx)
   ```nginx
   limit_req_zone $binary_remote_addr zone=api_limit:10m rate=10r/s;
   
   location /api/ {
       limit_req zone=api_limit burst=20;
   }
   ```

5. **CORS:** Restrict to known origins (see Configuration section)

6. **Logs:** Monitor for suspicious activity
   ```bash
   grep "error\|Error\|ERROR" /var/log/tank-sim-api.log
   ```

---

## Troubleshooting

### Port Already in Use

**Problem:** `Address already in use` error

**Solution:**

```bash
# Find process using port 8000
lsof -i :8000

# Kill the process
kill -9 <PID>

# Or use a different port
uvicorn api.main:app --port 8001
```

### WebSocket Connection Failures

**Problem:** WebSocket connections fail or are rejected

**Solutions:**

1. **Verify nginx WebSocket headers:**
   ```nginx
   proxy_http_version 1.1;
   proxy_set_header Upgrade $http_upgrade;
   proxy_set_header Connection "upgrade";
   ```

2. **Check firewall allows WebSocket:**
   ```bash
   sudo ufw allow 443/tcp
   ```

3. **Verify API server is running:**
   ```bash
   curl http://localhost:8000/api/health
   ```

### CORS Errors in Browser

**Problem:** Browser shows CORS error

**Solution:** Update CORS origins in `api/main.py` or `.env`:

```python
allowed_origins = os.getenv(
    "CORS_ORIGINS",
    "http://localhost:3000"
).split(",")

app.add_middleware(
    CORSMiddleware,
    allow_origins=allowed_origins,
    ...
)
```

### Module Import Errors

**Problem:** `ModuleNotFoundError: No module named 'tank_sim'`

**Solutions:**

1. **Rebuild C++ library:**
   ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```

2. **Reinstall Python package:**
   ```bash
   pip install -e .
   ```

3. **Check Python path:**
   ```bash
   python -c "import tank_sim; print(tank_sim.__file__)"
   ```

### Multiple Workers Issue

**Problem:** Simulation state diverges when using multiple workers

**Solution:** ALWAYS use exactly 1 worker:

```bash
# CORRECT
uvicorn api.main:app --workers 1

# WRONG (don't do this)
uvicorn api.main:app --workers 4
```

Multiple workers create separate simulator instances that don't share state.

### Service Won't Start

**Problem:** `systemctl start tank-sim-api` fails

**Solutions:**

1. **Check service status and errors:**
   ```bash
   systemctl status tank-sim-api
   journalctl -u tank-sim-api -n 20
   ```

2. **Verify working directory exists:**
   ```bash
   ls -la /home/tank-sim/tank_dynamics
   ```

3. **Check permissions:**
   ```bash
   sudo chown -R tank-sim:tank-sim /home/tank-sim/tank_dynamics
   ```

4. **Test manually:**
   ```bash
   sudo -u tank-sim python -m uvicorn api.main:app --host 0.0.0.0 --port 8000
   ```

---

## Performance Tuning

### CPU Optimization

The simulation is compute-bound. For better performance:

1. **Compile with optimizations:**
   ```bash
   cmake -B build -S . \
     -DCMAKE_BUILD_TYPE=Release \
     -DCMAKE_CXX_FLAGS="-O3 -march=native"
   ```

2. **Run on dedicated core (optional):**
   ```bash
   taskset -c 0 uvicorn api.main:app --host 0.0.0.0 --port 8000
   ```

### Memory Optimization

History buffer is fixed at 7200 entries (~2 hours). To reduce memory:

In `api/simulation.py`, change:
```python
self.history: deque = deque(maxlen=3600)  # 1 hour instead of 2
```

### Connection Limits

For many concurrent WebSocket clients, increase system limits:

```bash
# Increase file descriptors
ulimit -n 4096

# In systemd service:
[Service]
LimitNOFILE=4096
```

---

## Backup and Disaster Recovery

### Preserving Configuration

The API has no persistent state except the history buffer (in-memory). To preserve configuration:

1. **Document PID tuning parameters:**
   ```bash
   curl http://localhost:8000/api/config | jq '.pid_gains'
   ```

2. **Export before shutting down:**
   ```bash
   curl http://localhost:8000/api/history?duration=7200 > history_backup.json
   ```

### Restoring After Downtime

The API restarts with initial conditions. To resume testing:

1. **Restore tuning parameters:**
   ```bash
   curl -X POST http://localhost:8000/api/pid \
     -H "Content-Type: application/json" \
     -d '{"Kc": 1.0, "tau_I": 10.0, "tau_D": 5.0}'
   ```

2. **History is lost** (was in-memory). For persistence, implement database logging or export to file periodically.

---

## Upgrade Procedure

### From Development to Production

1. **Build on a staging system first**
2. **Run full test suite:**
   ```bash
   pytest api/tests/ -v
   ctest --test-dir build --output-on-failure
   ```

3. **Deploy to production:**
   ```bash
   # Backup old installation
   sudo systemctl stop tank-sim-api
   sudo cp -r /home/tank-sim/tank_dynamics /home/tank-sim/tank_dynamics.backup
   
   # Pull new version
   cd /home/tank-sim/tank_dynamics
   git pull origin main
   
   # Rebuild and install
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   pip install -e .
   pip install -r api/requirements.txt
   
   # Restart
   sudo systemctl start tank-sim-api
   ```

4. **Verify:**
   ```bash
   curl http://localhost:8000/api/health
   ```

---

## Support and Logs

### Collecting Debug Information

When reporting issues, include:

```bash
# System info
uname -a
python --version
cmake --version

# Build info
ls -la build/

# Python packages
pip list | grep -E "fastapi|uvicorn|pydantic"

# Service status
systemctl status tank-sim-api
journalctl -u tank-sim-api -n 100 > logs.txt

# API connectivity
curl -v http://localhost:8000/api/health
curl -v http://localhost:8000/api/config
```

Attach `logs.txt` to bug reports.

### Additional Resources

- **API Documentation:** `docs/API_REFERENCE.md`
- **Swagger UI:** `http://localhost:8000/docs`
- **Architecture Plan:** `docs/plan.md`
- **Source Code:** `api/` directory

---

## Checklist for Production Deployment

- [ ] C++ library builds without errors (`cmake --build build --config Release`)
- [ ] All tests pass (`pytest api/tests/ -v` and `ctest --test-dir build`)
- [ ] Environment variables configured in `.env` or systemd service
- [ ] CORS origins restricted to trusted domains
- [ ] Systemd service file created and enabled
- [ ] TLS/SSL certificates obtained (Let's Encrypt)
- [ ] Nginx reverse proxy configured with WebSocket support
- [ ] Firewall rules configured (allow 80, 443; deny 8000)
- [ ] Health check endpoint responds (`curl /api/health`)
- [ ] WebSocket connection works
- [ ] Logs configured and monitored
- [ ] Backup procedure documented
- [ ] Upgrade procedure tested on staging
- [ ] Security review completed (authentication, HTTPS, firewall)
- [ ] Monitoring/alerting configured
- [ ] Documentation updated with deployment details

---

## Additional Notes

### Why Single Worker?

The Tank Dynamics API maintains a single simulation instance in memory. With multiple uvicorn workers:
- Each worker gets its own simulator instance
- State diverges between workers
- Clients connected to different workers see different data
- WebSocket broadcasts from one worker don't reach clients on another

**Solution:** Always use `--workers 1`. The simulation loop is I/O-bound (1 Hz = plenty of time for broadcasting), so a single worker can handle many concurrent WebSocket clients.

### Historical Data Persistence

The current implementation stores history only in RAM. For persistent storage, consider:

1. **File-based export** (simple)
   - Periodically write history to JSON/CSV
   - Cron job: `curl .../api/history?duration=7200 >> history.jsonl`

2. **Database integration** (scalable)
   - PostgreSQL, InfluxDB, or TimescaleDB
   - Modify `SimulationManager.history` to write to DB

3. **Message queue** (distributed)
   - Kafka or Redis for distributed logging
   - Multiple services can consume state updates

### Scaling Beyond Single Machine

For multiple API instances with shared simulation:

- Consider distributed simulation (topic for another discussion)
- Or shared-state backend (Redis cache)
- Current architecture assumes single simulator
