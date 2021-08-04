#include <FastLED.h>

#define SERPENTINE_LAYOUT true
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 32
#define NUM_LEDS 1024
#define DATA_PIN 3
#define CLOCK_PIN 2
#define BRIGHTNESS 32
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
     //debug: read one led color ( 3 bytes)
     //Serial.readBytes( (char*)(&leds[5]), 3); // read three bytes: r, g, and b.
     Serial.readBytes( (char*)leds, NUM_LEDS * 3);
//     for(int i = 0; i < NUM_LEDS; i++) {
//      CHSV temp = CHSV(hsvs[i]);
//      if (temp.hue > 60 && temp.hue < 200) {
//        temp.val = 0;
//        temp.sat = 0;
//      }
//      if (temp.sat > 100 && temp.val > 100) {
//        temp.hue = 0;
//        temp.val = 255;
//        temp.sat = 255;
//      } else {
//         temp.val = 0;
//        temp.sat = 0;
//      }
//      leds[i] = CHSV(temp);
//     }
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
