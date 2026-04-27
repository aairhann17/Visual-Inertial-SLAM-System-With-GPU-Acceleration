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
