#!/usr/bin/env python3
import random, sys
from pathlib import Path

# ── CONFIGURE ─────────────────────────────────────────────────────────────────
DATASET_DIR = Path("")    # root containing train/{images,labels}
CLASS_ID     = "1"                             # now that 0→4 and 1→5, class1 lives at ID “4”
TARGET_KEEP  = 2200                            # we want ~3.5k of class1
# ──────────────────────────────────────────────────────────────────────────────

train_labels = DATASET_DIR / "train" / "labels"
train_images = DATASET_DIR / "train" / "images"

if not train_labels.is_dir() or not train_images.is_dir():
    print(f"❌ Cannot find train/labels or train/images under {DATASET_DIR}", file=sys.stderr)
    sys.exit(1)

# 1) gather all label files that include class-ID “4”
all_cls1 = []
for txt in train_labels.glob("*.txt"):
    for ln in txt.read_text().splitlines():
        parts = ln.split()
        if parts and parts[0] == CLASS_ID:
            all_cls1.append(txt)
            break

current_count = len(all_cls1)
delete_count  = current_count - TARGET_KEEP

if delete_count <= 0:
    print(f"✅ No pruning needed: found {current_count}, target is {TARGET_KEEP}.")
    sys.exit(0)

print(f"Found {current_count} class {CLASS_ID} files; will delete {delete_count} of them.")

# 2) randomly select which ones to delete
random.seed(42)
to_delete = random.sample(all_cls1, delete_count)

# 3) delete each label + its matching image(s)
extensions = ("jpg","jpeg","png","bmp")
for lbl_path in to_delete:
    lbl_path.unlink()
    stem = lbl_path.stem
    for ext in extensions:
        img = train_images / f"{stem}.{ext}"
        if img.exists():
            img.unlink()

print(f"✔️  Deleted {delete_count} examples of class {CLASS_ID} from train/")
