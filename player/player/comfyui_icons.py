"""
ComfyUI icon generator — uses ComfyUI API to generate UI icons.
Requires ComfyUI server running on http://127.0.0.1:8188

Usage:
    python comfyui_icons.py                  # all icons
    python comfyui_icons.py --names play,pause  # specific icons
    python comfyui_icons.py --size 512       # generation size
    python comfyui_icons.py --output-size 64 # final output size
"""

import argparse
import json
import os
import time
import urllib.request
import urllib.error
from PIL import Image

COMFYUI_URL = "http://127.0.0.1:8188"

# ─ Icon prompt templates ───────────────────────────────────────────

ICON_PROMPTS = {
    "play": "a single play button icon, white filled triangle pointing right, centered, flat minimal vector icon design, clean edges, solid white shape, UI component, transparent background",
    "pause": "a single pause button icon, two white vertical parallel bars, centered, flat minimal vector icon design, clean edges, solid white shapes, UI component, transparent background",
    "stop": "a single stop button icon, a white filled square with rounded corners, centered, flat minimal vector icon design, clean edges, solid white shape, UI component, transparent background",
    "previous": "a single previous track button icon, white filled triangle pointing left with a vertical bar on its left side, centered, flat minimal vector icon design, clean edges, UI component, transparent background",
    "next": "a single next track button icon, white filled triangle pointing right with a vertical bar on its right side, centered, flat minimal vector icon design, clean edges, UI component, transparent background",
    "volume": "a single volume speaker icon, white filled speaker shape with sound wave arcs, centered, flat minimal vector icon design, clean edges, UI component, transparent background",
    "fullscreen": "a single fullscreen expand icon, four white corner arrows pointing outward, centered, flat minimal vector icon design, clean edges, UI component, transparent background",
}

NEGATIVE_PROMPT = "text, watermark, blurry, 3d, photo, realistic, shadow, gradient, multiple objects, deformed, ugly, low quality, jpeg artifacts, background pattern, colorful"


def queue_prompt(prompt):
    """Send prompt to ComfyUI and return prompt_id."""
    data = json.dumps({"prompt": prompt}).encode("utf-8")
    req = urllib.request.Request(f"{COMFYUI_URL}/prompt", data=data, headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req) as resp:
        return json.loads(resp.read())["prompt_id"]


def get_history(prompt_id):
    """Get generation history for a prompt_id."""
    with urllib.request.urlopen(f"{COMFYUI_URL}/history/{prompt_id}") as resp:
        return json.loads(resp.read())


def get_image(filename, subfolder, folder_type):
    """Download generated image from ComfyUI."""
    data = urllib.parse.urlencode({"filename": filename, "subfolder": subfolder, "type": folder_type})
    with urllib.request.urlopen(f"{COMFYUI_URL}/view?{data}") as resp:
        return resp.read()


def build_workflow(prompt_text, negative_text, size, seed):
    """Build ComfyUI workflow JSON for icon generation."""
    return {
        "3": {
            "class_type": "KSampler",
            "inputs": {
                "seed": seed,
                "steps": 20,
                "cfg": 7.5,
                "sampler_name": "euler",
                "scheduler": "normal",
                "denoise": 1.0,
                "model": ["4", 0],
                "positive": ["6", 0],
                "negative": ["7", 0],
                "latent_image": ["5", 0],
            },
        },
        "4": {
            "class_type": "CheckpointLoaderSimple",
            "inputs": {"ckpt_name": "v1-5-pruned-emaonly.safetensors"},
        },
        "5": {
            "class_type": "EmptyLatentImage",
            "inputs": {"width": size, "height": size, "batch_size": 1},
        },
        "6": {
            "class_type": "CLIPTextEncode",
            "inputs": {"text": prompt_text, "clip": ["4", 1]},
        },
        "7": {
            "class_type": "CLIPTextEncode",
            "inputs": {"text": negative_text, "clip": ["4", 1]},
        },
        "8": {
            "class_type": "VAEDecode",
            "inputs": {"samples": ["3", 0], "vae": ["4", 2]},
        },
        "9": {
            "class_type": "SaveImage",
            "inputs": {"filename_prefix": "icon", "images": ["8", 0]},
        },
    }


def remove_background(img, threshold=240):
    """Convert near-white pixels to transparent."""
    arr = img.convert("RGBA")
    data = list(arr.getdata())
    new_data = []
    for item in data:
        if item[0] > threshold and item[1] > threshold and item[2] > threshold:
            new_data.append((255, 255, 255, 0))
        else:
            new_data.append(item)
    arr.putdata(new_data)
    return arr


def generate_icon(name, prompt, size, output_size, output_dir):
    """Generate a single icon using ComfyUI."""
    print(f"  generating: {name} ...")

    seed = int(time.time() * 1000) % (2**32)
    workflow = build_workflow(prompt, NEGATIVE_PROMPT, size, seed)

    prompt_id = queue_prompt(workflow)

    # Wait for completion
    while True:
        history = get_history(prompt_id)
        if prompt_id in history:
            break
        time.sleep(1)

    # Get output image
    outputs = history[prompt_id]["outputs"]
    for node_id, output in outputs.items():
        if "images" in output:
            for img_info in output["images"]:
                img_data = get_image(img_info["filename"], img_info.get("subfolder", ""), img_info["type"])

                # Save and process
                from io import BytesIO
                img = Image.open(BytesIO(img_data))
                icon = remove_background(img)
                icon = icon.resize((output_size, output_size), Image.LANCZOS)

                path = os.path.join(output_dir, f"{name}.png")
                icon.save(path, "PNG")
                print(f"  saved: {path} ({output_size}x{output_size})")
                return

    print(f"  WARNING: no output for {name}")


def main():
    import urllib.parse

    parser = argparse.ArgumentParser(description="ComfyUI icon generator")
    parser.add_argument("--size", type=int, default=512, help="Generation size (default: 512)")
    parser.add_argument("--output-size", type=int, default=64, help="Final output size (default: 64)")
    parser.add_argument("--output-dir", type=str, default="resources", help="Output directory")
    parser.add_argument("--names", type=str, default=None, help="Comma-separated icon names")
    args = parser.parse_args()

    os.makedirs(args.output_dir, exist_ok=True)

    # Check server
    try:
        urllib.request.urlopen(f"{COMFYUI_URL}/system_stats", timeout=5)
        print(f"ComfyUI server: {COMFYUI_URL}")
    except Exception as e:
        print(f"Error: Cannot connect to ComfyUI at {COMFYUI_URL}: {e}")
        return

    names = args.names.split(",") if args.names else list(ICON_PROMPTS.keys())
    for name in names:
        name = name.strip()
        if name not in ICON_PROMPTS:
            print(f"  skip unknown: {name}")
            continue
        generate_icon(name, ICON_PROMPTS[name], args.size, args.output_size, args.output_dir)

    print(f"\nDone. Generated {len(names)} icon(s) in {args.output_dir}/")


if __name__ == "__main__":
    main()
