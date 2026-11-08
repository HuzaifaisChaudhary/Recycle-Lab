import socket
import os
import time
from http.server import BaseHTTPRequestHandler, HTTPServer
from ultralytics import YOLO

# Path to the trained YOLOv8 model (relative to this file)
MODEL_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                          "..", "..", "models", "recycle_yolov8n.pt")

# ================= CONFIGURATION =================
# 1. ESP8266 SETTINGS (Must match your Arduino Code)
ESP_IP = "192.168.43.122"  
ESP_PORT = 80

# 2. SERVER SETTINGS (Where this script listens for images)
HOST_IP = "0.0.0.0"       
HOST_PORT = 8000          

# 3. CLASS MAPPING (YOLO Class -> ESP Command)
# Sends '1' for Paper, '2' for Plastic, '3' for Metal
CLASS_MAP = {
    "paper": "1",
    "plastic": "2",
    "metal": "3"
}

# 4. SETUP
SAVE_FOLDER = "received_images"
os.makedirs(SAVE_FOLDER, exist_ok=True)

print("⏳ Loading YOLO model...")
try:
    model = YOLO(MODEL_PATH)
    print("✅ Model loaded!")
except Exception as e:
    print(f"❌ Error: Could not load '{MODEL_PATH}'. {e}")
    exit()

# ================= FUNCTION TO TALK TO ESP =================
def trigger_esp_light(command_char):
    """
    Opens a socket to the ESP, sends one character, and disconnects.
    """
    print(f"📡 Sending '{command_char}' to ESP ({ESP_IP})...")
    
    try:
        # 1. Create Socket
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.settimeout(3) # 3 second timeout
        
        # 2. Connect
        client.connect((ESP_IP, ESP_PORT))
        
        # 3. Send
        client.send(command_char.encode())
        
        # 4. Close
        client.close()
        print("✅ Command sent successfully.")

        # --- ADDED DELAY HERE ---
        print("⏳ Waiting 10 seconds for hardware...")
        time.sleep(10)
        print("▶️ Done waiting.")
        # ------------------------

        return True
    except Exception as e:
        print(f"⚠️ Failed to connect to ESP: {e}")
        return False

# ================= HTTP HANDLER =================
class RequestHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        if self.path == "/upload":
            try:
                # 1. Receive Image
                content_length = int(self.headers['Content-Length'])
                file_data = self.rfile.read(content_length)
                
                filename = time.strftime("%Y%m%d-%H%M%S") + ".jpg"
                filepath = os.path.join(SAVE_FOLDER, filename)
                
                with open(filepath, "wb") as f:
                    f.write(file_data)
                
                print(f"\n📸 New Image: {filename}")

                # 2. Classify Image
                results = model.predict(filepath, verbose=False)
                result = results[0]

                if result.probs is not None:
                    # Classification model: take the top class
                    top_idx = result.probs.top1
                    confidence = round(float(result.probs.top1conf * 100), 2)
                elif result.boxes is not None and len(result.boxes) > 0:
                    # Detection model: take the most confident box
                    best = int(result.boxes.conf.argmax())
                    top_idx = int(result.boxes.cls[best])
                    confidence = round(float(result.boxes.conf[best] * 100), 2)
                else:
                    # Nothing on the belt in this frame
                    print("🔍 No item detected.")
                    self.send_response(200)
                    self.send_header("Content-type", "text/plain")
                    self.end_headers()
                    self.wfile.write(b"No item detected")
                    return

                class_name = result.names[top_idx] # e.g., "plastic"
                print(f"🧠 AI Detected: {class_name.upper()} ({confidence}%)")

                # 3. Send to ESP
                # Convert "plastic" -> "2"
                command = CLASS_MAP.get(class_name.lower())
                
                if command:
                    trigger_esp_light(command)
                    status_msg = f"Classified {class_name}, Sent {command}"
                else:
                    print(f"⚠️ Unknown class '{class_name}'. No signal sent.")
                    status_msg = f"Classified {class_name}, No Signal Sent"

                # 4. Reply to Sender
                self.send_response(200)
                self.send_header("Content-type", "text/plain")
                self.end_headers()
                self.wfile.write(status_msg.encode())

            except Exception as e:
                print(f"Error: {e}")
                self.send_response(500)
                self.end_headers()
        else:
            self.send_response(404)
            self.end_headers()

# ================= START SERVER =================
if __name__ == "__main__":
    print(f"🚀 Server running at http://{HOST_IP}:{HOST_PORT}")
    server = HTTPServer((HOST_IP, HOST_PORT), RequestHandler)
    server.serve_forever()