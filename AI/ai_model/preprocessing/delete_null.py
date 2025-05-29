#!/usr/bin/env python3
import sys
from pathlib import Path

# ── CONFIG: adjust this to your dataset root ───────────────────────────────────
DATASET_DIR = Path("")  # contains train/images and train/labels
# ──────────────────────────────────────────────────────────────────────────────

train_labels = DATASET_DIR / "train" / "labels"
train_images = DATASET_DIR / "train" / "images"

if not train_labels.is_dir() or not train_images.is_dir():
    print(f"Error: expected folders not found under {DATASET_DIR}", file=sys.stderr)
    sys.exit(1)

removed = 0
for label_file in train_labels.glob("*.txt"):
    if not label_file.read_text().strip():  # empty or whitespace-only
        stem = label_file.stem
        # delete label
        label_file.unlink()
        print(f"Deleted label: {label_file}")
        # delete matching images
        for img in train_images.glob(f"{stem}.*"):
            img.unlink()
            print(f"Deleted image: {img}")
        removed += 1

print(f"\nDone. Processed {removed} empty label files.")
