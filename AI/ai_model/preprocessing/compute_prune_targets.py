#!/usr/bin/env python3
from pathlib import Path
import sys

# ── CONFIG ─────────────────────────────────────────────────────────────────────
DATASET_DIR          = Path("")  # root containing train/valid/test/{images,labels}
CLASS_ID             = ""      # your remapped class1 ID
ORIGINAL_TRAIN_COUNT = 12000    # original # of class-4 in train before any pruning
TARGET_TRAIN         = 2800     # how many you kept in train
SPLITS               = ("valid", "test")
# ────────────────────────────────────────────────────────────────────────────────

# count current class-4 per split
curr_counts = {}
for split in SPLITS:
    cnt = 0
    labels_dir = DATASET_DIR / split / "labels"
    if not labels_dir.is_dir():
        print(f"⚠️  Missing {labels_dir}, skipping.", file=sys.stderr)
        continue
    for txt in labels_dir.glob("*.txt"):
        for ln in txt.read_text().splitlines():
            if ln.split()[0] == CLASS_ID:
                cnt += 1
                break
    curr_counts[split] = cnt

# compute ratio
ratio = TARGET_TRAIN / ORIGINAL_TRAIN_COUNT
print(f"Prune ratio = {TARGET_TRAIN} / {ORIGINAL_TRAIN_COUNT} = {ratio:.4f}\n")

# suggest deletes
for split in SPLITS:
    orig = curr_counts.get(split, 0)
    keep   = int(orig * ratio)
    delete = orig - keep
    print(f"{split:5s}: current={orig:4d}, keep≈{keep:4d}, delete≈{delete:4d}")
