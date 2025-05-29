# Data Preparation & Training Scripts

This folder contains helper scripts and a notebook to prepare, analyze, and train your dataset and model. Each script is standalone and focuses on a specific preprocessing or evaluation task.

## Table of Contents

* [Prerequisites](#prerequisites)
* [Scripts Overview](#scripts-overview)
* [Usage](#usage)
* [Dependencies](#dependencies)
* [License](#license)

---

## Prerequisites

* Python 3.7+
* A working virtual environment (recommended)
* Required Python packages (see [Dependencies](#dependencies))

---

## Scripts Overview

| Script                     | Description                                                                                     |
| -------------------------- | ----------------------------------------------------------------------------------------------- |
| `add_false_samples.py`     | Augment minority classes by duplicating or generating false samples to balance dataset classes. |
| `balance_count.py`         | Analyze class distribution and output target counts for balancing or sampling purposes.         |
| `compute_prune_targets.py` | Calculate which samples or channels to prune based on class frequencies or model metrics.       |
| `count_class.py`           | Count the number of samples per class in your dataset directory.                                |
| `count_null.py`            | Scan annotation files and report entries with missing or null labels.                           |
| `delete_null.py`           | Remove samples or annotation entries that have null or invalid labels.                          |
| `remap_labels.py`          | Remap label values according to a provided mapping (e.g., merge classes or standardize labels). |
| `inference_test.py`        | Run inference on a set of images or videos, and report metrics (accuracy, speed).               |
| `train.ipynb`              | Jupyter notebook demonstrating end-to-end model training and evaluation workflow.               |

---

## Usage

Activate your virtual environment and install dependencies:

```bash
python -m venv venv
source venv/bin/activate    # macOS/Linux
# .\venv\Scripts\activate   # Windows
pip install -r requirements.txt
```

### Running a Script

Each script provides a `--help` flag showing its arguments:

```bash
python add_false_samples.py --help
python balance_count.py --help
```

#### Examples

* **Count samples per class**

  ```bash
  python count_class.py \
    --data-dir /path/to/images \
    --output class_counts.csv
  ```

* **Remove null annotations**

  ```bash
  python delete_null.py \
    --annotations /path/to/annotations.json \
    --output cleaned_annotations.json
  ```

* **Remap labels**

  ```bash
  python remap_labels.py \
    --input labels.csv \
    --mapping label_map.yaml \
    --output remapped_labels.csv
  ```

* **Run inference test**

  ```bash
  python inference_test.py \
    --model checkpoint.pt \
    --input images/ \
    --output results.json
  ```

* **Open training notebook**

  ```bash
  jupyter lab train.ipynb
  ```

---

## Dependencies

Managed via `requirements.txt` (create one if missing):

```
numpy
pandas
argparse
torch
torchvision
scikit-learn
pyyaml
opencv-python
jupyter
```

Install with:

```bash
pip install -r requirements.txt
```

---

## License

This code is released under the terms of the [LICENSE](../LICENSE) file in the project root.
