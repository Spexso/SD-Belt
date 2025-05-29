#!/usr/bin/env python3
from pathlib import Path

DATASET_DIR = Path("")  # ← adjust

empty_files = []

for split in ("train", "test", "val"):
    for txt_path in (DATASET_DIR / split / "labels").glob("*.txt"):
        text = txt_path.read_text()
        # strip out whitespace and newlines
        if not text.strip():
            empty_files.append(txt_path)

if empty_files:
    print("Empty label files found:")
    for p in empty_files:
        print(" ", p)
else:
    print("✅ No empty label files detected.")
