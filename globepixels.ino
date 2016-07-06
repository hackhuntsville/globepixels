#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN            8
#define NUMPIXELS      120
#define GLOBE_SIZE     2   //How many leds are inside of one globe
#define GLOBE_SPACING  10  //this minus GLOBE_SIZE equals the amount of LEDs between globes
#define GLOBE_COUNT    30  //just to save RAM - we should calculate this on the fly though
#define FRAMERATE      30  //how many frames per second to we ideally want to run

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

unsigned long lastFrame;
unsigned long lastCleanup;
unsigned long frameCount;
unsigned long sloshCount;
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
  S_COLOR,
  S_SPARKLE,
  S_DRIP
} sstate_t;
sstate_t s = S_NOTOUCH;
#define S_RAINBOW_SNAKE_LENGTH 15

void setup() {

  pinMode(13, OUTPUT); 

  Serial.begin(38400);
  Serial.setTimeout(10);
  Serial.println("#globepixels coming up");

  pixels.begin();
  g = G_RAINBOW;
  s = S_RAIN;
  Serial.println("#leds up");
  
  Wire.begin(8);
  //Wire.onReceive(handleWire);
  Serial.println("#wire up");

  lastFrame = millis(); lastCleanup = millis(); frameCount = 0; sloshCount = 0;
  Serial.print("#up at "); Serial.println(lastFrame);
  
}

void setGlobe(int x, uint32_t color) {
  globes[x] = color;
}
int getGlobe(int x) {
  return globes[x];
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

bool isInGlobe(int pos) {
  return (pos%GLOBE_SPACING) < GLOBE_SIZE;
}

int whichGlobe(int pos) {
  return pos/GLOBE_SPACING;
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
void runS_SPARKLE() {
  for ( int i=0; i<25; i++ ) { 
    if ( random(0,100) < 15 ) {
      pixels.setPixelColor(random(0,NUMPIXELS), s_color); //light one
    } else {
      pixels.setPixelColor(random(0,NUMPIXELS), 0); //extinguish one
    }
  }
}
int drip_pos = 0; 
bool drip_flip = false;
void runS_DRIP() {

  //Serial.print("#Drip pos = "); Serial.println(drip_pos);

  if ( drip_pos <= 0 ) {
    drip_pos = NUMPIXELS-1;
  }

  if ( isInGlobe(drip_pos) ) {
    pixels.setPixelColor(drip_pos+1,0); //blank the one above this because it's not in the globe.
    int which = whichGlobe(drip_pos);
    byte x = getGlobe(which);
    if ( drip_flip == false ) {
      if ( x < 200 ) {
        x+=10;
      } else {
        x=200;
        drip_flip = true;
      }
    } else { //drip_flip is true; let's fade out instead;
      if ( x > 10 ) {
        x-=10;
      } else {
        x=0;
        drip_flip=false;
        while (drip_pos > 0 and isInGlobe(drip_pos)) {
          drip_pos--;
        }
      }
    }
    setGlobe(which,pixels.Color(0,(x*.75),x));
  } else { //it's not a globe, do something cool in between.
    pixels.setPixelColor(drip_pos,pixels.Color(0,0,50));
    if ( !isInGlobe(drip_pos+1) ) {
      pixels.setPixelColor(drip_pos+1,0);
    }
    drip_pos--;
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
  } else if ( s == S_SPARKLE ) {
    runS_SPARKLE();
  } else if ( s == S_DRIP ) {
    runS_DRIP();
  }
}

/*
void handleWire(int count) {
  Serial.print("#Got i2c ");
  while (0 < Wire.available()) {
    char c = Wire.read(); 
    Serial.print(c);
  }
  Serial.println();
}
*/

void loop() {

  if ( (millis() - lastCleanup) > 1000 ) {
    double fr = (double)frameCount/((double)(millis()-lastCleanup)/1000);
    float load = estimateLoad();
    Serial.print("#FRAME RATE: "); Serial.print(fr);
    Serial.print(" - SLOSH: "); Serial.print(sloshCount);
    Serial.print(" - LOAD: "); Serial.println(load);
    
    lastCleanup = millis();
    frameCount = 0; sloshCount = 0;
  }

  if ( (millis() - lastFrame) > (1000/FRAMERATE) ) {
    lastFrame = millis();
    frameCount++;
    
    //Serial.print("### BEGIN FRAME ### "); Serial.println(lastFrame);
  
    digitalWrite(13,LOW); //start of LED processing

    //Serial.print("#begin globes at "); Serial.println(millis());
    runGlobes();
    //Serial.print("#begin strip at "); Serial.println(millis());
    runStrip();
  
    //Serial.print("#begin other at "); Serial.println(millis());
    digitalWrite(13,HIGH); //tell the user we're done
  
    //Serial.print("#begin write at "); Serial.println(millis());
    writeGlobes();

    //make sure we aren't overloading, and dim if we are.
    estimateLoad(); //TODO - actually do something with this

    pixels.show(); // This sends the updated pixel color to the hardware.

  } else {
    sloshCount++; //we didn't do anything so let's indicate that we had a spare cycle.
  }

  processControlStream(Serial);
  processControlStream(Wire);

}

//My utilities below

float estimateLoad() {
  //thanks from https://github.com/teachop/xcore_neopixel_buffered/blob/master/module_neopixel/src/neopixel.xc#L38
  float load = 0;
  uint32_t color = 0;
  int element = 0;
  for ( int i=0; i<NUMPIXELS; i++ ) {
    color = pixels.getPixelColor(i);
    element = Red(color);
    load += ((float)element/255)*20;
    //Serial.print("Red val is "); Serial.print(element);
    //Serial.print(" so load is "); Serial.print((((float)element/255)*20)); Serial.println("mA");
    element = Green(color);
    load += ((float)element/255)*20;
    element = Blue(color);
    load += ((float)element/255)*20;
    //Serial.println(load);
  }
  return load;
}

void processControlStream(Stream &stream) {

  if ( stream.peek() == -1 ) {
    //do nothing. There is nothing to read
  }
  //we must remember to pop the command char off, if it's a single-byte command.
  else if ( stream.peek() == (byte)'r' ) { g=G_RAINBOW; stream.read(); }
  else if ( stream.peek() == (byte)'b' ) { g=G_BLANK; stream.read(); }
  else if ( stream.peek() == (byte)'c' ) { g_color = getColorFromStream(stream); } //set color
  else if ( stream.peek() == (byte)'o' ) { g=G_COLOR; stream.read(); } //color mode
  else if ( stream.peek() == (byte)'s' ) { g=G_STROBEONCE; stream.read(); }
  else if ( stream.peek() == (byte)'n' ) { g=G_NOTOUCH; stream.read(); }

  else if ( stream.peek() == (byte)'R' ) { s=S_RAINBOW; stream.read(); }
  else if ( stream.peek() == (byte)'B' ) { s=S_BLANK; stream.read(); }
  else if ( stream.peek() == (byte)'C' ) { s_color = getColorFromStream(stream); } //set color
  else if ( stream.peek() == (byte)'O' ) { s=S_COLOR; stream.read(); } //color mode
  else if ( stream.peek() == (byte)'A' ) { s=S_RAIN; stream.read(); }
  else if ( stream.peek() == (byte)'P' ) { s=S_PAPARAZZI; stream.read(); }
  else if ( stream.peek() == (byte)'K' ) { s=S_SPARKLE; stream.read(); }
  else if ( stream.peek() == (byte)'N' ) { s=S_NOTOUCH; stream.read(); }
  else if ( stream.peek() == (byte)'D' ) { s=S_DRIP; g=G_NOTOUCH; stream.read(); }
  
  else { Serial.print((char)stream.read()); Serial.println("?"); }

}

uint32_t getColorFromStream(Stream &stream) {
  //we have the 'c' character and the three color bytes.
  stream.read(); //throw away the 'c'
  Serial.print("#Set color to ");
  int r=stream.parseInt(); Serial.print(r); 
  int g=stream.parseInt(); Serial.print(","); Serial.print(g); 
  int b=stream.parseInt(); Serial.print(","); Serial.println(b);
  return pixels.Color(r,g,b);
}

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

