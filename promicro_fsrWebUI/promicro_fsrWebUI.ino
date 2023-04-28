#include <Wire.h>
#include <EEPROM.h>
#include <Joystick.h>

Joystick_ Joystick(0x09, 0x04, 4, 0, false, false, false, false, false, false, false, false, false, false, false);

char instruction;
byte highB;
byte lowB;

//array for digital pins linked to PCB LEDs
int pcbLED[4] = {4,5,6,7};

//array for last and currrent states
int lastState[4] = {0,0,0,0};
int currentState[4] = {0,0,0,0};

//containers for analog values
unsigned int a0read = 0;
unsigned int a1read = 0;
unsigned int a2read = 0;
unsigned int a3read = 0;
//containers for thresholds
unsigned int a0thresh = 0;
unsigned int a1thresh = 0;
unsigned int a2thresh = 0;
unsigned int a3thresh = 0;

//padding value
unsigned int pad = 10;

//booleans for updating eeprom
bool a0write = false;
bool a1write = false;
bool a2write = false;
bool a3write = false;

//array for read values
byte readBytes[8];
//array for threshold values
byte threshBytes[8];
//array for values to be written to thresholds
byte writeBytes[8];

// BEGIN FASTLED SETUP
#include <FastLED.h>
#define NUM_PANELS 4
#define NUM_LEDS 30
#define DATA_PIN 9
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define BRIGHTNESS 255 //255
CRGB leds[NUM_LEDS];

// This is a map of panel leds. 
// Column 0 is the button number
// Column 1 is the number of LEDs in the panel strip
// Column 3 is the LED offset of the first LED in the strip 
int16_t PANEL_LED[4][3] = {
  {0, 7, 22},  // Left
  {1, 8, 14},  // Down
  {2, 7, 0 },  // Up
  {3, 7, 7 },  // Right
};

// Set the color profile for the lights
int COLOR_PROFILE = 1;
int NUM_COLORS = 0;

// Re-usable palletes
CRGB ALL_WHITE[4] = {CRGB::White,  CRGB::White, CRGB::White, CRGB::White};
CRGB ALL_BLACK[4] = {CRGB::Black,  CRGB::Black, CRGB::Black, CRGB::Black};
CRGB ALL_RED[4]   = {CRGB::Red,    CRGB::Red,   CRGB::Red,   CRGB::Red};
CRGB ALL_BLUE[4]  = {CRGB::Blue,   CRGB::Blue,  CRGB::Blue,  CRGB::Blue};
CRGB ALL_GREEN[4] = {CRGB::Green,  CRGB::Green, CRGB::Green, CRGB::Green};
CRGB ALL_GOLD[4]  = {CRGB::Gold,   CRGB::Gold,  CRGB::Gold,  CRGB::Gold};

// Idle lights, light up when the panel isn't being pressed
CRGB IDLE_COLORS[][4] = {
               // Left,              Up,               Down,             Right
  /* 0  Test   */ {CRGB::Gold,       CRGB::Red,        CRGB::Green,      CRGB::Purple},
  /* 1  ITG    */ {CRGB::Blue,       CRGB::Red,        CRGB::Red,        CRGB::Blue},
  /* 2  DDR    */ {CRGB::DeepSkyBlue,CRGB::DeepPink,   CRGB::DeepPink,   CRGB::DeepSkyBlue},
  /* 3  Brazil */ {CRGB::Gold,       CRGB::Gold,       CRGB::Gold,       CRGB::Gold},
  /* 4  Frozen */ {CRGB::White,      CRGB::White,      CRGB::White,      CRGB::White},
  /* 5  Italy  */ {CRGB::Green,      CRGB::White,      CRGB::White,      CRGB::Red},
  /* 6  One    */ {CRGB::Black,      CRGB::Black,      CRGB::Black,      CRGB::Black},
  /* 7  Prncss */ {CRGB::MediumPurple,CRGB::MediumPurple,CRGB::MediumPurple,CRGB::MediumPurple},
  /* 8  Navi   */ {CRGB::Blue,       CRGB::Blue,       CRGB::Blue,       CRGB::Blue},
  /* 9  USA    */ {CRGB::Red,        CRGB::Red,        CRGB::Red,        CRGB::Red},
  /* 10 Y->BLK */ ALL_GOLD,
  /* 11 R->BLK */ ALL_RED,
  /* 12 B->BLK */ ALL_BLUE,
  /* 13 G->BLK */ ALL_GREEN,
  /* 14 W->BLK */ ALL_WHITE,
  /* 15 BLK->W */ ALL_BLACK,
  /* 16 BLK->R */ ALL_BLACK,
  /* 17 BLK->B */ ALL_BLACK,
  /* 18 BLK->G */ ALL_BLACK,
};

// Active lights, light up when the panel is pressed
CRGB ACTIVE_COLORS[][4] = {
               // Left,              Up,               Down,             Right
  /* 0  Test   */ ALL_WHITE, 
  /* 1  ITG    */ ALL_WHITE, 
  /* 2  DDR    */ ALL_WHITE, 
  /* 3  Brazil */ ALL_WHITE, 
  /* 4  Frozen */ {CRGB::Blue,       CRGB::Blue,       CRGB::Blue,       CRGB::Blue}, 
  /* 5  Italy  */ {CRGB::Blue,       CRGB::Blue,       CRGB::Blue,       CRGB::Blue}, 
  /* 6  One    */ ALL_WHITE, 
  /* 7  Prncss */ {CRGB::Gold,       CRGB::Gold,       CRGB::Gold,       CRGB::Gold}, 
  /* 8  Navi   */ ALL_WHITE,
  /* 9  USA    */ {CRGB::Blue,       CRGB::White,       CRGB::White,       CRGB::Blue},
  /* 10 Y->BLK */ ALL_BLACK,
  /* 11 R->BLK */ ALL_BLACK,
  /* 12 B->BLK */ ALL_BLACK,
  /* 13 G->BLK */ ALL_BLACK,
  /* 14 W->BLK */ ALL_BLACK,
  /* 15 BLK->W */ ALL_WHITE,
  /* 16 BLK->R */ ALL_RED,
  /* 17 BLK->B */ ALL_BLUE,
  /* 18 BLK->G */ ALL_GREEN,
};

void setIdleColors() {
  // Set each light to its idle color
  for( int k=0; k < NUM_PANELS; k++) {
    idlePanelLEDs(k);
  }
}

void idlePanelLEDs(int k) {
  int panel = PANEL_LED[k][0];
  int num_panel_leds = PANEL_LED[k][1];
  int start_index = PANEL_LED[k][2];
  int led_index = 0;
  for( int  j=0; j < num_panel_leds; j++) {
    led_index = j + start_index;
    leds[led_index] = IDLE_COLORS[COLOR_PROFILE][panel];
  }
  FastLED.show();
}

void activePanelLEDs(int k) {
  int panel = PANEL_LED[k][0];
  int num_panel_leds = PANEL_LED[k][1];
  int start_index = PANEL_LED[k][2];
  int led_index = 0;
  for( int  j=0; j < num_panel_leds; j++) {
    led_index = j + start_index;
    leds[led_index] = ACTIVE_COLORS[COLOR_PROFILE][panel];
  }
  FastLED.show();
}

void setup() {
  //get number of possible panel colors
  NUM_COLORS = sizeof(IDLE_COLORS)/sizeof(IDLE_COLORS[0]);
  //setup and turn off PCB LEDs
  for (int i=0; i<sizeof pcbLED; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  // Add the LEDs
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);   
  FastLED.setBrightness(BRIGHTNESS);
  // Set each light to its idle color on start
  setIdleColors();
  Serial.begin(115200);           // start serial for output
  a0thresh = (EEPROM.read(1)<<8) | EEPROM.read(0);
  a1thresh = (EEPROM.read(3)<<8) | EEPROM.read(2);
  a2thresh = (EEPROM.read(5)<<8) | EEPROM.read(4);
  a3thresh = (EEPROM.read(7)<<8) | EEPROM.read(6);
  Wire.begin(8); // join bus as 8
  Wire.onReceive(receiveEvent); // register receive event
  Wire.onRequest(requestEvent); // register request event
  Joystick.begin();
}

void loop() {
  
  a0read = (unsigned int)analogRead(A0);
  a1read = (unsigned int)analogRead(A1);
  a2read = (unsigned int)analogRead(A2);
  a3read = (unsigned int)analogRead(A3);
  readBytes[0] = lowByte(a0read);
  readBytes[1] = lowByte((unsigned int)a0read>>8);
  readBytes[2] = lowByte(a1read);
  readBytes[3] = lowByte((unsigned int)a1read>>8);
  readBytes[4] = lowByte(a2read);
  readBytes[5] = lowByte((unsigned int)a2read>>8);
  readBytes[6] = lowByte(a3read);
  readBytes[7] = lowByte((unsigned int)a3read>>8);

  if (a0write == true) {
    EEPROM.update(0, lowByte(a0thresh));
    EEPROM.update(1, lowByte((unsigned int)a0thresh>>8));
    Serial.println("A0 thresh wrote to EEPROM is: "+String(a0thresh));
    a0write=false;
  }
  if (a1write == true) {
    EEPROM.update(2, lowByte(a1thresh));
    EEPROM.update(3, lowByte((unsigned int)a1thresh>>8));
    Serial.println("A1 thresh wrote to EEPROM is: "+String(a1thresh));
    a1write=false;
  }
  if (a2write == true) {
    EEPROM.update(4, lowByte(a2thresh));
    EEPROM.update(5, lowByte((unsigned int)a2thresh>>8));
    Serial.println("A2 thresh wrote to EEPROM is: "+String(a2thresh));
    a2write=false;
  }
  if (a3write == true) {
    EEPROM.update(6, lowByte(a3thresh));
    EEPROM.update(7, lowByte((unsigned int)a3thresh>>8));
    Serial.println("A3 thresh wrote to EEPROM is: "+String(a3thresh));
    a3write=false;
  }
  //Serial.println(a0read);

  if (a0read > a0thresh) {
    Joystick.setButton(0, 1);
    digitalWrite(pcbLED[0], HIGH);
    activePanelLEDs(0);
  }
  else if (a0read < a0thresh - pad) {
    Joystick.setButton(0, 0);
    digitalWrite(pcbLED[0], LOW);
    idlePanelLEDs(0);
  }
  if (a1read > a1thresh) {
    Joystick.setButton(1, 1);
    digitalWrite(pcbLED[1], HIGH);
    activePanelLEDs(1);
  }
  else if (a1read < a1thresh - pad) {
    Joystick.setButton(1, 0);
    digitalWrite(pcbLED[1], LOW);
    idlePanelLEDs(1);
  }
  if (a2read > a2thresh) {
    Joystick.setButton(2, 1);
    digitalWrite(pcbLED[2], HIGH);
    activePanelLEDs(2);
  }
  else if (a2read < a2thresh - pad) {
    Joystick.setButton(2, 0);
    digitalWrite(pcbLED[2], LOW);
    idlePanelLEDs(2);
  }
  if (a3read > a3thresh) {
    Joystick.setButton(3, 1);
    digitalWrite(pcbLED[3], HIGH);
    activePanelLEDs(3);
  }
  else if (a3read < a3thresh - pad) {
    Joystick.setButton(3, 0);
    digitalWrite(pcbLED[3], LOW);
    idlePanelLEDs(3);
  }

  
  delay(1);
    

}

// event receiving 
void receiveEvent(size_t howMany) {
  (void) howMany;
  instruction = Wire.read();    // receive byte as a char
}
//event requesting
void requestEvent() {
  //can't use serial in an ISR
  //Serial.println("instruction is: "+String(instruction)); // print the integer
  
  if (instruction == 'r')
  {
    Wire.write("r");
    for (int i=0; i<sizeof(readBytes); i=i+1) {
      Wire.write(readBytes[i]);
    }
  }
  else if (instruction == 't')
  {
    threshBytes[0] = lowByte(a0thresh);
    threshBytes[1] = lowByte((unsigned int)a0thresh>>8);
    threshBytes[2] = lowByte(a1thresh);
    threshBytes[3] = lowByte((unsigned int)a1thresh>>8);
    threshBytes[4] = lowByte(a2thresh);
    threshBytes[5] = lowByte((unsigned int)a2thresh>>8);
    threshBytes[6] = lowByte(a3thresh);
    threshBytes[7] = lowByte((unsigned int)a3thresh>>8);
    Wire.write("t");
    for (int i=0; i<sizeof(threshBytes); i=i+1) {
      Wire.write(threshBytes[i]);
    }
    //can't use Serial in an ISR
    //Serial.println("Read thresholds: "+String(a0thresh)+" "+String(a1thresh)+" "+String(a2thresh)+" "+String(a3thresh)); 

      
  }
  else if (instruction == 'w')
  {
    lowB = Wire.read(); // receive a byte as character
    highB = Wire.read();
    if (a0thresh != ((highB<<8) | lowB)) { a0write = true; }
    a0thresh = (highB<<8) | lowB;
    lowB = Wire.read(); // receive a byte as character
    highB = Wire.read();
    if (a1thresh != ((highB<<8) | lowB)) { a1write = true; }
    a1thresh = (highB<<8) | lowB;
    lowB = Wire.read(); // receive a byte as character
    highB = Wire.read();
    if (a2thresh != ((highB<<8) | lowB)) { a2write = true; }
    a2thresh = (highB<<8) | lowB;
    lowB = Wire.read(); // receive a byte as character
    highB = Wire.read();
    if (a3thresh != ((highB<<8) | lowB)) { a3write = true; }
    a3thresh = (highB<<8) | lowB;
    Wire.write("w");
    //can't use Serial in an ISR
    //Serial.println("Write thresholds: "+String(a0thresh)+" "+String(a1thresh)+" "+String(a2thresh)+" "+String(a3thresh));;

  }
  else if (instruction == 'p')
  {
    if (COLOR_PROFILE == 0) {
      COLOR_PROFILE=(NUM_COLORS-1);
    }
    else {
      COLOR_PROFILE=COLOR_PROFILE-1;
    }
    //setIdleColors();
    Wire.write("p");
    //can't use Serial in an ISR
  }
  else if (instruction == 'n')
  {
    if (COLOR_PROFILE == (NUM_COLORS-1)) {
      COLOR_PROFILE=0;
    }
    else {
      COLOR_PROFILE=COLOR_PROFILE+1;
    }
    Wire.write("n");
    //can't use Serial in an ISR
  }
  else 
  {
    //Serial.println("unknown instruction");
  }
  
}
