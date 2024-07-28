#include <WiFi.h>
#include "esp_camera.h"
#include <HTTPClient.h>

// Credenciales WiFi
const char* ssid = "Azalia";
const char* password = "queonda123";

const char* serverName = "http://192.168.196.108:5000/upload"; 


#define CAMERA_MODEL_AI_THINKER


const int sensorPin = 2; 
const int ledPin = 4;    

bool wasDoorOpen = false; 

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Espera a que se conecte a la red WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado a WiFi");

  setupCamera();
  
  pinMode(sensorPin, INPUT_PULLUP); // Configura el pin del sensor como entrada con resistencia de pull-up interna
  pinMode(ledPin, OUTPUT);         // Configura el pin del LED como salida
}

void loop() {
  int sensorState = digitalRead(sensorPin); // Lee el estado del pin del sensor

  Serial.print("Estado del sensor: ");
  Serial.println(sensorState); // Imprime el valor del sensor para depuración

  if (sensorState == LOW) { // Si el sensor está activado (puerta abierta)
    digitalWrite(ledPin, LOW); // Apaga el LED
    if (!wasDoorOpen) { // Solo captura la imagen si el estado anterior era cerrado
      Serial.println("Puerta Abierta");
      captureAndSendImage(); // Captura y envía la imagen
      wasDoorOpen = true; // Actualiza el estado de la puerta
    }
  } else {
    digitalWrite(ledPin, HIGH); // Enciende el LED
    if (wasDoorOpen) { // Solo actualiza el estado si la puerta estaba abierta
      Serial.println("Puerta Cerrada");
      wasDoorOpen = false; // Actualiza el estado de la puerta
    }
  }
}

void captureAndSendImage() {
  Serial.println("Capturando imagen...");
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Error al capturar la imagen");
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW"; // Se puede generar un valor único
    String contentType = "multipart/form-data; boundary=" + boundary;
    

    String postData = "--" + boundary + "\r\n";
    postData += "Content-Disposition: form-data; name=\"file\"; filename=\"captured_image.jpg\"\r\n";
    postData += "Content-Type: image/jpeg\r\n\r\n";
    String endData = "\r\n--" + boundary + "--\r\n";
    
    int postDataLength = postData.length() + fb->len + endData.length();
    
    http.begin(serverName);
    http.addHeader("Content-Type", contentType);
    http.addHeader("Content-Length", String(postDataLength));
    
    // Crear buffer para la solicitud POST
    uint8_t* buffer = new uint8_t[postDataLength];
    memcpy(buffer, postData.c_str(), postData.length());
    memcpy(buffer + postData.length(), fb->buf, fb->len);
    memcpy(buffer + postData.length() + fb->len, endData.c_str(), endData.length());
    
    int httpResponseCode = http.sendRequest("POST", buffer, postDataLength);
    
    delete[] buffer;

    if (httpResponseCode > 0) {
      Serial.println("Foto enviada con éxito");
    } else {
      Serial.printf("Error al enviar la foto. Código de respuesta: %d\n", httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("No conectado a WiFi");
  }

  esp_camera_fb_return(fb);

  delay(1000); 
}

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sccb_sda = 26;
  config.pin_sccb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_SVGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_DRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error al iniciar la cámara: 0x%x\n", err);
    return;
  }
}
