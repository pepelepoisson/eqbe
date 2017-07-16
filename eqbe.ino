// Arduino Built-in Library
#include<Wire.h>

// External Library
#include "painlessMesh.h"
#include "FastLED.h"
FASTLED_USING_NAMESPACE

// Project Library
#include "font-5x5.h"

// Configuration

#define BRIGHTNESS          30
#define FRAMES_PER_SECOND   60 // 125 frames/sec <=> 8 milli/frame

#define NUM_LEDS  25

#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

#define MIN_WAIT_SECOND 45  // Broadcast interval
#define MAX_WAIT_SECOND 90

// Setup

#define DATA_PIN    14
#define MOSFET_PIN  16

#define RED_PIN     15
#define GREEN_PIN   12
#define BLUE_PIN    13

#define PUSH_BUTTON_PIN 0

#define   MESH_PREFIX     "eqbe"
#define   MESH_PASSWORD   "PapasInventeurs"
#define   MESH_PORT       5858


#define MAX_FRAME_COUNT 160 // 160 * 8 milli = 1280 milli; Duration of transition effect is 1.3 second

enum patternStateEnum { NORMAL, TRANSITION, NEW_CONNECTION, CHANGED_CONNECTIONS, PATTERN_STATE_COUNT };
uint8_t gPatternState = NORMAL;

enum patternEnum { SOLID, RAINBOW, CONFETTI, SINELON, BPM, JUGGLE, LETTER, PATTERN_COUNT };
const char *patternName[PATTERN_COUNT] = { "solid", "rainbow", "confetti", "sinelon", "bpm", "juggle", "letter" };

enum riderEnum { SEBASTIEN, PASCAL, UNKNOWN_RIDER, RIDER_COUNT };
const char *rider[RIDER_COUNT] = { "Sebastien", "Pascal", "Unknown" };

CRGB leds[NUM_LEDS];

uint8_t gFrameCount = 0; // Inc by 1 for each Frame of Trasition, New/Changed connection(s) pattern
uint8_t gHue = 0;
void (*gActivePattern)();
void (*gSelectedPattern)();
bool gIsOurSentPattern = true;

// Variables used in CheckAccel() routine
uint8_t MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; //These will be the raw data from the MPU6050.;
#define ACCELEROMETER_ORIENTATION 4     // 0, 1, 2, 3 or 4 to set the orientation of the accerometer module
int a_forward=0,a_sideway=0,a_vertical=0;
enum FACES { FACE_NONE, FACE_1, FACE_2, FACE_3, FACE_4, FACE_5, FACE_6, FACES_COUNT };
const char *faceName[FACES_COUNT] = { "None", "Face 1", "Face 2", "Face 3", "Face 4", "Face 5 (LEDs up)", "Face 6 (LEDs down)" };
uint8_t orientation = FACE_NONE;

void solid()
{
  // FastLED's built-in rainbow generator
  fill_solid( leds, NUM_LEDS, CHSV(gHue, 255, 255));
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
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
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void letter() {
  drawLetter('D');
}

void transitionPulse() {
  gFrameCount += 1;

  uint8_t hue = gIsOurSentPattern ? HUE_AQUA : HUE_ORANGE;

  uint8_t value = 0;
  if (gFrameCount < MAX_FRAME_COUNT - 45) { // Tiny black interval between the pulse and next pattern
    value = quadwave8(gFrameCount * 2);
  }

  fill_solid(leds, NUM_LEDS, CHSV(hue, 255, value));
}

void glitter() {
  gFrameCount += 1;

  if (gFrameCount > MAX_FRAME_COUNT - 45) { // Tiny black interval between the pulse and next pattern
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  } else {
    if (gFrameCount % 4 == 1) { // Slow down frame rate
      for ( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(0, 0, random8() < 60 ? random8() : random8(64));
      }
    }
  }
}

void redGlitter() {
  gFrameCount += 1;

  if (gFrameCount > MAX_FRAME_COUNT - 45) { // Tiny black interval between the pulse and next pattern
    fill_solid(leds, NUM_LEDS, CRGB::Black);
  } else {
    if (gFrameCount % 4 == 1) { // Slow down frame rate
      for ( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(HUE_RED, 0, random8() < 60 ? random8() : random8(64));
      }
    }
  }
}

painlessMesh  mesh;

int8_t nodeId2riderIdx(uint32_t nodeId) {
  switch (nodeId) {
    case 0xa6182c9a:
      return SEBASTIEN;
    case 0x7f0fb8c3:
      return PASCAL;
    default:
      return UNKNOWN_RIDER;
  }
}

void setSelectedPattern(uint8_t patternIdx) {
  switch (patternIdx) {
    case SOLID:
      gSelectedPattern = solid;
      break;
    case RAINBOW:
      gSelectedPattern = rainbow;
      break;
    case CONFETTI:
      gSelectedPattern = confetti;
      break;
    case SINELON:
      gSelectedPattern = sinelon;
      break;
    case BPM:
      gSelectedPattern = bpm;
      break;
    case JUGGLE:
      gSelectedPattern = juggle;
      break;
    case LETTER:
      gSelectedPattern = letter;
      break;
    default:
      Serial.printf("ERROR! Unknown Pattern index, %d\n", patternIdx);
  }
}

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, []() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& msg = jsonBuffer.createObject();

  uint8_t patternIdx = LETTER; // random(0, PATTERN_COUNT);
  setSelectedPattern(patternIdx);
  msg["pattern"] = patternIdx;
  Serial.printf("  Setting my pattern to %s (index: %d)\n", patternName[patternIdx], patternIdx);

  gHue = random(0, 255);
  msg["hue"] = gHue;
  Serial.printf("  Setting my hue to %d\n", gHue);

  gIsOurSentPattern = true;

  String str;
  msg.printTo(str);
  mesh.sendBroadcast(str);
  taskSendMessage.setInterval( random( TASK_SECOND * MIN_WAIT_SECOND, TASK_SECOND * MAX_WAIT_SECOND ));

  gFrameCount = 0;
  gPatternState = TRANSITION;
});

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Message %s\n", msg.c_str());

  if (mesh.getNodeId() == from) {
    Serial.printf("  Sent from ourself! Going to ignore this message\n");
    return;
  }

  int8_t riderIdx = nodeId2riderIdx(from);
  Serial.printf("  Sent from rider %s (nodeId: 0x%08x)\n", rider[riderIdx], from);

  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(msg);
  if (root.containsKey("pattern")) {
    uint8_t patternIdx = root["pattern"];
    Serial.printf("  Set my pattern to %s (index: %d)\n", patternName[patternIdx], patternIdx);
    setSelectedPattern(patternIdx);

    gFrameCount = 0;
    gPatternState = TRANSITION;
    gIsOurSentPattern = false;

    // Reset the wait after message reception
    taskSendMessage.setInterval( random( TASK_SECOND * MIN_WAIT_SECOND, TASK_SECOND * MAX_WAIT_SECOND ));
  }

  if (root.containsKey("hue")) {
    gHue = root["hue"];
    Serial.printf("  Set my hue to %d\n", gHue);
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection\n");
  Serial.printf("  Remote module ==> NodeId: 0x%08x, Rider: %s\n", nodeId, rider[nodeId2riderIdx(nodeId)]);

  gFrameCount = 0;
  gPatternState = NEW_CONNECTION;
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  Serial.printf("  %s\n", mesh.subConnectionJson().c_str());

  gFrameCount = 0;
  gPatternState = CHANGED_CONNECTIONS;
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time\n");
  Serial.printf("  Node time: %u, Offset: %d\n", mesh.getNodeTime(), offset);
}

void updatePattern () {
  void (*currentPattern)() = gActivePattern;

  // Check if transitive pattern cycle is completed
  if (gPatternState != NORMAL && gFrameCount > MAX_FRAME_COUNT) {
    gPatternState = NORMAL;
  }

  switch (gPatternState) {
    case NORMAL:
      gActivePattern = gSelectedPattern;
      break;
    case TRANSITION:
      gActivePattern = transitionPulse;
      break;
    case NEW_CONNECTION:
      gActivePattern = glitter;
      break;
    case CHANGED_CONNECTIONS:
      gActivePattern = redGlitter;
      break;
    default:
      Serial.printf("Unknow pattern state idx %d\n", gPatternState);
  }

  // Force an update for newly active pattern
  if (currentPattern != gActivePattern) {
    gActivePattern();
    FastLED.show();
  }
}

bool wasPressed;
void doButton() {
  bool isPressed = (digitalRead(PUSH_BUTTON_PIN) == LOW);

  if (isPressed && !wasPressed) { // Press transition
    wasPressed = true;
    Serial.println("[x] Push button pressed"); // Transition

    taskSendMessage.forceNextIteration();
  } else if (!isPressed && wasPressed) { // Unpress transition
    wasPressed = false;
    // Serial.println("[-] Push button released transition !!!");
  } else if (isPressed) {
    // Button is press (PRESS-STATE)
    // Serial.println("[_] Push button is pressed.");
  } else {
    // Serial.println("[^] Push button is released.");
  }
}

void setup() {
  Serial.begin(9600);

  delay(250);

  Serial.printf("\nBegin Setup.\n");

  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, HIGH);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CHSV(HUE_GREEN, 255, 100));
  FastLED.show();
  
// Set up MPU 6050:
  Wire.begin();
  #if ARDUINO >= 157
    Wire.setClock(400000UL); // Set I2C frequency to 400kHz
  #else
    TWBR = ((F_CPU / 400000UL) - 16) / 2; // Set I2C frequency to 400kHz
  #endif

  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  
  wasPressed = false;
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);

  gPatternState = NORMAL;
  gActivePattern = solid;
  gSelectedPattern = solid;

  randomSeed( analogRead( A0 ) );

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  mesh.scheduler.addTask( taskSendMessage );
  taskSendMessage.enable();

  uint32_t nodeId = mesh.getNodeId();
  Serial.printf("This module ==> NodeId: 0x%08x, Rider: %s\n", nodeId, rider[nodeId2riderIdx(nodeId)]);

  Serial.printf("Setup completed. %d\n", NUM_LEDS);
}

void loop() {
  mesh.update();

  doButton();

  updatePattern();

  EVERY_N_MILLISECONDS( 500 ) {
    CheckAccel();
  }

  EVERY_N_MILLISECONDS( 15 ) {
    if (gPatternState == NORMAL) {
      gHue += 1;  // slowly cycle the "base color" through the rainbow
    }
  }

  EVERY_N_MILLISECONDS( 1000 / FRAMES_PER_SECOND ) {
    gActivePattern();
    FastLED.show();
  }
}

void CheckAccel(){
  // Reads acceleration from MPU6050 to evaluate current condition.
  // Tunables:
  // Output values: still, cruising, braking, fallen, unknown
  // Get accelerometer readings
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,(size_t)14,true);  // request a total of 14 registers
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  // Convert to expected orientation - includes unit conversion to "cents of g" for MPU range set to 2g
  a_forward = (ACCELEROMETER_ORIENTATION == 0?-AcX:(ACCELEROMETER_ORIENTATION == 1?-AcX:(ACCELEROMETER_ORIENTATION == 2?-AcX:(ACCELEROMETER_ORIENTATION == 3?-AcY:AcY))))/164.0;
  a_sideway = (ACCELEROMETER_ORIENTATION == 0?AcY:(ACCELEROMETER_ORIENTATION == 1?AcZ:(ACCELEROMETER_ORIENTATION == 2?-AcZ:(ACCELEROMETER_ORIENTATION == 3?AcZ:-AcZ))))/164.0;
  a_vertical = (ACCELEROMETER_ORIENTATION == 0?AcZ:(ACCELEROMETER_ORIENTATION == 1?-AcY:(ACCELEROMETER_ORIENTATION == 2?AcY:(ACCELEROMETER_ORIENTATION == 3?AcX:AcX))))/164.0;

  Serial.printf("f:%d s:%d v:%d\n", a_forward,a_sideway,a_vertical);

  // Detect cube orientation - Current test works with ACCELEROMETER_ORIENTATION = 4
  orientation=FACE_NONE;
  if (a_vertical >= 80 && abs(a_forward) <= 25 && abs(a_sideway) <= 25) {
    // coté 1 en haut
    orientation = FACE_1;
  }
  if (a_forward >= 80 && abs(a_vertical) <= 25 && abs(a_sideway) <= 25) {
    // coté 2 en haut
    orientation = FACE_2;
  }
  if (a_vertical <= -80 && abs(a_forward) <= 25 && abs(a_sideway) <= 25) {
    // coté 3 en haut
    orientation = FACE_3;
  }
  if (a_forward <= -80 && abs(a_vertical) <= 25 && abs(a_sideway) <= 25) {
    // coté 4 en haut
    orientation = FACE_4;
  }
  if (a_sideway >= 80 && abs(a_vertical) <= 25 && abs(a_forward) <= 25) {
    // coté LEDs en haut
    orientation = FACE_5;
  }
  if (a_sideway <= -80 && abs(a_vertical) <= 25 && abs(a_forward) <= 25) {
    // coté batteries en haut
    orientation = FACE_6;
  }

  Serial.printf("Orientation: %s\n", faceName[orientation]);
}

void drawLetter(char c) {
  uint8_t idx = c - 'A';
  uint32_t letterA = font_5x5[idx];
  uint32_t singleBit = 1;

  // FIXME: Use font size instead of magic 5*5
  for ( uint8_t i = 0; i < 25; i++) {
    leds[i] = CHSV(gHue, 255, letterA & singleBit ? 255 : 0);
    singleBit <<= 1;
  }
}
