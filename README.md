# 📷 MQTT_ESP32_cam

Proyek ini merupakan integrasi antara **ESP32-CAM** dan **MQTT** untuk sistem pengiriman data video/gambar serta monitoring menggunakan **Python + OpenCV**.

---

## 🚀 Fitur Utama

- 📡 Komunikasi **ESP32-CAM** dengan MQTT broker.
- 📷 Pengambilan gambar otomatis berdasarkan perintah atau kondisi tertentu.
- 👤 Deteksi wajah dari sisi klien menggunakan **OpenCV** dan **face_recognition**.
- 🔍 Monitoring real-time melalui Python script.
- 🔧 Struktur folder terpisah untuk kode ESP32 dan aplikasi klien.

---

## 🗂️ Struktur Proyek

MQTT_ESP32_cam/
├── MQTT_ESP32_cam/ # Firmware untuk ESP32-CAM
│ ├── MQTT_ESP32_cam.ino
│ ├── app_httpd.cpp
│ ├── camera_index.h
│ ├── camera_pins.h
│ ├── ci.json
│ └── partitions.csv
│
└── Python_OpenCV/ # Klien Python untuk deteksi wajah
├── main.py
├── known_faces/
│ ├── Pace.jpg
│ ├── Riski.jpg
│ ├── Ucin.jpg
│ └── azza.jpg
└── .idea/ # Konfigurasi IDE (opsional)

---

## 🔧 Tools & Library

### Hardware:
- ESP32-CAM
- Breadboard, sensor (opsional)

### Software:
- [Arduino IDE](https://www.arduino.cc/en/software)
- [Mosquitto MQTT Broker](https://mosquitto.org/)
- Python 3.10+
  - `opencv-python`
  - `paho-mqtt`
  - `face_recognition`
  - `numpy`

---

## 🧪 Cara Menjalankan

### 1. Upload ke ESP32-CAM
- Buka `MQTT_ESP32_cam.ino` di Arduino IDE.
- Pastikan library dan board sudah terpasang.
- Edit WiFi dan MQTT broker info:
  ```cpp
  const char* ssid = "WIFI_KAMU";
  const char* password = "PASSWORD_WIFI";
  const char* mqtt_server = "192.168.x.x";
Upload sketch.

2. Jalankan Python Klien
bash
Copy
Edit
cd Python_OpenCV
pip install opencv-python paho-mqtt face_recognition
python main.py
⚠️ Catatan
Folder .idea/ bisa diabaikan (file internal dari PyCharm/IDE).

Gambar di known_faces/ digunakan untuk proses deteksi wajah.

Gunakan .gitignore untuk mengecualikan file IDE dan cache.

🙋 Kontributor
👨‍💻 @ficrammanifur

📄 Lisensi
Proyek ini bersifat open-source dan dapat digunakan untuk keperluan pembelajaran, pengembangan, dan riset tanpa batasan.
