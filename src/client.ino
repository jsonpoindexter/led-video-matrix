#include <FastLED.h>

#define SERPENTINE_LAYOUT true
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 32
#define NUM_LEDS 1024
#define DATA_PIN 3
#define CLOCK_PIN 2
#define BRIGHTNESS 64
CRGB leds[NUM_LEDS];
//CHSV hsvs[NUM_LEDS];

void setup() { 
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  displayRGB(); 
  FastLED.clear();
  FastLED.show();
  
  Serial.begin(921600);
}

void loop() { 
 int startChar = Serial.read();
  if (startChar == '*') {
     Serial.readBytes( (char*)leds, NUM_LEDS * 3);
     FastLED.show();
  }
  else if (startChar >= 0) {
    // discard unknown characters
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
