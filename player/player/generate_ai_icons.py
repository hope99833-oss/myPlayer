"""
Local AI icon generator — uses diffusers + GPU to generate UI icons.
First run downloads the model (~4GB), subsequent runs use cache.

Usage:
    python generate_ai_icons.py                  # all icons, default size
    python generate_ai_icons.py --size 128       # custom size
    python generate_ai_icons.py --names play,pause  # specific icons
    python generate_ai_icons.py --model runwayml/stable-diffusion-v1-5
"""

import argparse
import os
import torch

os.environ["HF_ENDPOINT"] = os.environ.get("HF_ENDPOINT", "https://hf-mirror.com")

from diffusers import StableDiffusionPipeline

# ── Icon prompt templates ───────────────────────────────────────────

ICON_PROMPTS = {
    "play": {
        "prompt": "a single play button icon, white filled triangle pointing right, centered, flat minimal vector icon design, clean edges, solid white shape, UI component, transparent background",
        "negative": "text, watermark, blurry, 3d, photo, realistic, shadow, gradient, multiple objects, deformed, ugly, low quality, jpeg artifacts, background pattern",
    },
    "pause": {
        "prompt": "a single pause button icon, two white vertical parallel bars, centered, flat minimal vector icon design, clean edges, solid white shapes, UI component, transparent background",
        "negative": "text, watermark, blurry, 3d, photo, realistic, shadow, gradient, multiple objects, deformed, ugly, low quality, jpeg artifacts, background pattern",
    },
    "stop": {
        "prompt": "a single stop button icon, a white filled square with rounded corners, centered, flat minimal vector icon design, clean edges, solid white shape, UI component, transparent background",
        "negative": "text, watermark, blurry, 3d, photo, realistic, shadow, gradient, multiple objects, deformed, ugly, low quality, jpeg artifacts, background pattern",
    },
    "previous": {
        "prompt": "a single previous track button icon, white filled triangle pointing left with a vertical bar on its left side, centered, flat minimal vector icon design, clean edges, UI component, transparent background",
        "negative": "text, watermark, blurry, 3d, photo, realistic, shadow, gradient, multiple objects, deformed, ugly, low quality, jpeg artifacts, background pattern",
    },
    "next": {
        "prompt": "a single next track button icon, white filled triangle pointing right with a vertical bar on its right side, centered, flat minimal vector icon design, clean edges, UI component, transparent background",
        "negative": "text, watermark, blurry, 3d, photo, realistic, shadow, gradient, multiple objects, deformed, ugly, low quality, jpeg artifacts, background pattern",
    },
    "volume": {
        "prompt": "a single volume speaker icon, white filled speaker shape with sound wave arcs, centered, flat minimal vector icon design, clean edges, UI component, transparent background",
        "negative": "text, watermark, blurry, 3d, photo, realistic, shadow, gradient, multiple objects, deformed, ugly, low quality, jpeg artifacts, background pattern",
    },
    "fullscreen": {
        "prompt": "a single fullscreen expand icon, four white corner arrows pointing outward, centered, flat minimal vector icon design, clean edges, UI component, transparent background",
        "negative": "text, watermark, blurry, 3d, photo, realistic, shadow, gradient, multiple objects, deformed, ugly, low quality, jpeg artifacts, background pattern",
    },
}


def remove_background(img, threshold=240):
    """Convert near-white pixels to transparent for icon use."""
    import numpy as np
    arr = img.convert("RGBA")
    data = arr.getdata()
    new_data = []
    for item in data:
        if item[0] > threshold and item[1] > threshold and item[2] > threshold:
            new_data.append((255, 255, 255, 0))
        else:
            new_data.append(item)
    arr.putdata(new_data)
    return arr


def generate_icon(pipe, name, cfg, size, output_dir, num_steps=20, guidance=7.5):
    print(f"  generating: {name} ...")
    image = pipe(
        prompt=cfg["prompt"],
        negative_prompt=cfg["negative"],
        num_inference_steps=num_steps,
        guidance_scale=guidance,
        width=size,
        height=size,
    ).images[0]

    icon = remove_background(image)
    path = os.path.join(output_dir, f"{name}.png")
    icon.save(path, "PNG")
    print(f"  saved: {path}")
    return path


def main():
    parser = argparse.ArgumentParser(description="AI icon generator with diffusers + GPU")
    parser.add_argument("--size", type=int, default=512, help="Generation size (default: 512)")
    parser.add_argument("--output-size", type=int, default=64, help="Final output size (default: 64)")
    parser.add_argument("--output-dir", type=str, default="resources", help="Output directory")
    parser.add_argument("--names", type=str, default=None, help="Comma-separated icon names")
    parser.add_argument("--model", type=str, default="runwayml/stable-diffusion-v1-5", help="Model ID")
    parser.add_argument("--steps", type=int, default=20, help="Inference steps")
    parser.add_argument("--guidance", type=float, default=7.5, help="CFG scale")
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    print(f"Device: {torch.cuda.get_device_name(0)}")
    print(f"Model: {args.model}")
    print(f"Loading model (first run downloads ~4GB, then cached)...")

    pipe = StableDiffusionPipeline.from_pretrained(
        args.model,
        torch_dtype=torch.float16,
        safety_checker=None,
        requires_safety_checker=False,
    )
    pipe = pipe.to("cuda")
    pipe.enable_attention_slicing()

    names = args.names.split(",") if args.names else list(ICON_PROMPTS.keys())
    for name in names:
        name = name.strip()
        if name not in ICON_PROMPTS:
            print(f"  skip unknown: {name}")
            continue
        generate_icon(pipe, name, ICON_PROMPTS[name], args.size, args.output_dir, args.steps, args.guidance)

    print(f"\nDone. Generated {len(names)} icon(s) in {args.output_dir}/")


if __name__ == "__main__":
    main()
