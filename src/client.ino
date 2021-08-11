// Over The Air flashing
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
const char* ssid = "quackers";
const char* password = "ToesBlanket7Can";

// SPIFFS/Filesystem
#include <FS.h>
String video_dr = "/video_frames/"; // Base SPIFFs video directory
bool isDone;
File file;
FSInfo fs_info;

#include <FastLED.h>
#define SERPENTINE_LAYOUT true
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 32
#define NUM_LEDS 1024
// https://i2.wp.com/randomnerdtutorials.com/wp-content/uploads/2019/05/ESP8266-WeMos-D1-Mini-pinout-gpio-pin.png?quality=100&strip=all&ssl=1
#define DATA_PIN 4 // (D2) Green
#define CLOCK_PIN 5 // (D1) White
#define BRIGHTNESS 16
CRGB leds[NUM_LEDS];
#define MAX_FRAMES 194 // temp max frames, send this over serial later
int currentIncomingFrame = 0; // Keep track of how many frames we have recieved
int currentFrame = 0;


void setup() {
   // LED Setup 
  Serial.begin(921600);
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
  displayRGB(); 

  // SPIFFS / Filesystem setup
  SPIFFS.begin();

  // OTA Setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Check if an animation already exists
  isDone = SPIFFS.exists(video_dr + String(MAX_FRAMES));
}


char buf[NUM_LEDS * 3];
void loop() { 
  ArduinoOTA.handle();
  int startChar = Serial.read();
  // On start char read frame and save to SPIFFS
  if (startChar == '*') {
      isDone = false;
      // Read a frame from serial and store into SPIFFS
      Serial.readBytes(buf, NUM_LEDS * 3); 
      file = SPIFFS.open(
        video_dr + String(currentIncomingFrame), "w");
      file.write((uint8_t*)buf, NUM_LEDS * 3);
      file.close();
      SPIFFS.info(fs_info);
      Serial.println(fs_info.usedBytes);
      currentIncomingFrame++;
      if (currentIncomingFrame >= MAX_FRAMES){
        isDone = true;
      }
      
  }
  else if (startChar >= 0) {
    // discard unknown characters
  }
  if (isDone) {
    EVERY_N_MILLISECONDS(30) {
      file = SPIFFS.open(video_dr + String(currentFrame), "r");
      file.readBytes((char*)leds, NUM_LEDS * 3);
      file.close();
      FastLED.show();
      currentFrame++;
      if (currentFrame >= MAX_FRAMES) currentFrame = 0;
    }
  }
}


uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;

  if( SERPENTINE_LAYOUT == false) {
   
  }

  if( SERPENTINE_LAYOUT ) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (MATRIX_WIDTH - 1) - x;
      i = (y * MATRIX_WIDTH) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * MATRIX_WIDTH) + x;
    }
  } else {
     i = (y * MATRIX_WIDTH) + x;
  }

  return i;
}


void displayRGB() {
  leds[XY(MATRIX_WIDTH / 2  - 1, MATRIX_HEIGHT / 2)] = CRGB::Red;
  leds[XY(MATRIX_WIDTH / 2 , MATRIX_HEIGHT / 2)] = CRGB::Green;
  leds[XY(MATRIX_WIDTH / 2  + 1, MATRIX_HEIGHT / 2)] = CRGB::Blue;
  FastLED.show();
  delay(3000);
}
