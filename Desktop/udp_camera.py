import cv2
import socket
import struct
import time

# ==== Configuration ====
CAM_ID = 0                           # camera ID sent to client
QT_CLIENT_IP = '127.0.0.1'          # or your Qt receiver's IP
QT_CLIENT_PORT = 5000
JPEG_QUALITY = 80

# ==== Setup ====
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), JPEG_QUALITY]

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Encode as JPEG
    result, encimg = cv2.imencode('.jpg', frame, encode_param)
    if not result:
        continue

    jpeg_bytes = encimg.tobytes()

    # Skip frames that are too large
    if len(jpeg_bytes) > 60000:
        print(f"Frame too big ({len(jpeg_bytes)} bytes), skipping.")
        continue

    # Header: 1 byte camId + 4 bytes big-endian length
    header = struct.pack('>BI', CAM_ID, len(jpeg_bytes))
    packet = header + jpeg_bytes

    sock.sendto(packet, (QT_CLIENT_IP, QT_CLIENT_PORT))

    # Optional: slow down to avoid flooding
    time.sleep(1 / 30)  # ~30 FPS

cap.release()
sock.close()
