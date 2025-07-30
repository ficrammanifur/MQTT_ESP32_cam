<h1 align="center">📷 MQTT_ESP32_cam</h1>

<p align="center">
  <img src="https://img.shields.io/badge/last%20commit-today-brightgreen" />
  <img src="https://img.shields.io/badge/language-C%2B%2B%20%7C%20Python-blue" />
  <img src="https://img.shields.io/badge/platform-ESP32--CAM-informational" />
  <img src="https://img.shields.io/badge/protocol-MQTT-green" />
  <img src="https://img.shields.io/badge/computer_vision-OpenCV-orange" />
  <img src="https://img.shields.io/badge/face_detection-face__recognition-yellow" />
</p>

<p align="center">
  Prototipe robot yang meniru gerakan tangan manusia secara real-time menggunakan Mediapipe dan ESP32
</p>


---
## 🚀 Fitur Utama

- 📡 Komunikasi **ESP32-CAM** dengan MQTT broker.
- 📷 Pengambilan gambar otomatis berdasarkan perintah atau kondisi tertentu.
- 👤 Deteksi wajah dari sisi klien menggunakan **OpenCV** dan **face_recognition**.
- 🔍 Monitoring real-time melalui Python script.
- 🔧 Struktur folder terpisah untuk kode ESP32 dan aplikasi klien.

---

## 📁 Project Structure

```text
MQTT_ESP32_cam/
├── MQTT_ESP32_cam/                # Firmware untuk ESP32-CAM
│   ├── MQTT_ESP32_cam.ino
│   ├── app_httpd.cpp
│   ├── camera_index.h
│   ├── camera_pins.h
│   ├── ci.json
│   └── partitions.csv
│
├── Python_OpenCV/                 # Klien Python untuk deteksi wajah
│   ├── main.py
│   ├── known_faces/
│   │   ├── Pace.jpg
│   │   ├── Riski.jpg
│   │   ├── Ucin.jpg
│   │   └── azza.jpg
│   └── .idea/                     # Konfigurasi IDE (opsional)
```

---

## 🔧 Tools & Library

### Hardware:
- **ESP32-CAM**
- **Breadboard, sensor (opsional)**

### Software:
- [Arduino IDE](https://www.arduino.cc/en/software)
- [Mosquitto MQTT Broker](https://mosquitto.org/)
- **Python 3.10+**
  - `opencv-python`
  - `paho-mqtt`
  - `face_recognition`
  - `numpy`

---

## 🧪 Cara Menjalankan

### 1. Upload ke ESP32-CAM

1. Buka `MQTT_ESP32_cam.ino` di Arduino IDE.
2. Pastikan library dan board sudah terpasang.
3. Edit WiFi dan MQTT broker info:

```cpp
const char* ssid = "WIFI_KAMU";
const char* password = "PASSWORD_WIFI";
const char* mqtt_server = "192.168.x.x";
```

4. Upload sketch.

### 2. Jalankan Python Klien

```bash
cd Python_OpenCV
pip install opencv-python paho-mqtt face_recognition numpy
python main.py
```

---

## 📋 Konfigurasi MQTT

### Broker Settings
- **Host**: `192.168.x.x` (sesuaikan dengan IP broker Anda)
- **Port**: `1883` (default MQTT)
- **Topics**:
  - `esp32cam/image` - untuk pengiriman gambar
  - `esp32cam/command` - untuk perintah kontrol
  - `esp32cam/status` - untuk status device

### ESP32-CAM Configuration

```cpp
// WiFi Configuration
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT Configuration
const char* mqtt_server = "YOUR_MQTT_BROKER_IP";
const int mqtt_port = 1883;
const char* mqtt_user = "username";     // opsional
const char* mqtt_password = "password"; // opsional
```

---

## 🎯 Cara Kerja Sistem

### 1. ESP32-CAM
- Menginisialisasi kamera dan koneksi WiFi
- Terhubung ke MQTT broker
- Mengambil gambar berdasarkan trigger atau perintah
- Mengirim data gambar melalui MQTT

### 2. Python Client
- Subscribe ke topic MQTT untuk menerima gambar
- Memproses gambar menggunakan OpenCV
- Melakukan deteksi wajah dengan `face_recognition`
- Menampilkan hasil deteksi secara real-time

### 3. Face Recognition
- Memuat gambar referensi dari folder `known_faces/`
- Membandingkan wajah yang terdeteksi dengan database
- Menampilkan nama jika wajah dikenali

---

## 🔍 Troubleshooting

### ESP32-CAM Tidak Terhubung
- Periksa koneksi WiFi dan kredensial
- Pastikan MQTT broker dapat diakses
- Cek Serial Monitor untuk pesan error

### Python Client Error
- Pastikan semua dependencies terinstall
- Periksa koneksi ke MQTT broker
- Verifikasi format gambar yang diterima

### Face Recognition Tidak Akurat
- Gunakan gambar referensi dengan kualitas baik
- Pastikan pencahayaan cukup saat pengambilan gambar
- Sesuaikan threshold deteksi jika diperlukan

---

## 📊 Performance Tips

- Gunakan resolusi gambar yang sesuai (tidak terlalu tinggi)
- Optimalkan interval pengiriman gambar
- Implementasi buffer untuk menghindari lag
- Gunakan kompresi JPEG untuk mengurangi ukuran data

---

## 🔮 Pengembangan Selanjutnya

- [ ] Implementasi enkripsi untuk keamanan data
- [ ] Web interface untuk monitoring
- [ ] Database logging untuk riwayat deteksi
- [ ] Multiple camera support
- [ ] Real-time streaming video
- [ ] Mobile app integration

---

## ⚠️ Catatan

- Folder `.idea/` bisa diabaikan (file internal dari PyCharm/IDE).
- Gambar di `known_faces/` digunakan untuk proses deteksi wajah.
- Gunakan `.gitignore` untuk mengecualikan file IDE dan cache.
- Pastikan ESP32-CAM memiliki power supply yang cukup.

---

## 🙋 Kontributor

👨‍💻 [@ficrammanifur](https://github.com/ficrammanifur)


<div align="center">

**⭐ Star this repository if you find it helpful!**

<p><a href="#top">⬆ Kembali ke Atas</a></p>

</div>
