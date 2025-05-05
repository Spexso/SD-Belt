import asyncio
import websockets
import cv2
import base64

class WebSocketCameraServer:
    def __init__(self):
        self.cap = cv2.VideoCapture(0, cv2.CAP_MSMF)
        if not self.cap.isOpened():
            print("[FATAL] Kamera açılamadı.")
            exit(1)
        print("[OK] Kamera açıldı.")
        self.latest_frame = None

    async def capture_loop(self):
        while True:
            ret, frame = self.cap.read()
            if ret:
                self.latest_frame = frame
            await asyncio.sleep(0.01)

    async def handler(self, websocket, path=None):  # path opsiyonel yapıldı
        print(f"[INFO] İstemci bağlandı (path: {path})")
        try:
            while True:
                if self.latest_frame is not None:
                    _, buffer = cv2.imencode('.jpg', self.latest_frame)
                    b64 = base64.b64encode(buffer).decode('utf-8')
                    await websocket.send(b64)
                await asyncio.sleep(0.03)
        except websockets.ConnectionClosed:
            print("[INFO] Bağlantı kesildi.")

    async def run(self):
        asyncio.create_task(self.capture_loop())
        print("[SUNUCU] ws://localhost:8765 dinleniyor...")

        # 1. Çözüm: partial kullanımı
        from functools import partial
        handler_with_path = partial(self.handler, path=None)
        
        # 2. Çözüm (alternatif): lambda ile her iki parametreyi de iletme
        # handler_with_path = lambda ws, path: self.handler(ws, path)
        
        async with websockets.serve(handler_with_path, "0.0.0.0", 8765):
            await asyncio.Future()

if __name__ == "__main__":
    try:
        server = WebSocketCameraServer()
        asyncio.run(server.run())
    except KeyboardInterrupt:
        print("\n[ÇIKIŞ] Kamera kapatıldı.")
        server.cap.release()