#include "WiFi.h" 
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <FS.h>
#include "SD_MMC.h"
#include <EEPROM.h>            

const char* ssid = "Cookies Wifi";
const char* password = "Cenojas080569";

AsyncWebServer server(80);
boolean takeNewPhoto = false;

#define FILE_PHOTO "/photo.jpg"
#define EEPROM_SIZE 1
int pictureNumber = 0;

// AI Thinker CAM PIN Map
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

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta http-equiv="refresh" content="10">
  <style>
    body {
      background-color: #a8c209;
      color: white;
      font-family: Arial, sans-serif;
      text-align: center;
      margin: 0;
      padding: 0;
    }
    h1 {
      font-size: 2.5em;
      font-weight: bold;
      margin-top: 30px;
    }
    button {
      font-size: 16px;
      padding: 10px 20px;
      margin: 10px;
      border: none;
      border-radius: 5px;
      background-color: #ffffff;
      color: #000080;
      font-weight: bold;
      cursor: pointer;
      transition: 0.3s;
    }
    button:hover {
      background-color: #dcdcdc;
    }
    img {
      margin-top: 20px;
      max-width: 80%;
      border: 4px solid white;
      border-radius: 10px;
    }
    #log {
      margin-top: 20px;
      font-size: 1em;
    }
  </style>
</head>
<body>
  <h1>MAACM Satellite Payload</h1>
  <p>Capture mission photos and auto-refresh every 10 seconds.</p>
  <button onclick="rotatePhoto();">ROTATE</button>
  <button onclick="capturePhoto()">CAPTURE PHOTO</button>
  <button onclick="location.reload();">REFRESH PAGE</button>
  <div>
    <img src="saved-photo?t=" id="photo">
  </div>
  <div id="log">Loading latest mission info...</div>
</body>
<script>
  var deg = 0;
  function capturePhoto() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', "/capture", true);
    xhr.onload = function() {
      setTimeout(function() {
        var photo = document.getElementById("photo");
        photo.src = "saved-photo?t=" + new Date().getTime();
        fetchLog();
      }, 3000);
    };
    xhr.send();
  }
  function rotatePhoto() {
    var img = document.getElementById("photo");
    deg += 90;
    img.style.transform = "rotate(" + deg + "deg)";
  }
  function fetchLog() {
    fetch('/latest-info')
      .then(response => response.text())
      .then(data => {
        document.getElementById("log").innerHTML = data;
      });
  }
  fetchLog(); // initial fetch
</script>
</html>)rawliteral";

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  pinMode(13,INPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    ESP.restart();
  } else {
    Serial.println("SPIFFS mounted");
  }

  if (!SD_MMC.begin()) {
    Serial.println("SD Card Mount Failed");
  } else {
    if (SD_MMC.cardType() == CARD_NONE) {
      Serial.println("No SD Card");
    } else {
      Serial.println("SD Card initialized.");
    }
  }

  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;

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

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    ESP.restart();
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/html", index_html);
  });

server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
  takeNewPhoto = true;
  takePhotoToSD(); // <-- Add this line to save to SD card
  request->send(200, "text/plain", "Taking Photo");
});

  server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, FILE_PHOTO, "image/jpg", false);
  });

  server.on("/latest-info", HTTP_GET, [](AsyncWebServerRequest *request){
    String info = "Filename: MissionA" + String(pictureNumber - 1) + ".jpg<br>";
    info += "Timestamp: " + String(millis() / 1000) + "s since boot";
    request->send(200, "text/html", info);
  });

  server.begin();

  Serial.println("Press P to take a photo and save to SD card and show in browser.");
}

void loop() {
  if (takeNewPhoto) {
    capturePhotoSaveSpiffs();
    takeNewPhoto = false;
  }

  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'P' || command == 'p') {
      Serial.println("Taking photo to SD card...");
      takePhotoToSD();
      capturePhotoSaveSpiffs(); // also save for web view
    }
  }

  if(digitalRead(13) == HIGH){
      Serial.println("Taking photo to SD card...");
      takePhotoToSD();
      capturePhotoSaveSpiffs(); // also save for web 
    }
  delay(1);
}

void capturePhotoSaveSpiffs() {
  camera_fb_t * fb = NULL;
  bool ok = 0;

  do {
    Serial.println("Taking photo for SPIFFS...");
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    } else {
      file.write(fb->buf, fb->len);
      Serial.print("Photo saved to SPIFFS (");
      Serial.print(file.size());
      Serial.println(" bytes)");
    }
    file.close();
    esp_camera_fb_return(fb);
    ok = checkPhoto(SPIFFS);
  } while (!ok);
}

bool checkPhoto(fs::FS &fs) {
  File f_pic = fs.open(FILE_PHOTO);
  return f_pic && f_pic.size() > 100;
}

void takePhotoToSD() {
  if (!SD_MMC.begin()) {
    Serial.println("SD Card Mount Failed");
    return;
  }

  if (SD_MMC.cardType() == CARD_NONE) {
    Serial.println("No SD Card");
    return;
  }

  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  String path = "/MissionA" + String(pictureNumber) + ".jpg";
  fs::FS &fs = SD_MMC;

  File file = fs.open(path.c_str(), FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write(fb->buf, fb->len);
    Serial.printf("Saved photo to: %s\n", path.c_str());
    pictureNumber++;
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  file.close();
  esp_camera_fb_return(fb);
}
