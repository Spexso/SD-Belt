from ultralytics import YOLO
import cv2, os


model_path = 'model.pt'  # Path to your YOLOv8 model file
samples_dir = 'test_images_old'  # Directory containing test images

# 1. Load your model
model = YOLO(model_path)

print(model.names)

# 2. Paths
test_folder   = samples_dir
output_folder = '/'
os.makedirs(output_folder, exist_ok=True)

# 3. Inference loop
for img_name in os.listdir(test_folder):
    img_path = os.path.join(test_folder, img_name)
    if not img_path.lower().endswith(('.jpg','.jpeg','.png')):
        continue

    # Run inference
    results = model.predict(
        source=img_path,
        conf=0.4,      # set to 0 so you see _all_ confidences
        save=True,
        iou=0.45,
        save_dir=output_folder
    )

    # results is a list, one element per image (here always len=1)
    r = results[0]

    print(f"\n=== {img_name} ===")
    # r.boxes is a Boxes object; each box has .cls (int), .conf (float), .xyxy
    for i, box in enumerate(r.boxes):
        cls_id   = int(box.cls)                     # numeric class
        cls_name = model.names[cls_id]              # human-readable
        conf     = float(box.conf)                  # confidence score
        x1, y1, x2, y2 = box.xyxy[0].tolist()       # bounding-box coords

        print(f"  Det #{i+1}: {cls_name:12s}  conf={conf:.3f}  box=[{x1:.1f},{y1:.1f},{x2:.1f},{y2:.1f}]")

    print(f"Annotated image saved to {output_folder}/{img_name}")

print("\nDone!")
