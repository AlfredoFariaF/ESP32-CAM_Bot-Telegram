#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"

// ==========================================
// 1. CONFIGURAÇÕES DE REDE E TELEGRAM
// ==========================================
const char* ssid = "*******";
const char* password = "*******";

String botToken = "*******"; // Ex: "123456789:ABCdefGhIJKlmNoPQRsTUVWxyz"
String chatId = "*******";        // Ex: "1234567890"

// ==========================================
// 2. CONFIGURAÇÕES DO TEMPORIZADOR
// ==========================================
unsigned long lastTime = 0;
// 30000 milissegundos = 30 segundos
unsigned long timerDelay = 30000; // 30 segundos

// Mude para 'false' se não quiser usar o flash
bool useFlash = true; 

#define FLASH_LED_PIN 4 // O LED principal do ESP32-CAM fica no pino 4

// ==========================================
// 3. PINOS DA CÂMERA (Modelo AI-THINKER)
// ==========================================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void configCamera() {
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
  config.pixel_format = PIXFORMAT_JPEG;

  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Falha na inicializacao da camera: 0x%x", err);
    delay(1000);
    ESP.restart();
  }
}

void sendPhotoToTelegram() {
  const char* myDomain = "api.telegram.org";
  String getAll="";
  String getBody="";

  // Verifica a variável de configuração do flash
  if (useFlash) {
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(50); // Aguarda o sensor ajustar a exposição à nova luz
  }

  // Tira a foto
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  
  // Desliga o flash de forma segura, garantindo que não fique aceso
  digitalWrite(FLASH_LED_PIN, LOW);

  if(!fb) {
    Serial.println("Falha ao capturar imagem da camera");
    return;
  }

  Serial.println("Conectando ao Telegram para envio...");
  WiFiClientSecure client;
  client.setInsecure();

  if (client.connect(myDomain, 443)) {
    Serial.println("Conectado! Enviando foto...");
    
    String head = "--Boundary\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + chatId + "\r\n--Boundary\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--Boundary--\r\n";

    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;
  
    client.println("POST /bot" + botToken + "/sendPhoto HTTP/1.1");
    client.println("Host: " + String(myDomain));
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=Boundary");
    client.println();
    client.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0; n<fbLen; n=n+1024) {
      if (n+1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client.write(fbBuf, remainder);
      }
    }   
    client.print(tail);
    
    long startTimer = millis();
    boolean state = false;
    while ((startTimer + 5000) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        if (state==true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } else if (c != '\r') {
          getAll += String(c);
        }
      }
      if (getBody.length()>0) { break; }
    }
    Serial.println();
    Serial.println("Foto enviada com sucesso!");
    client.stop();
  } else {
    Serial.println("Erro ao conectar no Telegram.");
  }
  
  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);
  
  // Configura o pino do flash e garante que comece desligado
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW); 

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");

  configCamera();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    sendPhotoToTelegram();
    lastTime = millis();
  }
}