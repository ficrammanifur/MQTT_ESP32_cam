import cv2
import face_recognition
import numpy as np
import os
import urllib.request
import paho.mqtt.client as mqtt
import time

# URL dari ESP32-CAM
ESP32_URL = "http://192.168.32.45/capture"

# Folder wajah yang dikenal
KNOWN_FACES_DIR = "known_faces"

known_face_encodings = []
known_face_names = []

# MQTT Setup
broker = "broker.emqx.io"
topic = "OpenCV-IoT6601"
client = mqtt.Client()

# Callback untuk koneksi MQTT
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("[INFO] Terhubung ke MQTT broker")
    else:
        print(f"[ERROR] Gagal terhubung ke MQTT broker, kode: {rc}")

def on_publish(client, userdata, mid):
    print(f"[INFO] Pesan MQTT terkirim dengan ID: {mid}")

client.on_connect = on_connect
client.on_publish = on_publish

try:
    client.connect(broker, 1883, 60)
    client.loop_start()  # Mulai loop MQTT di background
    print("[INFO] Menghubungkan ke MQTT broker...")
    time.sleep(2)  # Beri waktu untuk koneksi
except Exception as e:
    print(f"[ERROR] Gagal menghubungkan ke MQTT: {e}")

# Load semua wajah dari folder known_faces
if not os.path.exists(KNOWN_FACES_DIR):
    print(f"[ERROR] Folder {KNOWN_FACES_DIR} tidak ditemukan!")
    print("Buat folder 'known_faces' dan masukkan foto wajah yang akan dikenali")
    exit()

for filename in os.listdir(KNOWN_FACES_DIR):
    if filename.lower().endswith(('.jpg', '.jpeg', '.png')):
        image_path = os.path.join(KNOWN_FACES_DIR, filename)
        try:
            image = face_recognition.load_image_file(image_path)
            encoding = face_recognition.face_encodings(image)
            if encoding:
                known_face_encodings.append(encoding[0])
                known_face_names.append(os.path.splitext(filename)[0])
                print(f"[INFO] Loaded face: {filename}")
            else:
                print(f"[WARNING] Tidak ada wajah yang terdeteksi di {filename}")
        except Exception as e:
            print(f"[ERROR] Gagal memuat {filename}: {e}")

if len(known_face_encodings) == 0:
    print("[ERROR] Tidak ada wajah yang berhasil dimuat!")
    print("Pastikan file gambar di folder 'known_faces' mengandung wajah yang jelas")
    exit()

print(f"[INFO] Berhasil memuat {len(known_face_encodings)} wajah yang dikenal.")

# Variabel untuk mencegah spam MQTT
last_mqtt_message = ""
last_mqtt_time = 0
mqtt_cooldown = 2  # Cooldown 2 detik antar pesan MQTT

print("[INFO] Memulai pengenalan wajah...")
print("Tekan ESC untuk keluar")

while True:
    # Ambil gambar dari ESP32-CAM
    try:
        img_resp = urllib.request.urlopen(ESP32_URL, timeout=5)
        img_np = np.array(bytearray(img_resp.read()), dtype=np.uint8)
        frame = cv2.imdecode(img_np, -1)
        
        if frame is None:
            print("[ERROR] Frame kosong dari ESP32-CAM")
            continue
            
    except Exception as e:
        print(f"[ERROR] Tidak bisa mengambil gambar dari ESP32-CAM: {e}")
        time.sleep(1)
        continue

    # Resize frame agar tampilannya lebih kecil dan proses lebih cepat
    scale_percent = 50  # Dikurangi untuk performa yang lebih baik
    width = int(frame.shape[1] * scale_percent / 100)
    height = int(frame.shape[0] * scale_percent / 100)
    frame_small = cv2.resize(frame, (width, height))

    # Ubah ke RGB untuk face_recognition
    rgb_frame = cv2.cvtColor(frame_small, cv2.COLOR_BGR2RGB)

    # Deteksi wajah
    face_locations = face_recognition.face_locations(rgb_frame)
    face_encodings = face_recognition.face_encodings(rgb_frame, face_locations)

    # Status untuk MQTT
    face_recognized = False
    recognized_name = "Tidak dikenal"

    # Proses setiap wajah yang terdeteksi
    for (top, right, bottom, left), face_encoding in zip(face_locations, face_encodings):
        matches = face_recognition.compare_faces(known_face_encodings, face_encoding, tolerance=0.6)
        name = "Tidak dikenal"
        confidence = "Unknown"

        # Hitung jarak wajah untuk mendapatkan yang paling cocok
        face_distances = face_recognition.face_distance(known_face_encodings, face_encoding)
        
        if len(face_distances) > 0:
            best_match_index = np.argmin(face_distances)
            if matches[best_match_index] and face_distances[best_match_index] < 0.6:
                name = known_face_names[best_match_index]
                confidence = f"{round((1-face_distances[best_match_index])*100, 1)}%"
                face_recognized = True
                recognized_name = name

        # Gambar kotak dan nama di frame yang sudah di-resize
        color = (0, 255, 0) if face_recognized else (0, 0, 255)
        cv2.rectangle(frame_small, (left, top), (right, bottom), color, 2)
        
        # Label dengan nama dan confidence
        label = f"{name}"
        if confidence != "Unknown":
            label += f" ({confidence})"
            
        cv2.putText(frame_small, label, (left, bottom + 20), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)

    # Kirim pesan MQTT berdasarkan hasil pengenalan
    current_time = time.time()
    mqtt_message = "1" if face_recognized else "0"
    
    # Kirim MQTT hanya jika ada perubahan status atau sudah lewat cooldown
    if (mqtt_message != last_mqtt_message or 
        current_time - last_mqtt_time > mqtt_cooldown):
        
        try:
            result = client.publish(topic, mqtt_message)
            
            if face_recognized:
                print(f"[INFO] Wajah dikenali: {recognized_name} - Mengirim '1' (Buka Relay)")
            else:
                if len(face_locations) > 0:
                    print("[INFO] Wajah tidak dikenal - Mengirim '0' (Tutup Relay)")
                else:
                    print("[INFO] Tidak ada wajah terdeteksi - Mengirim '0' (Tutup Relay)")
            
            last_mqtt_message = mqtt_message
            last_mqtt_time = current_time
            
        except Exception as e:
            print(f"[ERROR] Gagal mengirim MQTT: {e}")

    # Tampilkan status di frame
    status_text = f"Status: {'DIKENAL' if face_recognized else 'TIDAK DIKENAL'}"
    relay_text = f"Relay: {'ON' if face_recognized else 'OFF'}"
    
    cv2.putText(frame_small, status_text, (10, 30), 
               cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0) if face_recognized else (0, 0, 255), 2)
    cv2.putText(frame_small, relay_text, (10, 60), 
               cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0) if face_recognized else (0, 0, 255), 2)

    # Tampilkan frame
    cv2.imshow("ESP32-CAM Face Recognition", frame_small)

    # Keluar jika ESC ditekan
    if cv2.waitKey(1) & 0xFF == 27:  # ESC
        break

# Cleanup
print("[INFO] Menutup aplikasi...")
client.loop_stop()
client.disconnect()
cv2.destroyAllWindows()
print("[INFO] Aplikasi ditutup")