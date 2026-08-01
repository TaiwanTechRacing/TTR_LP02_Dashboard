#!/usr/bin/env python3
"""
Re-encode GIFs down to something a dashboard can actually store.

The source animations are web-sized: 374x374 and 480x480 at 33 fps, 12.2 MB for
three of them. The panel is 480x272, the containers holding them are 300x200,
and the internal flash has about 1.5 MB spare. Three levers close that gap, in
descending order of effect:

  frame rate   33 fps is far more than a decorative animation needs. Halving it
               halves the data, and 15-17 fps still reads as smooth.
  size         Scaling to the container removes pixels nothing ever displays.
  palette      A single palette shared by every frame compresses far better
               than a per-frame adaptive one. Re-encoding naively with adaptive
               palettes actually made one of these files *larger* than the
               original.

Usage:

    python tools/gif_optimize.py dashboard_layout/gif -o dashboard_layout/gif/out
    python tools/gif_optimize.py input.gif -o out --fps 12 --colors 64

Defaults target roughly 3 MB for the current three animations, which fits any
W25Q part on the board with room to spare.
"""

import argparse
import io
import os
import sys

try:
    from PIL import Image, ImageSequence
except ImportError:
    sys.exit("Pillow is required:  pip install pillow")


DEFAULT_BOX = (300, 200)     # the EEZ container size
DEFAULT_FPS = 17
DEFAULT_COLORS = 128


def frame_interval_ms(image, fallback=40):
    """Source frame interval, falling back when a GIF omits it."""
    return image.info.get("duration") or fallback


def repack(path, box, target_fps, colors):
    src = Image.open(path)

    src_interval = frame_interval_ms(src)
    target_interval = max(1, int(round(1000.0 / target_fps)))

    # Keep one frame in every `step`, so the result lands near the target rate
    # without resampling timings unevenly.
    step = max(1, int(round(float(target_interval) / src_interval)))
    out_interval = src_interval * step

    scale = min(box[0] / float(src.size[0]), box[1] / float(src.size[1]), 1.0)
    new_size = (max(1, int(src.size[0] * scale)), max(1, int(src.size[1] * scale)))

    frames = []
    for index, frame in enumerate(ImageSequence.Iterator(src)):
        if index % step:
            continue
        frames.append(frame.copy().convert("RGB").resize(new_size, Image.LANCZOS))

    if not frames:
        raise ValueError("no frames survived decimation")

    # One palette derived from the first frame and reused, rather than an
    # adaptive palette per frame. Shared palettes let the GIF encoder emit
    # inter-frame deltas instead of a fresh colour table every frame.
    base = frames[0].quantize(colors=colors, method=Image.MEDIANCUT)
    paletted = [f.quantize(palette=base, dither=Image.FLOYDSTEINBERG) for f in frames]

    # Source frames carry a tuple-valued transparency entry that PIL's GIF
    # writer cannot serialise. After quantising to a shared palette the frames
    # are opaque anyway, so drop it.
    for frame in paletted:
        frame.info.pop("transparency", None)

    buffer = io.BytesIO()
    paletted[0].save(buffer, format="GIF", save_all=True,
                     append_images=paletted[1:], loop=0,
                     duration=out_interval, optimize=True, disposal=2)

    return {
        "data": buffer.getvalue(),
        "size": new_size,
        "frames": len(paletted),
        "fps": 1000.0 / out_interval,
        "src_size": src.size,
        "src_frames": getattr(src, "n_frames", 1),
    }


def collect_inputs(path):
    if os.path.isdir(path):
        return sorted(os.path.join(path, n) for n in os.listdir(path)
                      if n.lower().endswith(".gif"))
    return [path]


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("input", help="a .gif or a directory of them")
    parser.add_argument("-o", "--output", required=True, help="output directory")
    parser.add_argument("--max-width", type=int, default=DEFAULT_BOX[0])
    parser.add_argument("--max-height", type=int, default=DEFAULT_BOX[1])
    parser.add_argument("--fps", type=float, default=DEFAULT_FPS)
    parser.add_argument("--colors", type=int, default=DEFAULT_COLORS,
                        help="palette entries, 2..256")
    args = parser.parse_args()

    inputs = collect_inputs(args.input)
    if not inputs:
        sys.exit("no .gif files found in %s" % args.input)

    os.makedirs(args.output, exist_ok=True)
    box = (args.max_width, args.max_height)

    print("%-26s %11s %11s  %s" % ("file", "before", "after", "result"))
    print("-" * 78)

    before_total = 0
    after_total = 0

    for path in inputs:
        before = os.path.getsize(path)
        result = repack(path, box, args.fps, args.colors)
        after = len(result["data"])

        out_path = os.path.join(args.output, os.path.basename(path))
        with open(out_path, "wb") as handle:
            handle.write(result["data"])

        before_total += before
        after_total += after

        print("%-26s %11d %11d  %dx%d -> %dx%d, %d -> %d frames, %.0f fps"
              % (os.path.basename(path), before, after,
                 result["src_size"][0], result["src_size"][1],
                 result["size"][0], result["size"][1],
                 result["src_frames"], result["frames"], result["fps"]))

    print("-" * 78)
    print("%-26s %11d %11d  %.2f MB -> %.2f MB  (%.0f%% smaller)"
          % ("TOTAL", before_total, after_total,
             before_total / 1048576.0, after_total / 1048576.0,
             100.0 * (1.0 - float(after_total) / before_total)))
    print()
    print("Written to %s" % os.path.abspath(args.output))


if __name__ == "__main__":
    main()
