import os
import shutil

source_dir = ""
dst_dir = ""

def add_false_samples(false_images_folder, dataset_folder, split='train', limit=1):
    """
    Add false samples with empty label files to your dataset
    """
    images_dest = os.path.join(dataset_folder, split, 'images')
    labels_dest = os.path.join(dataset_folder, split, 'labels')

    count = 0
    
    # Copy false images
    for img_name in os.listdir(false_images_folder):
        if count >= limit:
            break
        count += 1
        if img_name.lower().endswith(('.jpg', '.jpeg', '.png')):
            # Copy image
            src_img = os.path.join(false_images_folder, img_name)
            dst_img = os.path.join(images_dest, img_name)
            shutil.copy2(src_img, dst_img)
            
            # Create empty label file
            label_name = os.path.splitext(img_name)[0] + '.txt'
            label_path = os.path.join(labels_dest, label_name)
            
            # Create empty label file (just touch the file)
            open(label_path, 'w').close()
            
    print(f"Added {len(os.listdir(false_images_folder))} false samples to {split}")

# Usage
add_false_samples(source_dir, dst_dir, 'train', 300)
add_false_samples(source_dir, dst_dir, 'valid', 100)
add_false_samples(source_dir, dst_dir, 'test', 100)