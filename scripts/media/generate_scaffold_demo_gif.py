#!/usr/bin/env python3
"""Generate a lightweight animated GIF for README visual preview."""

from __future__ import annotations

import math
from pathlib import Path

from PIL import Image, ImageDraw


def make_frame(index: int, width: int, height: int, total: int) -> Image.Image:
    img = Image.new("RGB", (width, height), (10, 18, 32))
    draw = ImageDraw.Draw(img)

    margin = 24
    draw.rectangle((margin, margin, width - margin, height - margin), outline=(60, 90, 140), width=2)
    draw.text((margin + 8, margin + 6), "Visual-Inertial SLAM (scaffold demo)", fill=(180, 210, 255))

    points = []
    for i in range(index + 1):
        t = i / max(1, total - 1)
        x = margin + 20 + t * (width - (2 * margin + 40))
        y_center = margin + (height - 2 * margin) * 0.55
        y = y_center + math.sin(t * math.pi * 3.0) * 45.0
        points.append((x, y))

    if len(points) > 1:
        draw.line(points, fill=(66, 153, 245), width=3)
    if points:
        draw.ellipse((points[0][0] - 4, points[0][1] - 4, points[0][0] + 4, points[0][1] + 4), fill=(52, 211, 153))
        draw.ellipse((points[-1][0] - 5, points[-1][1] - 5, points[-1][0] + 5, points[-1][1] + 5), fill=(248, 113, 113))

    for i in range(20):
        t = (i + index * 0.3) / 20.0
        fx = margin + 10 + (t % 1.0) * (width - 2 * margin - 20)
        fy = margin + 40 + ((i * 31 + index * 7) % (height - 2 * margin - 50))
        draw.rectangle((fx, fy, fx + 2, fy + 2), fill=(226, 232, 240))

    status = f"Frame: {index + 1:03d}  |  Tracking: ACTIVE"
    draw.text((margin + 8, height - margin - 18), status, fill=(196, 210, 229))
    return img


def main() -> None:
    width, height = 860, 420
    total_frames = 48
    frames = [make_frame(i, width, height, total_frames) for i in range(total_frames)]

    out_path = Path("docs/media/scaffold-demo.gif")
    out_path.parent.mkdir(parents=True, exist_ok=True)
    frames[0].save(
        out_path,
        save_all=True,
        append_images=frames[1:],
        optimize=True,
        duration=55,
        loop=0,
    )
    print(f"Generated {out_path}")


if __name__ == "__main__":
    main()

