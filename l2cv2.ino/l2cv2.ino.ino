#include "FastLED.h"

FASTLED_USING_NAMESPACE

// developed for use with L2Cv2 for Maker Faire 2016 by Bob Eells
// with many thanks to Mark Kriegsman's demoreel 100 which provided the basic concepts and some of the initial patterns. ( See http://fastled.io/ )
// and to Andrew Tuline (http://tuline.com/) and Daniel Wilson, 
// and to my co-conspirators on this project, notably Bruce, Alex and Jeremy. 
// Portions of this code are shared under GPL by the above authors...other pieces are shared under Creative Commons or other very permissive licenses.
// Any section not marked as belonging to another programmer can be assumed "CC BY".  The world needs more blinky lights.  Go make it go.  

#if FASTLED_VERSION < 3001000
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN_LEFT 6
#define DATA_PIN_RIGHT 5
#define LED_TYPE    WS2811
#define COLOR_ORDER_LEFT RBG
#define COLOR_ORDER_RIGHT RBG
#define NUM_LEDS_LEFT    192 
#define NUM_LEDS_RIGHT   192
#define NUM_LEDS NUM_LEDS_LEFT + NUM_LEDS_RIGHT
#define PX_PER_BOARD 6

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

CRGB leds[NUM_LEDS];

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
void handleSerial();
void ripple();
void lightning();
void one_sine();
void lightLED();
void blendme();
void averageFade();
void omgp();
void SomeoneIsInTheTower();
void debug_boundary_conditions();

uint8_t global_freq = 16;                                         // You can change the frequency, thus distance between bars.

CRGB global_fg = CRGB::Green;
CRGB global_fg2 = CRGB::Red;
CRGB global_fg3 = CRGB::Blue;
CRGB global_bg = CRGB::Black;
int global_span = 5;
int global_gate = 30;
int global_wait = 15;
int global_bright = 50;
int global_pos = 0;
bool global_lock = false;

//for ripple
uint8_t colour;                                               // Ripple colour is randomized.
int center = 0;                                               // Center of the current ripple.
int step = -1;                                                // -1 is the initializing step.
uint8_t myfade = 255;                                         // Starting brightness.
#define maxsteps 16                                           // Case statement wouldn't allow a variable.


//TODO = function to receive serial from Pi and update:
// - global_ colors and parameters above
// - next Pattern in gPatterns
// Then call nextPattern() to trigger it.  
// this means that there's always a queue for unattended use, using random params, 
// but if someone changes it we'll let them drive it.


void setup() {
  Serial.begin(9600);
  Serial.setTimeout(20);
  delay(3000); // 3 second delay for recovery

  Serial.println("L2Cv2 is starting up.");
  
  Serial.print("Configured for left hand side on pin ");
  Serial.print(DATA_PIN_LEFT);
  Serial.print(" of ");
  Serial.print(NUM_LEDS_LEFT);  
  Serial.println(" leds");
  
  Serial.print("Configured for right hand side on pin ");
  Serial.print(DATA_PIN_RIGHT);
  Serial.print(" of ");
  Serial.print(NUM_LEDS_RIGHT);  
  Serial.println(" leds");  

  Serial.print("Total LEDS is: ");
  Serial.println(NUM_LEDS);
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN_LEFT,COLOR_ORDER_LEFT>(leds, NUM_LEDS_LEFT).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,DATA_PIN_RIGHT,COLOR_ORDER_RIGHT>(leds, NUM_LEDS_LEFT, NUM_LEDS_RIGHT).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(global_bright);

  all(CRGB::White);
  FastLED.show();
  delay(3000);  
  all(CRGB::Red);
  FastLED.show();
  delay(1000);
  all(CRGB::Green);
  FastLED.show();
  delay(1000);
  all(CRGB::Blue);
  FastLED.show();
  delay(1000);  
  all(CRGB::White);
  FastLED.show();
  delay(3000);  

  randomSeed(analogRead(0));
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {  chase, wipe, blendme, one_sine, lightning, ripple, two_chase, paparockzi, ants, randBlocks, randPods, confetti, sinelon, juggle, bpm, rainbow, rainbowWithGlitter, averageFade, omgp };
char* patternNames[] = {"chase","wipe", "blendme", "one_sine", "lightning", "ripple", "two_chase", "paparockzi", "ants", "randBlocks", "randPods", "confetti", "sinelon", "juggle", "bpm", "rainbow", "rainbowWithGlitter", "averageFade", "omgp" };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
int gw_pod = 0;
int s_pod = 0;
int tower_pod = 0;
char do_what;
  
void loop()
{
  if(Serial.available())
  {
    handleSerial();
  }
  
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  

  //this is our global wait between frames
  FastLED.delay(global_wait); 
  gw_pod++; 
  if(gw_pod >= NUM_LEDS) 
    gw_pod = 0; 
  if(gw_pod < 0) 
    gw_pod = NUM_LEDS;
  
  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 1 ) { s_pod++; if(s_pod > NUM_LEDS) s_pod = 0;} //used for pod that advances once per second.
  EVERY_N_SECONDS( 90 ) { nextPattern(); } // change patterns periodically

}

void handleSerial()
{
  while(Serial.available())
  {
    do_what = Serial.read();
    switch(do_what){
      //a = max_brightness
      case 'a':
        global_bright = Serial.parseInt();
        FastLED.setBrightness(global_bright);
        break;
      //b = background color 
      case 'b':
        global_bg = CRGB(Serial.parseInt(),Serial.parseInt(),Serial.parseInt());
        break;
      //d = debug 
      case 'd':
        echoDebugs();
        break;
      //f = foreground color 
      case 'f':
        global_fg = CRGB(Serial.parseInt(),Serial.parseInt(),Serial.parseInt());
        break;
      //g = foreground color 2
      case 'g':
        global_fg2 = CRGB(Serial.parseInt(),Serial.parseInt(),Serial.parseInt());
        break;          
      //h = foreground color 3
      case 'h':
        global_fg3 = CRGB(Serial.parseInt(),Serial.parseInt(),Serial.parseInt());
        break;         
      //l = lock     
      case 'l':
        global_lock = ! global_lock;
        Serial.print("Global lock value now set to ");
        Serial.println(global_lock);
        break;
      //m = light specific pod
      case 'm':
        global_pos = Serial.parseInt();
        break;
      //p = new pattern
      case 'p':
        nextPattern();
        break;         
      //q = specific pattern
      case 'q':
        gCurrentPatternNumber = Serial.parseInt();
        if(gCurrentPatternNumber > ARRAY_SIZE(gPatterns) || gCurrentPatternNumber < 0)
        {
          gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
          Serial.print("Your pattern number was out of range.  Using ");
          Serial.println(gCurrentPatternNumber);
        }          
        break;          
      //r = frequency adjustment (used for one_sine)
      case 'r':
        global_freq = Serial.parseInt();
        break;
      //w = wait
      case 's':
        global_span = Serial.parseInt();
        break;
      case 't':
        global_gate = Serial.parseInt();
        break;
      case 'w':
        global_wait = Serial.parseInt();
        break;
      case 'z':
        tower_pod = Serial.parseInt();
        SomeoneIsInTheTower();
        break;
      default:
        Serial.println("Did not understand command");
        break;
    }
    echoDebugs();
  }    
}

//debug_boundary_conditions
//good for counting pods or debugging edge cases.
//uses no globals
void debug_boundary_conditions(){
  bool swit = false;
  CRGB c;
  for(int i=0;i<NUM_LEDS;i++){
    if(i%10 == 0)
    {
      swit = !swit;
    }
    if(swit){
      c = CRGB::Blue;    
    }else{
      c = CRGB::Red;
    }
    leds[findLED(i)] = c;
    FastLED.show();
  }
}

//echoDebugs
//spits out some of the global values
//reads several globals, modifies none.
void echoDebugs()
{
  Serial.println("Current values are:");
  Serial.print("global_bright = ");
  Serial.println(global_bright);
  Serial.print("global_wait = ");
  Serial.println(global_wait); 
  Serial.print("global_span  = ");
  Serial.println(global_span);   
  Serial.print("global_fg = ");
  Serial.println(global_fg); 
  Serial.print("global_fg2 = ");
  Serial.println(global_fg2); 
  Serial.print("global_bg = ");
  Serial.println(global_bg); 
  Serial.print("gCurrentPatternNumber = ");
  Serial.println(gCurrentPatternNumber);  
}


//patterns by bob

//SomeoneIsInTheTower
//responds to the z Serial char. Draws a pixel.
void SomeoneIsInTheTower()
{
  fadeToBlackBy( leds, NUM_LEDS, 1);
  leds[findLED(tower_pod)] = global_fg;
  Serial.print("I set tower_pod at ");
  Serial.println(tower_pod);
  FastLED.show();
  delay(500);
}

//chase 
//pattern 0
//sends a pod around the ring.
//uses global_bg, gw_pod (generated from global_wait) and global_fg
void chase(){
  all(global_bg);
  leds[findLED(gw_pod)] = global_fg;
}

//two_chase
//pattern 6
//sends two pods around the ring in opposite directions
//uses global_bg, gw_pod, global_fg, global_fg2.
void two_chase(){
  all(global_bg);
  leds[findLED(gw_pod)] = global_fg;
  int rev_pod = NUM_LEDS - gw_pod;
  leds[findLED(rev_pod)] = global_fg2;
}



//randPods
//Pattern 10
//uses no globals
void randPods(){
  leds[random(NUM_LEDS)] = CRGB(random(255),random(255),random(255));
}

//randBlocks
//Pattern 9
//uses PX_PER_BOARD but no runtime globals
void randBlocks(){
  CRGB block_c = CRGB(random(255),random(255),random(255));
  for(int i=0;i<NUM_LEDS;i++)
  {
    if(i % PX_PER_BOARD == 0)
      block_c = CRGB(random(255),random(255),random(255));
    leds[i] = block_c;
  }
}

//wipe 
//Pattern 1
//uses global_fg, global_bg
void wipe()
{
  if(gw_pod == 0)
    all(global_bg);
  leds[findLED(gw_pod)] = global_fg;
}

//ants
//Pattern 8
//alternate pods between two colors
//uses global_fg and global_bg
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

//paparockzi
//Pattern 7
//random camera-flashes in colors defined by global_fg and global_bg
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
    fadeToBlackBy( leds, NUM_LEDS, 1);
  }
}

//fract
//not currently working.
void fract()
{
  global_wait = 0;
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
    if(!Serial.available()) delay(wait);  
    wait = wait * .9;        
  }
}

//omgp - oh my god, Ponies!
//Pattern 18
//runs a rainbow chase
//no globals.
void omgp()
{
  switch((gw_pod+gHue) % 7)
  {
    case 0 :
      leds[findLED(gw_pod)] = CRGB::Red;
      break;
    case 1 :
      leds[findLED(gw_pod)] = CRGB::Orange;
      break;
    case 2 :
      leds[findLED(gw_pod)] = CRGB::Yellow;
      break;
    case 3 :
      leds[findLED(gw_pod)] = CRGB::Green;
      break;
    case 4 :
      leds[findLED(gw_pod)] = CRGB::Blue;
      break;
    case 5 :
      leds[findLED(gw_pod)] = CRGB::Indigo;
      break;
    case 6 :
      leds[findLED(gw_pod)] = CRGB::Violet;
      break;
  }
}

//end patterns by bob

//patterns from FastLED demoReel by Mark Kriegsman

//confetti
//Pattern 11
//no globals
void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[findLED(pos)] += CHSV( gHue + random8(64), 200, 255);
}

//sinelon 
//Pattern 12
//no globals
void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[findLED(pos)] += CHSV( gHue, 255, 192);
}

//bpm
//Pattern 13
//no globals
void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = global_wait;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[findLED(i)] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

//juggle
//Pattern 14
//no globals
void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[findLED(beatsin16(i+7,0,NUM_LEDS))] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

//rainbow
//Pattern 15
//no globals
void rainbow() 
{
  // FastLED's built-in rainbow generator
  //behaves strangely thanks to lack of findLED() wrapper in fill_rainbow;
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

//rainbowWithGlitter
//pattern 16
//no globals
void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

//helper only
void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

//end patterns from FastLED DemoReel

//patterns from FastLED-Demos by Andrew Tuline

uint8_t bgcol = 0;                                            // Background colour rotates.
int thisdelay = 100;                                           // Standard delay value.

void ripple() {
  for (int i = 0; i < NUM_LEDS; i++) leds[findLED(i)] = CHSV(bgcol++, 255, 5);  // Rotate background colour.
  fadeToBlackBy(leds, NUM_LEDS, 10);

  switch (step) {

    case -1:                                                          // Initialize ripple variables.
      center = random(NUM_LEDS);
      colour = random8();
      step = 0;
      break;

    case 0:
      leds[findLED(center)] = CHSV(colour, 255, 255);                          // Display the first pixel of the ripple.
      step ++;
      break;

    case maxsteps:                                                    // At the end of the ripples.
      step = -1;
      break;

    default:                                                             // Middle of the ripples.
      leds[findLED((center + step + NUM_LEDS) % NUM_LEDS)] += CHSV(colour, 255, myfade/step*2);       // Simple wrap from Marc Miller
      leds[findLED((center - step + NUM_LEDS) % NUM_LEDS)] += CHSV(colour, 255, myfade/step*2);
      step ++;                                                         // Next step.
      break;  
  } // switch step
} // ripple()

//  Lightnings is a program that lets you make an LED strip look like a 1D cloud of lightning
//  Original by: Daniel Wilson, 2014
//  Modified by: Andrew Tuline 2015
//  This modified version creates lightning along various sections of the strip. Looks great inside my poly fill constructed cloud.

uint8_t frequency = 50;                                       // controls the interval between strikes
uint8_t flashes = 8;                                          //the upper limit of flashes per strike
unsigned int dimmer = 1;

uint8_t ledstart;                                             // Starting location of a flash
uint8_t ledlen;                                               // Length of a flash

//  Lightnings is a program that lets you make an LED strip look like a 1D cloud of lightning
//  Original by: Daniel Wilson, 2014
//  Modified by: Andrew Tuline 2015
//  Modified slightly by: Bob Eells 2016

void lightning(){
  all(CRGB::Black);
  ledstart = random8(NUM_LEDS);           // Determine starting location of flash
  ledlen = random8(NUM_LEDS-ledstart);    // Determine length of flash (not to go beyond NUM_LEDS-1)
  for (int flashCounter = 0; flashCounter < random8(3,flashes); flashCounter++) {
    if(flashCounter == 0) dimmer = 5;     // the brightness of the leader is scaled down by a factor of 5
    else dimmer = random8(1,3);           // return strokes are brighter than the leader
    fill_solid(leds+ledstart,ledlen,CHSV(255, 0, 255/dimmer));
    FastLED.show();                       // Show a section of LED's
    delay(random8(4,10));                 // each flash only lasts 4-10 milliseconds
    fill_solid(leds+ledstart,ledlen,CHSV(255,0,0));   // Clear the section of LED's
    FastLED.show();     
    if (flashCounter == 0) delay (150);   // longer delay until next flash after the leader
    delay(50+random8(100));               // shorter delay between strokes  
  } // for()
  delay(random8(frequency)*100);          // delay between strikes
}

/* one_sine
By: Andrew Tuline
Date: Jan, 2015
This routine uses sine waves to move pixels around. It's much simpler than counting them.
You COULD add a beat and map the output to the location for it to go back and forth.
*/

#define qsubd(x, b)  ((x>b)?wavebright:0)                     // Digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b)  ((x>b)?x-b:0)                            // Analog Unsigned subtraction macro. if result <0, then => 0
// Initialize changeable global variables. Play around with these!!!
uint8_t wavebright = 128;                                     // You can change the brightness of the waves/bars rolling across the screen.
uint8_t thishue = 0;                                          // You can change the starting hue value for the first wave.
uint8_t thisrot = 1;                                          // You can change how quickly the hue rotates for this wave. Currently 0.
uint8_t allsat = 255;                                         // I like 'em fully saturated with colour.
int8_t thisspeed = -8;                                         // You can change the speed of the wave, and use negative values.
int thisphase = 0;                                            // Phase change value gets calculated.
uint8_t thiscutoff = 192;                                     // You can change the cutoff value to display this wave. Lower value = longer wave.
uint8_t bgclr = 0;                                            // A rotating background colour.
uint8_t bgbri = 16;                                           // Brightness of background colour


void one_sine() {                                                             // This is the heart of this program. Sure is short.
  thisphase += thisspeed;                                                     // You can change direction and speed individually.
  thishue = thishue + thisrot;                                                // Hue rotation is fun for thiswave.
  for (int k=0; k<NUM_LEDS+1; k++) {                                          // For each of the LED's in the strand, set a brightness based on a wave as follows:
    int thisbright = qsubd(cubicwave8((k*global_freq)+thisphase), thiscutoff);    // qsub sets a minimum value called thiscutoff. If < thiscutoff, then bright = 0. Otherwise, bright = 128 (as defined in qsub)..
    leds[findLED(k)] = CHSV(bgclr, 255, bgbri);                                        // First set a background colour, but fully saturated.
    leds[findLED(k)] += CHSV(thishue, allsat, thisbright);                             // Then assign a hue to any that are bright enough.
  }
  bgclr++;                                                                    // You can change the background colour or remove this and leave it fixed.
} // one_sine()

/*From  fill_grad
By: Andrew Tuline
Date: August, 2015*/

void blendme() {
  uint8_t starthue = beatsin8(20, 0, 255);
  uint8_t endhue = beatsin8(35, 0, 255);
  if (starthue < endhue) {
    fill_gradient(leds, NUM_LEDS, CHSV(starthue,255,255), CHSV(endhue,255,255), FORWARD_HUES);    // If we don't have this, the colour fill will flip around
  } else {
    fill_gradient(leds, NUM_LEDS, CHSV(starthue,255,255), CHSV(endhue,255,255), BACKWARD_HUES);
  }
} // blendme()

// patterns by Bruce

void averageFade() {
  int i,j,k,rs,gs,bs;
  CRGB returnVals[NUM_LEDS];
  // average node values with a slight negative bias for liquid fade
  // rs, gs, bs are sigmas across the averaged span
  // span must be odd otherwise it's asymmmetrical and weird things happen
  // yes, it's RGB, you HSV fans, can play with HSV later when there's more time
  for (j=0;j<NUM_LEDS;j++) {
    rs=0; gs=0; bs=0;
    for (k=j-global_span/2;k<(j+global_span/2)+1;k++) {
      if ((k>=0)&&(k<NUM_LEDS)) {
        rs+=leds[k].r; rs-=1; if(rs<0) rs=0;
        gs+=leds[k].g; gs-=1; if(gs<0) gs=0;
        bs+=leds[k].b; bs-=1; if(bs<0) bs=0;
      } else {
        rs+=leds[j].r; rs-=1; if(rs<0) rs=0;
        gs+=leds[j].g; gs-=1; if(gs<0) gs=0;
        bs+=leds[j].b; bs-=1; if(bs<0) bs=0;
      }
    }
    // now getting sigma/n for each channel
    // have to write to a buffer because it's not safe to tinker with leds[] yet
    returnVals[j].r=rs/global_span;
    returnVals[j].g=gs/global_span;
    returnVals[j].b=bs/global_span;
  }
  // completed the averaging, now it's safe to dump the results into leds[]
  for (j=0;j<NUM_LEDS;j++) {
    leds[j]=returnVals[j];
  }
  // and finally, a zombie door gated random node value set
  if (random(100)>global_gate) {
    i = random(NUM_LEDS-1); // because zero indexed :p
    leds[i].r=random(255);
    leds[i].g=random(255);
    leds[i].b=random(255);
  }
  FastLED.show();
}

// end patterns by Bruce


//helper functions

//all
//helper fucntion, sets entire ring to provided color in one frame.
void all(CRGB all_c){
  for(int i=0;i<NUM_LEDS;i++)
  {
    leds[i] = all_c;
  }
}

//lightLED
//helper function, used to light a specific LED from Serial input.
//sets entire ring to global_bg, then lights global_pos with global_fg
void lightLED()
{
  all(global_bg);
  leds[findLED(global_pos)] = global_fg;
  delay(1000);
}

//helper function to handle the second half of the ring being backwards from the first, since we're going out bidirectional from our teensy.
int findLED(int pos)
{
  //we want the first half of the ring to be normal, and the second half of the ring to be the distance from the end of the strip.
  //this flips the second half of the ring so that transitions from segment to segment are clean.
  //return pos;
  int new_pos;
  if(pos < (NUM_LEDS_LEFT))
  {
    return pos;
  }
  new_pos = NUM_LEDS  - (pos - NUM_LEDS_LEFT) - 1;
  return new_pos;
}

//nextPattern
//advances the gCurrentPatternNumber that's used to draw the current frame.
//On a rare dice roll we update some of the global values to keep things changing over time.
//can update global_fg, global_fg2, global_bg, global_wait.
void nextPattern()
{
  if(!global_lock)
  {
    /* disabling this while we build out code.  We will turn it back on for unattended ring, but it's causing problems in development.
    if(random(20) == 1)
    {
      global_fg = CRGB(random(255),random(255),random(255));
      global_fg2 = CRGB(random(255),random(255),random(255));      
      global_bg = CRGB(random(255),random(255),random(255));
      global_wait = random(1000);
    }
    */
    // add one to the current pattern number, and wrap around at the end
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
    Serial.print("Advancing to pattern ");
    Serial.println(patternNames[gCurrentPatternNumber]);    
  }else{
    Serial.println("Cannot advance to next pattern, pattern is currently locked.  Send l to unlock");
  }
}
