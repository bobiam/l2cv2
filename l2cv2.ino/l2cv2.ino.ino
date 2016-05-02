#include "FastLED.h"

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if FASTLED_VERSION < 3001000
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN_LEFT 5
#define DATA_PIN_RIGHT 6
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER_LEFT GRB
#define COLOR_ORDER_RIGHT GRB
#define NUM_LEDS_LEFT    70
#define NUM_LEDS_RIGHT   65
#define NUM_LEDS 140
#define PX_PER_BOARD 6

CRGB leds[NUM_LEDS];

#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120

void ChangePalettePeriodically();
void FillLEDsFromPaletteColors( uint8_t colorIndex);
void SetupPurpleAndGreenPalette();
void SetupBlackAndWhiteStripedPalette();
void SetupTotallyRandomPalette();
void rainbow();
void rainbowWithGlitter();
void wipe();
void confetti();
void sinelon();
void juggle();
void bpm();
void nextPattern();
void addGlitter( fract8 chanceOfGlitter);
int findLED(int pos);
void fract();
void fract_segments(CRGB c1,CRGB c2,int segment_size, int wait);
void randPods();
void ants();
void randBlocks();
void all(CRGB all_c);
void chase();

CRGB global_fg = CRGB::Green;
CRGB global_bg = CRGB::Blue;
CRGB global_length = 6;
int global_wait = 1000;

//TODO = function to receive serial from Pi and update:
// - global_ colors and parameters above
// - next Pattern in gPatterns
// Then call nextPattern() to trigger it.  
// this means that there's always a queue for unattended use, using random params, 
// but if someone changes it we'll let them drive it.


void setup() {
  Serial.begin(9600);
  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN_LEFT,COLOR_ORDER_LEFT>(leds, NUM_LEDS_LEFT).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA_PIN_RIGHT,COLOR_ORDER_RIGHT>(leds, NUM_LEDS_LEFT, NUM_LEDS_RIGHT).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  randomSeed(analogRead(0));
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { ants, chase, randBlocks, randPods, confetti, sinelon, juggle, bpm, rainbow, rainbowWithGlitter, wipe, fract };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
int gw_pod = 0;
int s_pod = 0;
char do_what;
  
void loop()
{
  if(Serial.available())
  {
    while(Serial.available())
    {
      do_what = Serial.read();
      switch(do_what){
        //w = wait
        case 'w':
          global_wait = Serial.parseInt();
          Serial.print("Updated wait to ");
          Serial.println(global_wait);
          break;
       //fg = foreground color 
        case 'f':
          global_fg = CRGB(Serial.parseInt(),Serial.parseInt(),Serial.parseInt());
          Serial.print("Updated fg to ");
          Serial.println(global_fg);          
          break;
        //bg = background color 
        case 'b':
          global_bg = CRGB(Serial.parseInt(),Serial.parseInt(),Serial.parseInt());
          Serial.print("Updated bg to ");
          Serial.println(global_bg);          
          break;
        case 'p':
          Serial.println("Advancing to next pattern");          
          nextPattern();
          break;         
        case 'd':
          Serial.println("Current values are:");
          Serial.print("global_wait = ");
          Serial.println(global_wait); 
          Serial.print("global_fg = ");
          Serial.println(global_fg); 
          Serial.print("global_bg = ");
          Serial.println(global_bg); 
          break;
        default:
          Serial.println("Did not understand command");
          break;
      }
    }
    
    //Serial.println(global_wait);
  }
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  
  FastLED.delay(global_wait); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( global_wait) { gw_pod++; if(gw_pod > NUM_LEDS) gw_pod = 0;} //advance the global wait pod with wrap
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 1 ) { s_pod++; if(s_pod > NUM_LEDS) s_pod = 0;} //used for pod that advances once per second.
  EVERY_N_SECONDS( 30 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  if(random(20) == 1)
  {
    global_fg = CRGB(random(255),random(255),random(255));
    global_bg = CRGB(random(255),random(255),random(255));
    global_length = random(NUM_LEDS);
    global_wait = random(1000);
  }
  Serial.println("Current values are:");
  Serial.print("global_wait = ");
  Serial.println(global_wait); 
  Serial.print("global_fg = ");
  Serial.println(global_fg); 
  Serial.print("global_bg = ");
  Serial.println(global_bg); 
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void chase(){
  all(global_bg);
  leds[findLED(gw_pod)] = global_fg;
}

void randPods(){
  leds[random(NUM_LEDS)] = CRGB(random(255),random(255),random(255));
}

void randBlocks(){
  CRGB block_c = CRGB(random(255),random(255),random(255));
  for(int i=0;i<NUM_LEDS;i++)
  {
    if(i % PX_PER_BOARD == 0)
      block_c = CRGB(random(255),random(255),random(255));
    leds[i] = block_c;
  }
}

void wipe()
{
  for(int x=0;x<gw_pod;x++)
  {
    leds[findLED(x)] = global_bg;
  }
  leds[findLED(gw_pod)] = global_fg;
}

void all(CRGB all_c){
  for(int i=0;i<NUM_LEDS;i++)
  {
    leds[i] = all_c;
  }
}

//BE - alternate pods between two colors
void ants(){
  int i;
  for(i=0; i< NUM_LEDS; i++)
  {
    if(gw_pod % 2)
    {     
      if(i % 2)
      {
        leds[findLED(i)] = global_fg;
      }else{
        leds[findLED(i)] = global_bg;
      }
    }else{
      if((i+1) % 2)
      {
        leds[findLED(i)] = global_fg;
      }else{
        leds[findLED(i)] = global_bg;
      }
    }
  }
}

int findLED(int pos)
{
  //we want the first half of the ring to be normal, and the second half of the ring to be the distance from the end of the strip.
  //this flips the second half of the ring so that transitions from segment to segment are clean.

  //return pos;
  if(pos < NUM_LEDS_LEFT)
    return pos;
  
  return NUM_LEDS - (pos - NUM_LEDS_LEFT);
  
}

void fract()
{
  CRGB c1, c2;
  int wait;
  c1 = global_fg;
  c2 = global_bg;
  wait = 500;
  fract_segments(c1,c2,70,wait);
  fract_segments(c1,c2,35,wait);
  fract_segments(c1,c2,17,wait);
  fract_segments(c1,c2,10,wait);  
  fract_segments(c1,c2,5,wait);  
  fract_segments(c1,c2,2,wait);
  fract_segments(c1,c2,1,wait);
}

void fract_segments(CRGB c1,CRGB c2,int segment_size, int wait)
{
  for(int j=1;j<100;j++)
  {
    CRGB c;
    int switcher_count = 0;
    for(int i=0;i<NUM_LEDS;i++)
    {
      if(j%2)
      {
        leds[findLED(i)] = c1;
      }else{
        leds[findLED(i)] = c2;
      }
      switcher_count++;
      if(switcher_count == segment_size)
      {
        switcher_count = 0;
        c = c1;
        c1 = c2;
        c2 = c;
      }      
    }
    FastLED.show();
    delay(wait);  
    wait = wait * .9;        
  }
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

