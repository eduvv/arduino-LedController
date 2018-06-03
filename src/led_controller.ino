#include <Wire.h>    
#include <U8g2lib.h> // https://github.com/olikraus/u8g2
#include <U8x8lib.h> 
#include <FastLED.h> // https://github.com/FastLED/FastLED

#define NUMITEMS(arg)((unsigned int)(sizeof(arg) / sizeof(arg[0]))) //calculates #items in an array

#define DATA_PIN                5 // datapin of the ledstrip
#define BUTTON_PIN              4 // pushbutton to change cursor
#define LED_TYPE          WS2812B 
#define COLOR_ORDER           GRB // Green Red Blue
#define NUM_LEDS               64 // how many LEDs you have
#define BRIGHTNESS             96 // 0-255, higher number is brighter.
#define STEPS                   5 // How wide the bands of color are.  1 = more like a gradient, 10+ = more like stripes
#define BLEND_TYPE    LINEARBLEND 

CRGB leds[NUM_LEDS];
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 3, /* data=*/ 2);

const String modes[] = {
    "SOLID",
    "FADE",
    "RAINBOW",
    "BLEED"
}; 
String colors[] = {
    "RED",
    "ORANGE",
    "YELLOW",
    "GREEN",
    "BLUE",
    "INDIGO",
    "VIOLET"
};

CRGB solidColor[] = {
  CRGB::Red,
  CRGB::Orange,
  CRGB::Yellow,
  CRGB::Green,
  CRGB::Blue,
  CRGB::Indigo,
  CRGB::Violet
};

const TProgmemPalette16 PurpleColors_p PROGMEM = {
  CRGB::Green,
  CRGB::Green,
  CRGB::Green,
  CRGB::Green,

  CRGB::Yellow,
  CRGB::Yellow,
  CRGB::Green,
  CRGB::Green,

  CRGB::Green,
  CRGB::Green,
  CRGB::Green,
  CRGB::Green,

  CRGB::Black, 
  CRGB::Violet,
  CRGB::MediumVioletRed, 
  CRGB::MediumVioletRed 
};

bool buttonState      = 0;
bool lastButtonState  = 0;
byte cursorPosition   = 0;
byte modesPosition    = 0;
byte colorsPosition   = 0;
uint16_t potVal       = 0;
byte progSpeed        = 100;
byte progBright       = 100;

static const byte NUM_READINGS  = 10;
uint16_t _smooth[NUM_READINGS];
uint16_t _smooth_readIndex      = 0; // the index of the current reading
uint16_t _smooth_total          = 0; // the running total
uint16_t _smooth_average        = 0;  

byte prevCursorPosition         = -1;
uint16_t prevPotVal             = -1;
bool BUTTON_CHANGED             = true;

static const int MAX_POT_VALUE  = 900;
static const int MIN_POT_VALUE  = 100;

int counter = 0;
int prevCounter = 0;

void setup() {
    delay(3000); //start delay
    pinMode(A0, INPUT); //potentiometer
    pinMode(BUTTON_PIN, INPUT_PULLUP); //pushbutton
    digitalWrite(BUTTON_PIN, HIGH);

    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    u8g2.begin(); //start lcd monitor
    Serial.begin(9600);
    drawOLED();
}

void loop() {
    potVal = readSmoothPotValue2();
    //Serial.println(potVal);
    moveCursor();
    ledCycle();
    drawOLED();
}

/**
 * Smooths out inputs of potentiometer by taking the average of NUM_READINGS
 * @return The smoothed value
*/
uint16_t readSmoothPotValue2(){
    uint16_t total = 0;
    for(int i = 0; i<NUM_READINGS;i++){
        total += analogRead(A0);
    }
    total = total/NUM_READINGS;
    return total;
}

/**
 * Checks if potentiometer has changed by a certain factor.
 * @param factor The factor by which this method should return true
 * @return true if the potentiometer changed by the given factor, false if not.
*/
bool changeInPotValue(byte factor){
    if(potVal >= prevPotVal + factor || potVal <= prevPotVal - factor){
        prevPotVal = potVal;
        return true;
    }else return false;
}

void drawOLED() {
    if(counter == prevCounter && !changeInPotValue(30)){
        return;
    }
    //Serial.println("OLED UPDATED");
    prevCounter = counter;

    //shitty C bypass to get u8g2 function to work with char*
    String modStr = "MODE: " + modes[modesPosition];
    String colStr = (modesPosition == 1 || modesPosition == 2) ? "COLOR: MIXED" : "COLOR: " + colors[colorsPosition];

    const char * modTxt = modStr.c_str();
    const char * colTxt = colStr.c_str();

    String cyclStr = "SPEED: ";
    cyclStr.concat(progSpeed);
    cyclStr.concat("%");
    String briStr = "BRIGHTNESS: ";
    briStr.concat(progBright);
    briStr.concat("%");

    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_9x15_t_symbols);
        drawCursor(cursorPosition);
        u8g2.drawStr(11, 15, modTxt);
        u8g2.drawStr(11, 31, colTxt);

        u8g2.drawFrame(11, 35, 117, 12);
        drawProgressBar(11, 35, 117, 12, progSpeed, "SPEED: ");

        u8g2.drawFrame(11, 52, 117, 12);
        drawProgressBar(11, 52, 117, 12, progBright, "BRIGHTNESS: ");
        BUTTON_CHANGED = false;
    } while (u8g2.nextPage());
}

/**
 * Chooses which row of the menu is selected by the position of the cursor.
 * Depending on what mode was currently selected (-> MODE:) it will run the right mode.
*/
void ledCycle(){
    //Calculate cursorPosition from potentiometer
    switch (cursorPosition) {
        case 0:
            modesPosition = map(potVal, 0, MAX_POT_VALUE, 0, NUMITEMS(modes) - 1);
            break;
        case 1:
            colorsPosition = map(potVal, 0, MAX_POT_VALUE, 0, NUMITEMS(colors) - 1);
            break;
        case 2:
            progSpeed = map(potVal, 0, MAX_POT_VALUE, 0.5, 100);
            break;
        case 3:
            progBright = map(potVal, 0, MAX_POT_VALUE, 0, 100);
            break;
    } 
    static uint8_t startIndex = 0;
    //invoke right method
    switch(modesPosition){
        case 0: mode_solid();
            break;
        case 1: 
            startIndex = startIndex + 1;
            mode_fade(startIndex);
            break;
        case 2: 
            static uint8_t startIndex2 = 0;
            startIndex2 = startIndex2 + 1;
            mode_rainbow(startIndex2);
            break;
        case 3:
            mode_bleed();
            break;
    }

    FastLED.show(); 
    Serial.println(constrain(map(progBright,0,100,0,254), 0, 254));
    FastLED.setBrightness(constrain(map(progBright,0,100,0,254), 0, 254));
    FastLED.delay(1000/(progSpeed*5));
}

/**
 * Fades between 2 color by a given percentage.
 * @param from The starting color you want to start fading from.
 * @param to The ending color you want your color to fade towards.
 * @param percentage The pecentage you want to change. must be 0-100: 0 is closer to 'startColor', 100 is closer to 'endColor'
 * @return The faded color.
*/
CRGB colorFade(CRGB from, CRGB to, byte percentage){
    if(percentage > 100){
        percentage = 100;
    }
    float _percentage = percentage/100;

    byte fadeR = from.red * _percentage + to.red * (1-_percentage);
    byte fadeG = from.green * _percentage + to.green * (1-_percentage);
    byte fadeB = from.blue * _percentage + to.blue * (1-_percentage);

    return CRGB(fadeR,fadeG,fadeB);
}

/**
 * Outputs a constant solid color.
*/
void mode_solid(){
    fill_solid(leds, NUM_LEDS, solidColor[colorsPosition]); 
    // for( int i = 0; i < NUM_LEDS; i++) {
    //     leds[i] = solidColor[colorsPosition];            
    // }
}
/**
 * Outputs all rainbowcolors being faded.
 * @param colorIndex The index from which color it should start.
*/
void mode_fade(uint8_t colorIndex){
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette(RainbowColors_p, colorIndex, progBright, BLEND_TYPE);              
    }
    colorIndex += STEPS;
}
/**
 * Outputs all rainbowcolors in a train-like effect.
 * @param colorIndex The index from which color it should start.
*/
void mode_rainbow(uint8_t colorIndex){
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette(RainbowColors_p, colorIndex, progBright, BLEND_TYPE);
        colorIndex += STEPS;              
    }
}

int brightness = 0;
int fadeAmount = 1;
/**
 * Outputs a solid color that goes from dim to bright in a beat-like effect.
*/
void mode_bleed(){
    //int _speed = map(progSpeed,0,100,0.7,1);
    mode_solid();
    fadeLightBy(leds,NUM_LEDS,brightness);

    brightness = brightness + fadeAmount;
    if(brightness <= 0 || brightness >= 255){
        fadeAmount = -fadeAmount ; 
    }  
    delay(9);
}

/**
 * Moves the cursor downwards if the button on BUTTON_PIN has been pushed.
*/
void moveCursor() {
    buttonState = digitalRead(BUTTON_PIN);
    if (buttonState != lastButtonState) {
        if (buttonState == LOW) {
            BUTTON_CHANGED = true;
            counter++;
            cursorPosition++;
            if (cursorPosition >= 4)
                cursorPosition = 0;
        } else {
            //Serial.println("button released");
        }
        delay(50); //software 'debounce'
    }
    lastButtonState = buttonState;
}

/**
 * Draws a progress bar on the SSD1306 display.
 * @param x The X-coordinate where the progress bar should begin. (=bottom left corner)
 * @param y The Y-coordinate where the progress bar should begin. (=bottom left corner)
 * @param width The width of the progress bar in pixels.
 * @param height The height of the progress bar in pixels.
 * @param progress The precentage of completion of the progressbar, should be 0-100.
 * @param text The text you want to be displayed inside the progressbar.
*/
void drawProgressBar(int x, int y, int width, int height, int progress, String text) {
    u8g2.setFont(u8g2_font_5x7_tf);

    progress = progress > 100 ? 100 : progress;
    progress = progress < 0 ? 0 : progress;

    float bar = ((float)(width - 4) / 100) * progress;
    u8g2.drawBox(x + 2, y + 2, bar, height - 4);

    text.concat(progress);
    text.concat("%");
    const char * txt = text.c_str();

    u8g2.setDrawColor(2);
    u8g2.setFontMode(1);
    u8g2.drawStr((width / 2) - text.length() * 1.4, y + 9, txt);
}

/**
 * Draws the cursor/pointer in front of every menu entry, 'hard'coded for a 128x64 display with 4 menu rows.
 * @param pos Position where cursor should be at = row position of the menu, should be 0-3 for a 4-row menu.
*/
void drawCursor(int pos) {
    u8g2.drawGlyph(2, 15 + pos * 16, 8594);
}