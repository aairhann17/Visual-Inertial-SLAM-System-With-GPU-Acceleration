# Visual-Inertial SLAM System With GPU Acceleration

[![CI](https://github.com/aairhann17/Visual-Inertial-SLAM-System-With-GPU-Acceleration/actions/workflows/ci.yml/badge.svg)](https://github.com/aairhann17/Visual-Inertial-SLAM-System-With-GPU-Acceleration/actions/workflows/ci.yml)

A modular C++17 Visual-Inertial SLAM/VIO project with optional CUDA acceleration, plus a full-stack orchestration layer (Express + PostgreSQL + React dashboard) for running, monitoring, and inspecting SLAM jobs.

![Scaffold demo](./docs/media/scaffold-demo.gif)

---

## Table of Contents

1. [What this project includes](#what-this-project-includes)
2. [Repository structure](#repository-structure)
3. [Core SLAM architecture](#core-slam-architecture)
4. [Full-stack architecture](#full-stack-architecture)
5. [Prerequisites](#prerequisites)
6. [Containerized one-command execution](#containerized-one-command-execution)
7. [Quick start (recommended)](#quick-start-recommended)
8. [Manual setup](#manual-setup)
9. [How to run jobs](#how-to-run-jobs)
10. [REST API reference](#rest-api-reference)
11. [ML confidence component](#ml-confidence-component)
12. [Performance benchmarks](#performance-benchmarks)
13. [Testing and validation](#testing-and-validation)
14. [Troubleshooting](#troubleshooting)
15. [Current limitations](#current-limitations)
16. [Roadmap](#roadmap)
17. [License](#license)

---

## What this project includes

### 1) Native SLAM runtime (C++)

- Feature detection/tracking
- Lock-free IMU ingestion ring buffer (SPSC) for deterministic sensor handoff
- IMU preintegration
- Triangulation and map management
- Backend optimization scaffolding
- Tracking health state machine with degraded/recovery transitions
- Structured logger abstraction with `spdlog` backend (fallback to stdout logger when unavailable)
- EuRoC runtime path in the current executable flow

Main executable:

- `build/Debug/vislam_app.exe`

### 2) Full-stack orchestration layer

- **Backend API**: [services/api/](./services/api/)
  - Launches `vislam_app`
  - Persists run state, logs, trajectory, and metrics
  - Streams run updates (SSE)
  - Validates EuRoC dataset path structure
- **Web dashboard**: [apps/web/](./apps/web/)
  - Create runs and monitor status
  - Live logs and run selection
  - Trajectory visualization
  - Confidence/risk metrics table
  - One-click scaffold test run for first-time setup checks
- **Database**: PostgreSQL via [docker-compose.yml](./docker-compose.yml)

---

## Repository structure

```text
.
├─ include/                  # C++ headers (frontend/backend/mapping/utils)
├─ src/                      # C++ implementation
├─ tests/                    # C++ tests
├─ gpu/                      # Optional CUDA components
├─ .github/workflows/        # CI pipeline
├─ scripts/
│  ├─ run-fullstack.ps1      # One-command full-stack bootstrap/start
│  ├─ evaluate_ate_rpe.py
│  ├─ plot_trajectory.py
│  ├─ benchmarks/
│     └─ benchmark_runtime.py
│  └─ media/
│     └─ generate_scaffold_demo_gif.py
├─ services/
│  └─ api/                   # Express API + PostgreSQL integration
├─ apps/
│  └─ web/                   # React dashboard (Vite)
├─ ml/
│  └─ models/                # Optional confidence-model weights example
├─ datasets/                 # Local datasets (not committed)
├─ Dockerfile                # API/runtime image (multi-stage, builds vislam_app)
├─ docker-compose.yml        # Full stack (postgres + api + web)
├─ docker-compose.gpu.yml    # GPU override for API service
├─ BENCHMARKS.md             # Latest benchmark summary
└─ docs/media/               # Visual demo assets for portfolio/readme
```

---

## Core SLAM architecture

High-level flow:

1. Camera + IMU input
2. Frontend feature detection/tracking
3. IMU preintegration
4. Triangulation + map/keyframe updates
5. Backend optimization
6. Trajectory output

The runtime currently supports:

- `--dataset euroc` path for EuRoC processing in the executable run flow
- `--dataset scaffold` for synthetic/scaffold smoke behavior

---

## Full-stack architecture

```text
React Dashboard (apps/web)
        │
        ▼
Express API (services/api)
  ├─ POST /api/runs                 -> spawn vislam_app
  ├─ GET  /api/runs/:id/stream      -> SSE run events
  ├─ GET  /api/runs/:id/trajectory  -> stored poses
  ├─ GET  /api/runs/:id/metrics     -> derived metrics + confidence
  └─ GET  /api/datasets/euroc/validate?path=...
        │
        ▼
PostgreSQL (runs, events, poses, metrics)
        │
        ▼
Artifacts (artifacts/runs/<runId>/trajectory.txt)
```

---

## Prerequisites

### Required

- Windows (current setup/scripts assume Windows PowerShell)
- Node.js 18+
- Docker Desktop
- CMake + MSVC toolchain
- Built native executable (`vislam_app.exe`)

### Optional

- CUDA toolkit for GPU targets
- ROS 2 (for lifecycle node target)

---

## Containerized one-command execution

From repository root:

```powershell
docker compose up --build
```

This starts:

- `postgres` (database)
- `api` (builds `vislam_app` in Docker and runs Express API)
- `web` (builds and serves React dashboard via Nginx)

Endpoints:

- Dashboard: `http://127.0.0.1:5173`
- API: `http://127.0.0.1:8080`

For CUDA-capable environments:

```powershell
docker compose -f docker-compose.yml -f docker-compose.gpu.yml up --build
```

> Note: GPU mode requires NVIDIA Container Toolkit and compatible host drivers.

---

## Quick start (recommended)

From repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run-fullstack.ps1 -InstallDeps
```

This script:

1. Verifies `build\Debug\vislam_app.exe` exists
2. Verifies Docker daemon is running
3. Starts PostgreSQL container
4. Creates missing `.env` files from examples
5. Installs API/Web dependencies (with `-InstallDeps`)
6. Initializes DB schema
7. Launches API + web dev servers in new terminals

URLs:

- API health: `http://127.0.0.1:8080/api/health`
- Dashboard: `http://127.0.0.1:5173`

Useful script flags:

- `-InstallDeps` force `npm install` in both apps
- `-NoServerLaunch` setup only (no servers started)

---

## Manual setup

### 1) Build native app

```powershell
cmake -S . -B build
cmake --build build --config Debug
```

Expected binary:

- `build\Debug\vislam_app.exe`

### 2) Start PostgreSQL

```powershell
docker compose up -d postgres
```

### 3) Backend setup

```powershell
cd services\api
copy .env.example .env
npm install
npm run db:init
npm run start
```

### 4) Frontend setup

```powershell
cd apps\web
copy .env.example .env
npm install
npm run dev -- --host 127.0.0.1 --port 5173
```

### 5) Runtime configuration reference

API environment file: `services/api/.env`

| Variable | Purpose | Default |
|---|---|---|
| `PORT` | API listen port | `8080` |
| `DATABASE_URL` | PostgreSQL connection string | `postgresql://vislam:vislam@localhost:5432/vislam` |
| `VISLAM_EXECUTABLE` | Path to native runtime executable | `build\Debug\vislam_app.exe` |
| `ARTIFACTS_DIR` | Run output storage root | `artifacts\runs` |
| `ENABLE_ML_CONFIDENCE` | Enable confidence/risk generation | `true` |
| `ML_CONFIDENCE_MODEL_PATH` | Optional JSON weights file for confidence scoring | unset |

### 6) ROS 2 lifecycle node (optional)

Enable and build the managed lifecycle node:

```powershell
cmake -S . -B build -DVISLAM_ENABLE_ROS2=ON
cmake --build build --config Debug --target vislam_ros2_lifecycle_node
```

If ROS 2 `rclcpp` / `rclcpp_lifecycle` packages are unavailable, CMake skips this target with a warning.

---

## How to run jobs

### A) First-time validation without datasets (recommended)

Use the dashboard button:

- **Use scaffold test run**

This validates API + DB + orchestration + UI wiring without EuRoC data.

### B) EuRoC run

1. Ensure your EuRoC path contains:
   - `mav0/cam0/data.csv`
   - `mav0/imu0/data.csv`
2. In dashboard, set:
   - `dataset = euroc`
   - `datasetPath = <path>`
3. Submit run.

The UI performs pre-submit EuRoC path validation and will block invalid paths.

---

## REST API reference

Base URL: `http://127.0.0.1:8080/api`

### Health

- `GET /health`

### Dataset validation

- `GET /datasets/euroc/validate?path=<datasetPath>`
  - Validates existence and expected EuRoC file layout
  - Resolves relative path from repository root
  - Attempts nested root detection up to two levels

### Run lifecycle

- `POST /runs`
  - Body:
    ```json
    {
      "dataset": "euroc",
      "datasetPath": "datasets/euroc/.../MH_01_easy",
      "maxFrames": 300
    }
    ```
- `GET /runs`
- `GET /runs/:id`
- `POST /runs/:id/cancel`

### Run outputs

- `GET /runs/:id/trajectory`
- `GET /runs/:id/metrics`
- `GET /runs/:id/logs`
- `GET /runs/:id/stream` (SSE)

---

## ML confidence component

The backend computes per-frame confidence/risk metadata from trajectory-derived metrics.

- Feature flag: `ENABLE_ML_CONFIDENCE=true`
- Optional model weights file: `ML_CONFIDENCE_MODEL_PATH`
- Fallback behavior: heuristic scoring when no model file is provided
- Risk classes:
  - `stable`
  - `warning`
  - `likely_drift`

Example model file:

- [ml/models/confidence-model.example.json](./ml/models/confidence-model.example.json)

---

## Performance benchmarks

Benchmark script:

- [scripts/benchmarks/benchmark_runtime.py](./scripts/benchmarks/benchmark_runtime.py)

Generate a benchmark report:

```powershell
python .\scripts\benchmarks\benchmark_runtime.py `
  --cpu-cmd ".\build\Debug\vislam_app.exe" `
  --iterations 100 `
  --markdown-output ".\BENCHMARKS.md"
```

Optional GPU comparison run:

```powershell
python .\scripts\benchmarks\benchmark_runtime.py `
  --cpu-cmd ".\build\Debug\vislam_app.exe" `
  --gpu-cmd ".\build-gpu\Debug\vislam_app.exe" `
  --iterations 100 `
  --markdown-output ".\BENCHMARKS.md"
```

Latest committed results:

- [BENCHMARKS.md](./BENCHMARKS.md)

---

## Testing and validation

### Native tests

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

### ASan + UBSan local run (Linux/WSL)

```bash
cmake -S . -B build-sanitized \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DVISLAM_ENABLE_CUDA=OFF \
  -DVISLAM_ENABLE_STRICT_WARNINGS=ON \
  -DVISLAM_BUILD_TESTS=ON \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build-sanitized -j$(nproc)
ctest --test-dir build-sanitized --output-on-failure
```

### Valgrind leak-check run (Linux/WSL)

```bash
cmake -S . -B build-valgrind -DCMAKE_BUILD_TYPE=RelWithDebInfo -DVISLAM_ENABLE_CUDA=OFF -DVISLAM_BUILD_TESTS=ON
cmake --build build-valgrind -j$(nproc)
valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./build-valgrind/test_imu
```

### Frontend build check

```powershell
cd apps\web
npm run build
```

### API health check

```powershell
curl http://127.0.0.1:8080/api/health
```

### CI pipeline

GitHub Actions workflow:

- [ci.yml](./.github/workflows/ci.yml)

Current CI jobs:

- C++ configure/build/test (`ctest`)
- C++ AddressSanitizer + UndefinedBehaviorSanitizer build/test job
- C++ Valgrind leak-check job on representative tests
- API dependency/schema/syntax checks
- Web production build
- C++ job enforces strict warning policy (`-Wall -Wextra -Wpedantic -Werror` / `/W4 /WX`)

### Regenerate README demo GIF

```powershell
python .\scripts\media\generate_scaffold_demo_gif.py
```

---

## Troubleshooting

### Docker not running

Symptoms:

- `failed to connect to docker API`

Fix:

- Start Docker Desktop, then re-run setup script.

### EuRoC path rejected

Use:

- `GET /api/datasets/euroc/validate?path=<yourPath>`

Check:

- path exists
- required CSV files are present
- nested extraction layout (if zip expanded into parent folders)

### `vislam_app.exe` not found

Fix:

1. Build C++ app (`cmake --build build --config Debug`)
2. Verify `VISLAM_EXECUTABLE` in [services/api/.env.example](./services/api/.env.example)-derived `.env`

### Run completed but no trajectory

For scaffold mode this is valid; backend now stores empty trajectory/metrics and marks run completed with warning logs.

---

## Current limitations

- Current executable orchestration path is EuRoC-focused for real dataset runs.
- KITTI/TUM folders may exist locally, but runtime integration is not yet wired end-to-end in current app flow.
- Dashboard trajectory panel is currently 2D.

---

## Roadmap

- Add first-class KITTI/TUM runtime branches in app entrypoint
- Add richer 3D map/trajectory visualization
- Move from heuristic confidence to trained model + evaluation pipeline
- Expand backend telemetry and diagnostics around tracking quality

---

## License

See [LICENSE](./LICENSE).
