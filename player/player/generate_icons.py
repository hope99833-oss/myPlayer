"""
Player icon generator — draws clean, watermark-free icons using Pillow.
Usage: python generate_icons.py [--size 64] [--output-dir resources]
"""

import argparse
import os
from PIL import Image, ImageDraw


def draw_rounded_rect(draw, xy, radius, fill):
    x0, y0, x1, y1 = xy
    r = min(radius, (x1 - x0) // 2, (y1 - y0) // 2)
    draw.rectangle([x0 + r, y0, x1 - r, y1], fill=fill)
    draw.rectangle([x0, y0 + r, x1, y1 - r], fill=fill)
    draw.pieslice([x0, y0, x0 + 2 * r, y0 + 2 * r], 180, 270, fill=fill)
    draw.pieslice([x1 - 2 * r, y0, x1, y0 + 2 * r], 270, 360, fill=fill)
    draw.pieslice([x0, y1 - 2 * r, x0 + 2 * r, y1], 90, 180, fill=fill)
    draw.pieslice([x1 - 2 * r, y1 - 2 * r, x1, y1], 0, 90, fill=fill)


def make_icon(draw_func, size):
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw_func(draw, size)
    return img


def draw_play(draw, s):
    m = int(s * 0.06)
    pts = [(m, m), (m, s - m), (s - m, s // 2)]
    draw.polygon(pts, fill=(255, 255, 255, 255))


def draw_pause(draw, s):
    m = int(s * 0.10)
    w = int(s * 0.22)
    gap = int(s * 0.10)
    cx = s // 2
    draw_rounded_rect(draw, [cx - gap // 2 - w, m, cx - gap // 2, s - m], w // 3, (255, 255, 255, 255))
    draw_rounded_rect(draw, [cx + gap // 2, m, cx + gap // 2 + w, s - m], w // 3, (255, 255, 255, 255))


def draw_stop(draw, s):
    m = int(s * 0.10)
    r = int(s * 0.06)
    draw_rounded_rect(draw, [m, m, s - m, s - m], r, (255, 255, 255, 255))


def draw_prev(draw, s):
    m = int(s * 0.06)
    bar_w = max(4, int(s * 0.08))
    cy = s // 2
    tri_h = int(s * 0.42)
    draw.rectangle([m, cy - tri_h, m + bar_w, cy + tri_h], fill=(255, 255, 255, 255))
    tri_left = m + bar_w + int(s * 0.04)
    draw.polygon([(tri_left, m), (tri_left, s - m), (s - m, cy)], fill=(255, 255, 255, 255))


def draw_next(draw, s):
    m = int(s * 0.06)
    bar_w = max(4, int(s * 0.08))
    cy = s // 2
    tri_h = int(s * 0.42)
    draw.rectangle([s - m - bar_w, cy - tri_h, s - m, cy + tri_h], fill=(255, 255, 255, 255))
    tri_right = s - m - bar_w - int(s * 0.04)
    draw.polygon([(m, cy), (tri_right, m), (tri_right, s - m)], fill=(255, 255, 255, 255))


def draw_volume(draw, s):
    cy = s // 2
    bx0 = int(s * 0.04)
    bx1 = int(s * 0.30)
    by0 = int(s * 0.25)
    by1 = int(s * 0.75)
    draw.rectangle([bx0, by0, bx1, by1], fill=(255, 255, 255, 255))
    cx1 = int(s * 0.58)
    draw.polygon([(bx1, by0), (bx1, by1), (cx1, int(s * 0.05)), (cx1, int(s * 0.95))], fill=(255, 255, 255, 255))
    lw = max(4, int(s * 0.08))
    ax = int(s * 0.66)
    r1 = int(s * 0.17)
    r2 = int(s * 0.30)
    draw.arc([ax - r1, cy - r1, ax + r1, cy + r1], -50, 50, fill=(255, 255, 255, 255), width=lw)
    draw.arc([ax - r2, cy - r2, ax + r2, cy + r2], -40, 40, fill=(255, 255, 255, 255), width=lw)


def draw_fullscreen(draw, s):
    m = int(s * 0.06)
    arm = int(s * 0.34)
    thickness = max(4, int(s * 0.10))
    color = (255, 255, 255, 255)
    draw.rectangle([m, m, m + arm, m + thickness], fill=color)
    draw.rectangle([m, m, m + thickness, m + arm], fill=color)
    draw.rectangle([s - m - arm, m, s - m, m + thickness], fill=color)
    draw.rectangle([s - m - thickness, m, s - m, m + arm], fill=color)
    draw.rectangle([m, s - m - thickness, m + arm, s - m], fill=color)
    draw.rectangle([m, s - m - arm, m + thickness, s - m], fill=color)
    draw.rectangle([s - m - arm, s - m - thickness, s - m, s - m], fill=color)
    draw.rectangle([s - m - thickness, s - m - arm, s - m, s - m], fill=color)


ICONS = {
    "play": draw_play,
    "pause": draw_pause,
    "stop": draw_stop,
    "previous": draw_prev,
    "next": draw_next,
    "volume": draw_volume,
    "fullscreen": draw_fullscreen,
}


def main():
    parser = argparse.ArgumentParser(description="Generate player icons with Pillow")
    parser.add_argument("--size", type=int, default=64, help="Icon size in pixels (default: 64)")
    parser.add_argument("--output-dir", type=str, default="resources", help="Output directory")
    parser.add_argument("--names", type=str, default=None, help="Comma-separated icon names")
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    names = args.names.split(",") if args.names else list(ICONS.keys())
    for name in names:
        name = name.strip()
        if name not in ICONS:
            print(f"  skip unknown icon: {name}")
            continue
        img = make_icon(ICONS[name], args.size)
        path = os.path.join(args.output_dir, f"{name}.png")
        img.save(path, "PNG")
        print(f"  {path}  ({args.size}x{args.size})")

    print(f"\nDone. Generated {len(names)} icon(s) in {args.output_dir}/")


if __name__ == "__main__":
    main()
