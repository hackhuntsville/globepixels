#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN            8
#define NUMPIXELS      300
#define GLOBE_SIZE     2   //How many leds are inside of one globe
#define GLOBE_SPACING  9   //this minus GLOBE_SIZE equals the amount of LEDs between globes
#define GLOBE_COUNT    30  //just to save RAM

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

uint32_t globes[GLOBE_COUNT];

void setup() {
  Serial.begin(115200);
  pixels.begin();
  
  setAllGlobes(pixels.Color(0,0,100));
}

void setGlobe(int x, uint32_t color) {
  globes[x] = color;
}

void setAllGlobes(uint32_t color) {
  for ( int i=0; i<GLOBE_COUNT; i++ ) {
    setGlobe(i,color);
  }
}

void writeGlobes() {
  for ( int globe_num=0; globe_num<GLOBE_COUNT; globe_num++ ) {
    int globe_pos = globe_num*GLOBE_SPACING;
    for ( int led_pos=globe_pos; led_pos<globe_pos+GLOBE_SIZE; led_pos++ ) {
      pixels.setPixelColor(led_pos,globes[globe_num]);
      Serial.print(led_pos); Serial.print(",");
    }
    Serial.print(" ");
  }
  Serial.println();
}

void loop() {
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(0,10,0)); // Moderately bright green color.
    
    writeGlobes();
    pixels.show(); // This sends the updated pixel color to the hardware.
    
    //delay(500); // Delay for a period of time (in milliseconds).
  }
}

