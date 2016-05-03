#include "FastLED.h"

FASTLED_USING_NAMESPACE

// developed for use with L2Cv2 for Maker Faire 2016.
// with many thanks to Mark Kriegsman's demoreel 100 which provided the basic concepts and some of the initial patterns.

#if FASTLED_VERSION < 3001000
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN_LEFT 5
#define DATA_PIN_RIGHT 6
#define LED_TYPE    WS2811
#define COLOR_ORDER_LEFT GRB
#define COLOR_ORDER_RIGHT GRB
#define NUM_LEDS_LEFT    70
#define NUM_LEDS_RIGHT   65
#define NUM_LEDS NUM_LEDS_LEFT + NUM_LEDS_RIGHT
#define PX_PER_BOARD 6

CRGB leds[NUM_LEDS];

#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120

//teensy doesn't compile correctly if we don't declare our functions.

void echoDebugs();
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
void paparockzi();
void two_chase();

CRGB global_fg = CRGB::Green;
CRGB global_fg2 = CRGB::Red;
CRGB global_fg3 = CRGB::Blue;
CRGB global_bg = CRGB::Black;
CRGB global_length = 6;
int global_wait = 50;

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
SimplePatternList gPatterns = {  two_chase, paparockzi, ants, chase, randBlocks, randPods, confetti, sinelon, juggle, bpm, rainbow, rainbowWithGlitter, wipe, fract };

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
          break;
       //fg = foreground color 
        case 'f':
          global_fg = CRGB(Serial.parseInt(),Serial.parseInt(),Serial.parseInt());
          break;
       //fg2 = foreground color 2
        case 'g':
          global_fg2 = CRGB(Serial.parseInt(),Serial.parseInt(),Serial.parseInt());
          break;          
       //fg3 = foreground color 3
        case 'h':
          global_fg3 = CRGB(Serial.parseInt(),Serial.parseInt(),Serial.parseInt());
          break;              
        //bg = background color 
        case 'b':
          global_bg = CRGB(Serial.parseInt(),Serial.parseInt(),Serial.parseInt());
          break;
        case 'p':
          nextPattern();
          break;         
        case 'd':
          echoDebugs();
          break;
        default:
          Serial.println("Did not understand command");
          break;
      }
      echoDebugs();
    }    
  }
  
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  

  //this is our global wait between frames
  FastLED.delay(global_wait); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( global_wait) { gw_pod++; if(gw_pod > NUM_LEDS) gw_pod = 0; if(gw_pod < 0) gw_pod = NUM_LEDS;} //advance the global wait pod with wrap
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 1 ) { s_pod++; if(s_pod > NUM_LEDS) s_pod = 0;} //used for pod that advances once per second.
  EVERY_N_SECONDS( 90 ) { nextPattern(); } // change patterns periodically

}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void echoDebugs()
{
  Serial.println("Current values are:");
  Serial.print("global_wait = ");
  Serial.println(global_wait); 
  Serial.print("global_fg = ");
  Serial.println(global_fg); 
  Serial.print("global_bg = ");
  Serial.println(global_bg); 
  Serial.print("gCurrentPatternNumber = ");
  Serial.println(gCurrentPatternNumber);  
}

void nextPattern()
{
  if(random(20) == 1)
  {
    global_fg = CRGB(random(255),random(255),random(255));
    global_bg = CRGB(random(255),random(255),random(255));
    global_length = random(NUM_LEDS);
    global_wait = random(1000);
  }
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}

//patterns by bob

void chase(){
  all(global_bg);
  leds[findLED(gw_pod)] = global_fg;
}

void two_chase(){
  all(global_bg);
  leds[findLED(gw_pod)] = global_fg;
  int rev_pod = NUM_LEDS - gw_pod;
  leds[findLED(rev_pod)] = global_fg2;
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
  {
    return pos;
  }
  return NUM_LEDS - (pos - NUM_LEDS_LEFT);
}

void paparockzi()
{
  all(global_bg);
  if(s_pod % 2 == 0)
  {
    for(int i=0;i<NUM_LEDS/50;i++)
    {
      leds[random(NUM_LEDS)] = global_fg;    
    }
  }else{
    fadeToBlackBy( leds, NUM_LEDS, 10);
  }
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

//end patterns by bob
//patterns from FastLED demoReel

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

//end patterns from FastLED DemoReel
