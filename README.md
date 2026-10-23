# Visual-Inertial SLAM System with GPU Acceleration

A research-grade, modular **Visual-Inertial Odometry (VIO) SLAM** system written in C++17, with optional CUDA GPU acceleration for real-time performance.

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Architecture](#2-architecture)
3. [Features](#3-features)
4. [Tech Stack](#4-tech-stack)
5. [Datasets](#5-datasets)
6. [Full-Stack + ML Extension](#6-full-stack--ml-extension)
7. [Future Work](#7-future-work)

---

## 1. Project Overview

**Simultaneous Localization and Mapping (SLAM)** is the problem of building a map of an unknown environment while simultaneously tracking an agent's location within it. **Visual-Inertial SLAM** fuses two complementary sensors:

- **Camera** — provides rich geometric information about the environment (texture, structure, landmarks).
- **IMU (Inertial Measurement Unit)** — provides high-frequency motion measurements (acceleration, angular velocity) that are robust to low-texture scenes and fast motion.

By tightly coupling these sensors, VIO SLAM achieves state estimation that is far more robust than either sensor alone — making it the foundation of modern AR/VR headsets, autonomous drones, self-driving vehicles, and robotic navigation systems.

**This system** implements the full VIO pipeline — feature detection and tracking, IMU preintegration, map triangulation, non-linear optimization, and pose-graph refinement — with CUDA-accelerated front-end kernels to sustain real-time throughput on commodity GPU hardware.

---

## 2. Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         Sensor Input                            │
│              Camera Frames          IMU Measurements            │
└────────────────┬────────────────────────┬───────────────────────┘
                 │                        │
                 ▼                        ▼
┌───────────────────────────┐   ┌─────────────────────────────┐
│        FRONTEND           │   │      IMU PREINTEGRATION      │
│                           │   │                              │
│  ┌─────────────────────┐  │   │  Numerical integration of   │
│  │  Feature Detection  │  │   │  gyro + accel measurements  │
│  │  (CUDA-accelerated) │  │   │  between keyframes into a   │
│  └────────┬────────────┘  │   │  delta pose / velocity      │
│           │               │   └──────────────┬──────────────┘
│  ┌────────▼────────────┐  │                  │
│  │  Feature Tracking   │  │                  │
│  │  (Optical Flow /    │  │                  │
│  │   CUDA LK)          │  │                  │
│  └────────┬────────────┘  │                  │
└───────────┼───────────────┘                  │
            │                                  │
            ▼                                  ▼
┌─────────────────────────────────────────────────────────────────┐
│                          MAPPING                                │
│                                                                 │
│   Triangulation → 3-D Map Points → Keyframe Management         │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                          BACKEND                                │
│                                                                 │
│   Bundle Adjustment (g2o / Ceres)                               │
│   Pose Graph Optimization                                       │
│   IMU-visual joint refinement                                   │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                    ESTIMATED TRAJECTORY                         │
│              + DENSE / SEMI-DENSE MAP OUTPUT                    │
└─────────────────────────────────────────────────────────────────┘
```

---

## 3. Features

| Module | Description |
|---|---|
| **Feature Detector** | Grid-based keypoint extraction; CUDA-accelerated scoring kernel |
| **Feature Tracker** | Nearest-neighbour optical flow tracking with configurable search radius |
| **IMU Preintegration** | Incremental integration of accelerometer and gyroscope readings into a compact delta-pose factor |
| **Triangulation** | Stereo / multi-view triangulation of 3-D map points from feature correspondences |
| **Keyframe Manager** | Selects keyframes based on parallax and tracking quality |
| **Map** | Persistent 3-D point map with keyframe indexing |
| **Bundle Adjustment** | Joint nonlinear refinement of camera poses and map points |
| **Pose Graph** | Lightweight pose-graph with edge constraints for global consistency |
| **Optimizer** | Trajectory smoother with iterative refinement |
| **Dataset Loader** | EuRoC MAV loader (current runtime path), plus interfaces prepared for additional dataset formats |
| **Evaluation Scripts** | Python ATE / RPE metric computation and trajectory plotting |

---

## 4. Tech Stack

| Layer | Technology |
|---|---|
| Language | C++17 |
| Linear Algebra | [Eigen 3](https://eigen.tuxfamily.org/) |
| Lie Groups / SE(3) | [Sophus](https://github.com/strasdat/Sophus) |
| Nonlinear Optimization | [g2o](https://github.com/RainerKuemmerle/g2o) / [Ceres Solver](http://ceres-solver.org/) |
| GPU Acceleration | [CUDA](https://developer.nvidia.com/cuda-toolkit) |
| Computer Vision | [OpenCV](https://opencv.org/) |
| Visualization | [Pangolin](https://github.com/stevenlovegrove/Pangolin) |
| Build System | CMake 3.18+ |

---

## 5. Datasets

### EuRoC MAV

Download sequences from [https://rpg.ifi.uzh.ch/docs/IJRR17_Burri.pdf](https://rpg.ifi.uzh.ch/docs/IJRR17_Burri.pdf) and place them under `datasets/euroc/`:

```
datasets/euroc/
└── MH_01_easy/
    ├── mav0/
    │   ├── cam0/
    │   ├── imu0/
    │   └── ...
```

Run:

```bash
./build/Debug/vislam_app --dataset euroc --path datasets/euroc/MH_01_easy
```

### KITTI Odometry

Download from [https://www.cvlibs.net/datasets/kitti/eval_odometry.php](https://www.cvlibs.net/datasets/kitti/eval_odometry.php) and place under `datasets/kitti/`:

```
datasets/kitti/
└── sequences/
    └── 00/
        ├── image_0/
        ├── image_1/
        └── calib.txt
```

Run:

```bash
./build/Debug/vislam_app --dataset kitti --path datasets/kitti/sequences/00
```

### TUM RGB-D

Download from [https://cvg.cit.tum.de/data/datasets/rgbd-dataset](https://cvg.cit.tum.de/data/datasets/rgbd-dataset) and place under `datasets/tum/`:

```
datasets/tum/
└── freiburg1_desk/
    ├── rgb/
    ├── depth/
    └── groundtruth.txt
```

Run:

```bash
./build/Debug/vislam_app --dataset tum --path datasets/tum/freiburg1_desk
```

### Evaluation

After a run, evaluate Absolute Trajectory Error (ATE) and Relative Pose Error (RPE):

```bash
ctest --test-dir build -C Debug --output-on-failure
```

## CUDA Note

`VISLAM_ENABLE_CUDA` is enabled by default. If no CUDA compiler is found, CMake skips `gpu/` targets automatically.

---

## 6. Full-Stack + ML Extension

This repository now includes a full-stack foundation that wraps the C++ `vislam_app` executable:

- API service: [services/api/](./services/api/)
- Web dashboard: [apps/web/](./apps/web/)
- Database schema: [services/api/src/sql/schema.sql](./services/api/src/sql/schema.sql)
- Local Postgres compose setup: [docker-compose.yml](./docker-compose.yml)

### 6.1 Backend (Express + PostgreSQL)

The API service orchestrates SLAM runs and stores:

- run lifecycle/status
- stdout/stderr events
- parsed trajectory poses
- frame-level metrics with ML confidence scores

Core endpoints:

- `POST /api/runs` — start run (`dataset`, `datasetPath`, `maxFrames`)
- `GET /api/runs` — list runs
- `GET /api/runs/:id` — run status/details
- `GET /api/runs/:id/trajectory` — trajectory rows
- `GET /api/runs/:id/metrics` — computed metrics + confidence/risk
- `GET /api/runs/:id/logs` — persisted run events
- `GET /api/runs/:id/stream` — Server-Sent Events stream
- `POST /api/runs/:id/cancel` — request cancellation

### 6.2 ML confidence component

The initial ML component is feature-flagged (`ENABLE_ML_CONFIDENCE=true`) and provides a confidence signal per frame:

- `confidence` in `[0,1]`
- `driftRisk` classification (`stable`, `warning`, `likely_drift`)

Current implementation supports:

- heuristic scoring out-of-the-box
- optional external model weights via `ML_CONFIDENCE_MODEL_PATH` JSON

### 6.3 Local setup

1) Start PostgreSQL:

```bash
docker compose up -d postgres
```

2) Configure API environment:

- copy [services/api/.env.example](./services/api/.env.example) to `services/api/.env`
- verify `VISLAM_EXECUTABLE` path points to your built binary

3) Install API dependencies and initialize DB:

```bash
cd services/api
npm install
npm run db:init
npm run start
```

4) Configure and run web app:

- copy [apps/web/.env.example](./apps/web/.env.example) to `apps/web/.env`

```bash
cd apps/web
npm install
npm run dev
```

Open `http://localhost:5173`.

### 6.4 Dataset note

The C++ runtime currently executes the EuRoC path branch (`--dataset euroc`) in `vislam_app`.  
KITTI and TUM sections above describe expected dataset structures, but end-to-end run orchestration in the current app entrypoint is EuRoC-focused.

---

## 7. Future Work

- Add first-class KITTI/TUM runtime branches in the C++ app orchestration path
- Replace heuristic confidence with trained model inference pipeline + evaluation harness
- Add richer trajectory/map visualization (3D viewer + keyframe overlays)
