#include <FastLED.h>
#include <FS.h>

#define SERPENTINE_LAYOUT true
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 32
#define NUM_LEDS 1024
#define DATA_PIN 4
#define CLOCK_PIN 5
#define BRIGHTNESS 16
CRGB leds[NUM_LEDS];
#define MAX_FRAMES 194 // temp max frames, send this over serial later
int currentIncomingFrame = 0; // Keep track of how many frames we have recieved
int currentFrame = 0;

String video_dr = "/video_frames/"; // Base SPIFFs video directory
bool isDone;

void setup() { 
  SPIFFS.begin(); // Start the SPI Flash Files System
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
  displayRGB(); 
  Serial.begin(921600);

  // Check if an animation already exists
  isDone = SPIFFS.exists(video_dr + String(MAX_FRAMES));
}


char buf[NUM_LEDS * 3];
File file;
FSInfo fs_info;
void loop() { 
  int startChar = Serial.read();
  // On start char read frame and save to SPIFFS
  if (startChar == '*') {
      isDone = false;
      // Read a frame from serial and store into SPIFFS
      Serial.readBytes(buf, NUM_LEDS * 3);
      file = SPIFFS.open(
        video_dr + String(currentIncomingFrame), "w");
      file.write(buf, NUM_LEDS * 3);
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
