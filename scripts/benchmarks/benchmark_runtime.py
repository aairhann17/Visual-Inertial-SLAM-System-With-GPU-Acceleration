#!/usr/bin/env python3
"""Benchmark vislam_app runtime latency and generate markdown results."""

from __future__ import annotations

import argparse
import json
import math
import os
import shlex
import statistics
import subprocess
import tempfile
import time
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path


@dataclass
class BenchmarkStats:
    label: str
    iterations: int
    mean_ms: float
    median_ms: float
    p95_ms: float
    min_ms: float
    max_ms: float
    loop_hz: float


def parse_command(raw: str) -> list[str]:
    return shlex.split(raw, posix=os.name != "nt")


def percentile(sorted_values: list[float], p: float) -> float:
    if not sorted_values:
        raise ValueError("No samples available for percentile computation.")
    index = (len(sorted_values) - 1) * p
    lower = math.floor(index)
    upper = math.ceil(index)
    if lower == upper:
        return sorted_values[lower]
    fraction = index - lower
    return sorted_values[lower] + (sorted_values[upper] - sorted_values[lower]) * fraction


def run_trials(base_cmd: list[str], iterations: int) -> BenchmarkStats:
    samples: list[float] = []
    with tempfile.TemporaryDirectory() as temp_dir:
        for idx in range(iterations):
            output_path = Path(temp_dir) / f"trajectory_{idx}.txt"
            cmd = [
                *base_cmd,
                "--dataset",
                "scaffold",
                "--path",
                ".",
                "--max-frames",
                "50",
                "--output",
                str(output_path),
            ]
            started = time.perf_counter()
            completed = subprocess.run(cmd, capture_output=True, text=True)
            elapsed_ms = (time.perf_counter() - started) * 1000.0

            if completed.returncode != 0:
                raise RuntimeError(
                    f"Benchmark command failed for {' '.join(base_cmd)}\n"
                    f"stdout:\n{completed.stdout}\n"
                    f"stderr:\n{completed.stderr}"
                )
            samples.append(elapsed_ms)

    ordered = sorted(samples)
    mean_ms = statistics.fmean(ordered)
    median_ms = statistics.median(ordered)
    p95_ms = percentile(ordered, 0.95)
    min_ms = ordered[0]
    max_ms = ordered[-1]
    loop_hz = 1000.0 / mean_ms
    return BenchmarkStats(
        label=" ".join(base_cmd),
        iterations=iterations,
        mean_ms=mean_ms,
        median_ms=median_ms,
        p95_ms=p95_ms,
        min_ms=min_ms,
        max_ms=max_ms,
        loop_hz=loop_hz,
    )


def to_markdown(cpu: BenchmarkStats, gpu: BenchmarkStats | None) -> str:
    generated = datetime.now(timezone.utc).isoformat()
    lines = [
        "# Benchmarks",
        "",
        "This file is generated from `scripts/benchmarks/benchmark_runtime.py` and reports scaffold-mode runtime latency.",
        "",
        f"- Generated at (UTC): `{generated}`",
        f"- CPU command: `{cpu.label}`",
    ]
    if gpu:
        lines.append(f"- GPU command: `{gpu.label}`")
    lines.extend(
        [
            "",
            "## Summary",
            "",
            "| Mode | Iterations | Mean (ms) | Median (ms) | P95 (ms) | Min (ms) | Max (ms) | Loop rate (Hz) |",
            "|---|---:|---:|---:|---:|---:|---:|---:|",
            f"| CPU | {cpu.iterations} | {cpu.mean_ms:.2f} | {cpu.median_ms:.2f} | {cpu.p95_ms:.2f} | {cpu.min_ms:.2f} | {cpu.max_ms:.2f} | {cpu.loop_hz:.2f} |",
        ]
    )
    if gpu:
        lines.append(
            f"| GPU | {gpu.iterations} | {gpu.mean_ms:.2f} | {gpu.median_ms:.2f} | {gpu.p95_ms:.2f} | {gpu.min_ms:.2f} | {gpu.max_ms:.2f} | {gpu.loop_hz:.2f} |"
        )
        speedup = cpu.mean_ms / gpu.mean_ms if gpu.mean_ms else float("inf")
        lines.extend(
            [
                "",
                "## Speedup",
                "",
                f"- CPU/GPU mean latency speedup: **{speedup:.2f}x**",
            ]
        )
    else:
        lines.extend(
            [
                "",
                "## GPU result",
                "",
                "GPU benchmark was not executed in this run. Re-run with `--gpu-cmd` on a CUDA-capable system.",
            ]
        )
    return "\n".join(lines) + "\n"


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--cpu-cmd",
        default="./build/Debug/vislam_app.exe",
        help="Command to execute CPU benchmark binary.",
    )
    parser.add_argument(
        "--gpu-cmd",
        default="",
        help="Optional command for GPU benchmark binary.",
    )
    parser.add_argument("--iterations", type=int, default=100)
    parser.add_argument(
        "--json-output",
        default="scripts/benchmarks/last-run.json",
        help="Output path for raw benchmark JSON.",
    )
    parser.add_argument(
        "--markdown-output",
        default="BENCHMARKS.md",
        help="Output path for markdown summary.",
    )
    args = parser.parse_args()

    cpu_cmd = parse_command(args.cpu_cmd)
    gpu_cmd = parse_command(args.gpu_cmd) if args.gpu_cmd.strip() else []

    cpu_stats = run_trials(cpu_cmd, args.iterations)
    gpu_stats = run_trials(gpu_cmd, args.iterations) if gpu_cmd else None

    payload = {
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "iterations": args.iterations,
        "cpu": cpu_stats.__dict__,
        "gpu": gpu_stats.__dict__ if gpu_stats else None,
    }

    json_path = Path(args.json_output)
    json_path.parent.mkdir(parents=True, exist_ok=True)
    json_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")

    markdown = to_markdown(cpu_stats, gpu_stats)
    Path(args.markdown_output).write_text(markdown, encoding="utf-8")
    print(f"Wrote {json_path} and {args.markdown_output}")


if __name__ == "__main__":
    main()

