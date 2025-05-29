#!/usr/bin/env python3
import sys
from pathlib import Path

# --- USER CONFIG ---
# point this at the root of your Roboflow download
DATASET_DIR = Path("")
# --------------------

old_id1 = "0"  # old class ID for class-4
old_id2 = "1"  # old class ID for class-5

new_id1 = "4"  # new class ID for class-4
new_id2 = "5"  # new class ID for class-5

# mapping old→new
CLASS_MAP = {old_id1: new_id1, old_id2: new_id2}


for split in ("train", "test", "valid"):

    print(f"Processing split: {split}")

    labels_dir = DATASET_DIR / split / "labels"
    if not labels_dir.is_dir():
        print(f"⚠️ labels folder not found: {labels_dir}", file=sys.stderr)
        continue

    progress = 0

    for txt_path in labels_dir.glob("*.txt"):
        progress += 1

        if progress % 100 == 0:
            print(f"Processing {progress} files...")
        lines = txt_path.read_text().splitlines()
        new_lines = []
        for ln in lines:
            parts = ln.strip().split()
            if not parts:
                continue
            old_cls = parts[0]
            new_cls = CLASS_MAP.get(old_cls, old_cls)
            new_lines.append(" ".join([new_cls, *parts[1:]]))

        txt_path.write_text("\n".join(new_lines))

    print(f"✔️  Processed {split}/labels ({len(list(labels_dir.glob('*.txt')))} files)")
