#include "esp_camera.h"
#include <WiFi.h>
#include <PubSubClient.h>

// =======================
// Konfigurasi WiFi & MQTT
// =======================
const char *ssid = "zadhaa";
const char *password = "piliang321";

const char *mqtt_broker = "broker.emqx.io";
const char *mqtt_topic = "OpenCV-IoT6601";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Pin Configuration
const int relayPin = 2;   // GPIO2 untuk relay
const int ledPin = 4;     // GPIO4 untuk LED indicator (opsional)

// Status Variables
bool relayStatus = false;
unsigned long lastReconnectAttempt = 0;
unsigned long lastMqttMessage = 0;

// =======================
// Pilih Model Kamera
// =======================
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// =======================
// Deklarasi Fungsi
// =======================
void startCameraServer();
void setupLedFlash(int pin);
void connectToWiFi();
void connectToMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();
void controlRelay(bool state);

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("=== ESP32-CAM Face Recognition System ===");

  // Setup Pins
  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(relayPin, LOW);   // Relay OFF saat startup
  digitalWrite(ledPin, LOW);     // LED OFF saat startup
  
  Serial.println("[INFO] Pin GPIO configured");

  // Setup MQTT
  mqttClient.setServer(mqtt_broker, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setKeepAlive(60);
  
  // Connect to WiFi
  connectToWiFi();
  
  // Connect to MQTT
  connectToMQTT();

  // Konfigurasi Kamera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // Optimasi berdasarkan PSRAM
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
      Serial.println("[INFO] PSRAM found - using optimized settings");
    } else {
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
      Serial.println("[INFO] PSRAM not found - using DRAM");
    }
  } else {
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // Inisialisasi Kamera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[ERROR] Camera init failed with error 0x%x\n", err);
    ESP.restart(); // Restart jika kamera gagal
  }
  Serial.println("[INFO] Camera initialized successfully");

  // Sensor settings
  sensor_t *s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // Flip vertical
    s->set_brightness(s, 1);   // Brightness
    s->set_saturation(s, -2);  // Saturation
  }

  // Set frame size untuk capture
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA); // 320x240 untuk performa yang baik
  }

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

  // Setup LED Flash jika tersedia
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  // Start Camera Server
  startCameraServer();

  Serial.print("[INFO] Camera server started!");
  Serial.print(" Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
  Serial.print("[INFO] Capture URL: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/capture");
  Serial.println("[INFO] System ready - waiting for face recognition commands...");
}

void loop() {
  // Maintain MQTT connection
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WARNING] WiFi disconnected - reconnecting...");
    connectToWiFi();
  }

  delay(100); // Reduced delay for better responsiveness
}

// =======================
// Implementasi Fungsi
// =======================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("[MQTT] Message received on topic '");
  Serial.print(topic);
  Serial.print("': ");
  Serial.println(msg);

  lastMqttMessage = millis();

  // Control relay based on message
  if (msg == "1") {
    controlRelay(true);
    Serial.println("[INFO] Face recognized - Opening solenoid");
  } else if (msg == "0") {
    controlRelay(false);
    Serial.println("[INFO] Face not recognized - Closing solenoid");
  } else {
    Serial.println("[WARNING] Unknown MQTT command: " + msg);
  }
}

void controlRelay(bool state) {
  if (relayStatus != state) {
    relayStatus = state;
    digitalWrite(relayPin, state ? HIGH : LOW);
    digitalWrite(ledPin, state ? HIGH : LOW); // LED indicator
    
    Serial.print("[RELAY] Status changed to: ");
    Serial.println(state ? "ON (Solenoid Open)" : "OFF (Solenoid Closed)");
  }
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("[WiFi] Connecting to ");
  Serial.print(ssid);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("[WiFi] Connected successfully!");
    Serial.print("[WiFi] IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("[WiFi] Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println();
    Serial.println("[ERROR] WiFi connection failed!");
    ESP.restart();
  }
}

void connectToMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("[MQTT] Attempting connection to ");
    Serial.print(mqtt_broker);
    Serial.print(":");
    Serial.print(mqtt_port);
    Serial.print("...");
    
    // Generate unique client ID
    String clientId = "ESP32CAM_" + String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println(" connected!");
      Serial.print("[MQTT] Client ID: ");
      Serial.println(clientId);
      
      // Subscribe to topic
      mqttClient.subscribe(mqtt_topic);
      Serial.print("[MQTT] Subscribed to topic: ");
      Serial.println(mqtt_topic);
      
      // Send online status
      mqttClient.publish((String(mqtt_topic) + "/status").c_str(), "online");
      
    } else {
      Serial.print(" failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void reconnectMQTT() {
  unsigned long now = millis();
  if (now - lastReconnectAttempt > 5000) {
    lastReconnectAttempt = now;
    
    if (!mqttClient.connected()) {
      Serial.println("[MQTT] Connection lost - attempting reconnection...");
      connectToMQTT();
    }
  }
}