#!/usr/bin/env python3
from pathlib import Path
import sys
from collections import Counter

# ── CONFIGURE this to point at your dataset root ─────────────────────────────
DATASET_DIR = Path("")  # contains train/, test/, val/
# ──────────────────────────────────────────────────────────────────────────────

counts = Counter()

for split in ("train", "test", "valid"):
    labels_dir = DATASET_DIR / split / "labels"
    if not labels_dir.is_dir():
        print(f"⚠️  {labels_dir} not found, skipping", file=sys.stderr)
        continue

    for txt in labels_dir.glob("*.txt"):
        for ln in txt.read_text().splitlines():
            parts = ln.strip().split()
            if not parts:
                continue
            cls = parts[0]
            # if you already ran the remapping script, cls will be "4" or "5"
            # otherwise cls will be "0" or "1" and you can map here:
            # cls = {"0":"4","1":"5"}.get(cls, cls)
            counts[cls] += 1

# Print results
print("Annotation counts:")
for cls_id in sorted(counts, key=int):
    print(f"  Class {cls_id}: {counts[cls_id]}")

total = sum(counts.values())
print(f"  └─ Total annotations: {total}")
