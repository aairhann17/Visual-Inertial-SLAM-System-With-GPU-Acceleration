# Visual-Inertial-SLAM-System-With-GPU-Acceleration

Starter scaffold for a modular visual-inertial SLAM project with optional CUDA acceleration.

## Structure

- `include/`: public module headers (`frontend`, `backend`, `mapping`, `utils`)
- `src/`: source implementations and `main.cpp`
- `gpu/`: CUDA kernels and CMake target (`vislam_gpu`)
- `tests/`: lightweight C++ test executables
- `scripts/`: Python utilities for trajectory plotting/evaluation

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/Debug/vislam_app
```

## Test

```bash
ctest --test-dir build -C Debug --output-on-failure
```

## CUDA Note

`VISLAM_ENABLE_CUDA` is enabled by default. If no CUDA compiler is found, CMake skips `gpu/` targets automatically.


## Architecture Diagram (High-Level Overview)

                         ┌────────────────────────────────────────┐
                         │              Input Streams              │
                         │────────────────────────────────────────│
                         │   • Camera Frames (Left/Right)         │
                         │   • IMU Data (Accel + Gyro)            │
                         └────────────────────────────────────────┘
                                         │
                                         ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│                              FRONTEND (VISION)                               │
│──────────────────────────────────────────────────────────────────────────────│
│  Feature Detection → Feature Tracking → Outlier Rejection → Keyframe Logic   │
│                                                                              │
│  • ORB/FAST Detector                                                         │
│  • KLT Optical Flow                                                          │
│  • RANSAC (Fundamental / Essential Matrix)                                   │
│  • Keyframe Selection                                                        │
└──────────────────────────────────────────────────────────────────────────────┘
                                         │
                                         ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│                               IMU PRE-INTEGRATION                            │
│──────────────────────────────────────────────────────────────────────────────│
│  • Bias Correction                                                            │
│  • Noise Propagation                                                          │
│  • Pre-integrated Delta Pose (ΔR, Δv, Δp)                                     │
│  • Jacobians + Covariance                                                     │
└──────────────────────────────────────────────────────────────────────────────┘
                                         │
                                         ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│                               INITIAL ESTIMATION                              │
│──────────────────────────────────────────────────────────────────────────────│
│  • Triangulation (3D Landmarks)                                               │
│  • PnP Pose Estimation                                                        │
│  • Visual + Inertial Fusion                                                   │
└──────────────────────────────────────────────────────────────────────────────┘
                                         │
                                         ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│                                BACKEND (OPTIMIZER)                            │
│──────────────────────────────────────────────────────────────────────────────│
│  • Nonlinear Optimization (Gauss-Newton / LM)                                 │
│  • Bundle Adjustment (Local / Global)                                         │
│  • Sliding Window Optimization                                                 │
│  • Marginalization of Old Keyframes                                           │
└──────────────────────────────────────────────────────────────────────────────┘
                                         │
                                         ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│                                LOOP CLOSURE                                   │
│──────────────────────────────────────────────────────────────────────────────│
│  • Bag-of-Words (DBoW2)                                                       │
│  • Loop Candidate Detection                                                   │
│  • Pose Graph Optimization                                                    │
│  • Global Trajectory Correction                                               │
└──────────────────────────────────────────────────────────────────────────────┘
                                         │
                                         ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│                                 MAPPING                                       │
│──────────────────────────────────────────────────────────────────────────────│
│  • 3D Landmark Management                                                     │
│  • Keyframe Graph                                                             │
│  • Map Pruning                                                                │
└──────────────────────────────────────────────────────────────────────────────┘
                                         │
                                         ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│                             GPU ACCELERATION (CUDA)                           │
│──────────────────────────────────────────────────────────────────────────────│
│  • Parallel Feature Extraction                                                 │
│  • GPU Optical Flow                                                            │
│  • Descriptor Computation                                                      │
└──────────────────────────────────────────────────────────────────────────────┘
                                         │
                                         ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│                           VISUALIZATION & EVALUATION                          │
│──────────────────────────────────────────────────────────────────────────────│
│  • Real-Time 3D Viewer (Pangolin)                                             │
│  • Trajectory Plots (ATE/RPE)                                                 │
│  • Map Rendering                                                               │
└──────────────────────────────────────────────────────────────────────────────┘

