#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN            8
#define NUMPIXELS      300
#define GLOBE_SIZE     2   //How many leds are inside of one globe
#define GLOBE_SPACING  10   //this minus GLOBE_SIZE equals the amount of LEDs between globes
#define GLOBE_COUNT    30  //just to save RAM - we should calculate this on the fly though
#define FRAMERATE      30  //how many frames per second to we ideally want to run

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

unsigned long lastFrame;
uint32_t globes[GLOBE_COUNT];

typedef enum {
  G_NOTOUCH,
  G_RAINBOW,
  G_BLANK,
  G_COLOR,
  G_STROBEONCE
} gstate_t;
gstate_t g = G_NOTOUCH;

typedef enum {
  S_NOTOUCH,
  S_RAINBOW,
  S_BLANK,
  S_RAIN,
  S_PAPARAZZI,
  S_COLOR
} sstate_t;
sstate_t s = S_NOTOUCH;
#define S_RAINBOW_SNAKE_LENGTH 15

void setup() {

  pinMode(13, OUTPUT); 

  Serial.begin(38400);
  Serial.println("#globepixels coming up");

  pixels.begin();
  g = G_RAINBOW;
  s = S_RAIN;
  Serial.println("#leds up");
  
  Wire.begin(8);
  Wire.onReceive(handleWire);
  Serial.println("#wire up");

  lastFrame = millis();
  Serial.print("#up at "); Serial.println(lastFrame);
  
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
      if ( led_pos < NUMPIXELS ) { //don't overrun the strand you idiot
        pixels.setPixelColor(led_pos,globes[globe_num]);
      }
    }
  }
}

int g_offset = 0;
void runG_RAINBOW() {
  
  if ( g_offset > 255 ) {
    g_offset=0;
  }
  
  for ( int i=0; i<GLOBE_COUNT; i++ ) {
    setGlobe(i, wheelForPos(i*GLOBE_SPACING,g_offset) );
  }
  
  g_offset += 5;
  
}
void runG_BLANK() {
  for ( int i=0; i<GLOBE_COUNT; i++ ) {
    setGlobe(i,0);
  }
}
uint32_t g_color = pixels.Color(80,141,172);
void runG_COLOR() {
  for ( int i=0; i<GLOBE_COUNT; i++ ) {
    setGlobe(i,g_color);
  }
}
void runG_STROBEONCE() {
  runG_COLOR();
  g=G_BLANK;
}

int s_snake_offset = 0;
int s_snake_end = 0;
void runS_RAINBOW() {

  if ( s_snake_offset > NUMPIXELS ) {
    s_snake_offset = 0;
  }

  pixels.setPixelColor(s_snake_offset, (HalfColor(wheelForPos(s_snake_offset), 1)));
  if ( s_snake_offset >= S_RAINBOW_SNAKE_LENGTH ) {
    s_snake_end = s_snake_offset-S_RAINBOW_SNAKE_LENGTH;
  } else {
    s_snake_end = NUMPIXELS-S_RAINBOW_SNAKE_LENGTH+s_snake_offset;
  }
  pixels.setPixelColor(s_snake_end, pixels.Color(0,0,0));
  //fade stuff 
  pixels.setPixelColor(s_snake_end+1, HalfColor(pixels.getPixelColor(s_snake_end+1), 1));
  pixels.setPixelColor(s_snake_end+2, HalfColor(pixels.getPixelColor(s_snake_end+2), 2));
  pixels.setPixelColor(s_snake_end+3, HalfColor(pixels.getPixelColor(s_snake_end+3), 3));
  
  s_snake_offset++;

  //delay(50);

}
void runS_BLANK() {
  for ( int i=0; i<NUMPIXELS; i++ ) {
    pixels.setPixelColor(i,0);
  }
}
void runS_RAIN() {
  
  //fade out existing pixels
  for ( int i=0; i<NUMPIXELS; i++ ) {
    byte x = pixels.getPixelColor(i);
    if ( x < 10 ) {
      x=0;
    } else {
      x-=10;
    }
    pixels.setPixelColor(i,pixels.Color(x,x,x));
  }

  //decide if we want to add a new raindrop
  if ( random(0,2) == 0 ) {
    //we do
    pixels.setPixelColor(random(0,NUMPIXELS), pixels.Color(255,255,255));
  }
  
  //delay(40);
}
void runS_PAPARAZZI() {
  runS_BLANK();
  //decide if we want to add a new raindrop
  if ( random(0,100) < 60 ) {
    //we do
    pixels.setPixelColor(random(0,NUMPIXELS), pixels.Color(255,255,255));
  }
  //delay(random(75,100));
}
uint32_t s_color = pixels.Color(80,141,172);
void runS_COLOR() {
  for ( int i=0; i<NUMPIXELS; i++ ) {
    pixels.setPixelColor(i,s_color);
  }
}


void runGlobes() {
  if ( g == G_RAINBOW ) {
    runG_RAINBOW();
  } else if ( g == G_BLANK ) {
    runG_BLANK();
  } else if ( g == G_COLOR ) {
    runG_COLOR();
  } else if ( g == G_STROBEONCE ) {
    runG_STROBEONCE();
  }
}
void runStrip() {
  if ( s == S_RAINBOW ) {
    runS_RAINBOW();
  } else if ( s == S_RAIN ) {
    runS_RAIN();
  } else if ( s == S_PAPARAZZI ) {
    runS_PAPARAZZI();
  } else if ( s == S_BLANK ) {
    runS_BLANK();
  } else if ( s == S_COLOR ) {
    runS_COLOR();
  }
}

void handleWire(int count) {
  Serial.println("#Got i2c");
  while (0 < Wire.available()) {
    char c = Wire.read(); 
    Serial.print(c);
  }
  Serial.println();
}

void loop() {

  if ( (millis() - (1000/FRAMERATE)) > lastFrame ) {
    lastFrame = millis();
    
    Serial.print("### BEGIN FRAME ### "); Serial.println(lastFrame);
  
    digitalWrite(13,LOW); //start of LED processing

    //Serial.print("#begin globes at "); Serial.println(millis());
    runGlobes();
    //Serial.print("#begin strip at "); Serial.println(millis());
    runStrip();
  
    //Serial.print("#begin other at "); Serial.println(millis());
    digitalWrite(13,HIGH); //tell the user we're done
  
    //Serial.print("#begin write at "); Serial.println(millis());
    writeGlobes();
    pixels.show(); // This sends the updated pixel color to the hardware.

  }

  if ( Serial.peek() == -1 ) {
    //do nothing. There is nothing to read
  }
  //we must remember to pop the command char off, if it's a single-byte command.
  else if ( Serial.peek() == (byte)'r' ) { g=G_RAINBOW; Serial.read(); }
  else if ( Serial.peek() == (byte)'b' ) { g=G_BLANK; Serial.read(); }
  else if ( Serial.peek() == (byte)'c' ) {
    if ( Serial.available() >= 4 ) {
      //we have the 'c' character and the three color bytes.
      Serial.read(); //throw away the 'c'
      g_color = pixels.Color(Serial.read(), Serial.read(), Serial.read());
      g=G_COLOR;
    }
  }
  else if ( Serial.peek() == (byte)'s' ) { g=G_STROBEONCE; Serial.read(); }

  else if ( Serial.peek() == (byte)'R' ) { s=S_RAINBOW; Serial.read(); }
  else if ( Serial.peek() == (byte)'B' ) { s=S_BLANK; Serial.read(); }
  else if ( Serial.peek() == (byte)'C' ) {
    if ( Serial.available() >= 4 ) {
      //we have the 'c' character and the three color bytes.
      Serial.read(); //throw away the 'c'
      s_color = pixels.Color(Serial.read(), Serial.read(), Serial.read());
      s=S_COLOR;
    }
  }
  else if ( Serial.peek() == (byte)'A' ) { s=S_RAIN; Serial.read(); }
  else if ( Serial.peek() == (byte)'P' ) { s=S_PAPARAZZI; Serial.read(); }
  else { Serial.print((char)Serial.read()); Serial.println("?"); }
    
}

//My utilities below

uint32_t wheelForPos(int x) { return wheelForPos(x,0); }
uint32_t wheelForPos(int x, int offset) {
  int thing = ((float)x/(float)NUMPIXELS)*255;
  thing += offset;
  while ( thing > 255 ) {
    thing -= 255;
  }
  return Wheel((byte)thing);
}

uint32_t ReduceColor(uint32_t color, int percent) {
  //broken
  int red = (int)(color>>16 & 0xFF);
  int green = (int)(color>>8 & 0xFF);
  int blue = (int)(color & 0xFF);
  //Serial.print(red); Serial.print(","); Serial.print(green); Serial.print(","); Serial.println(blue);
  return pixels.Color((byte)red,(byte)green,(byte)blue);
}

//Stolen utilities below

//from strandtest
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//from https://learn.adafruit.com/multi-tasking-the-arduino-part-3/put-it-all-together-dot-dot-dot
uint32_t HalfColor(uint32_t color, int times) {
  // Shift R, G and B components one bit to the right
  uint32_t dimColor = pixels.Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
  if ( times == 1 ) {
    return dimColor;
  } else {
    return HalfColor(dimColor,times-1);
  }
}
// Returns the Red component of a 32-bit color
uint8_t Red(uint32_t color)
{
  return (color >> 16) & 0xFF;
}
     
// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color)
{
  return (color >> 8) & 0xFF;
}
     
// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color)
{
  return color & 0xFF;
}

