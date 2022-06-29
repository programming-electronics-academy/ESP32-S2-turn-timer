/**

      If button held -> set flag to set time
*/


#include <FastLED.h>

//pins
const byte DATA_PIN = 11; // neo-pixel data pin
const unsigned int ROTARY_ENC_PIN_A = 33;
const unsigned int ROTARY_ENC_PIN_B = 34;
const unsigned int ROTARY_ENC_SWITCH = 21;

//Rotary Encoder States
#define NO_CHANGE 0
#define TURN_CW   1
#define TURN_CCW  2

//Direction     ┌─  ccw  ─┐  N  ┌─  cw  ─┐
//position       0  1  2  3  4  5  6  7  8
byte aState[] = {3, 2, 0, 1, 3, 2, 0, 1, 3};
byte lastState = 3;
volatile int count = 0;
unsigned int position = 4;
volatile byte encoderStatus = NO_CHANGE;

// LED array
const byte NUM_LEDS = 12;
CRGB leds[NUM_LEDS];

// Colors used when setting time.  Color ref: https://github.com/FastLED/FastLED/wiki/FastLED-HSV-Colors
const CHSV UNCOUNTED_COLOR = CHSV(0, 255, 255);          //Red
const CHSV SECONDS_COUNTED_COLOR = CHSV(160, 255, 255);  //Blue
const CHSV MINUTES_COUNTED_COLOR = CHSV(96, 255, 255);   // Green
const CHSV HOURS_COUNTED_COLOR = CHSV(64, 255, 255);     // Yellow
const CHSV RESET_COLOR = CHSV(213, 255, 255);     // Purple

// Colors / "Hue" used durring each turn (0-255)
const int START_HUE = 85;                       //Green(ish) MUST be a number < END_HUE
const int END_HUE = 255;                        //Red
const int HUE_INCREMENT = END_HUE - START_HUE;  // Used to change color based on selected turn time

// Timing settings
long turnTime = 0;                                  // Manages the turnTime - determined in setup(), used in loop()
const unsigned long HOLD_TO_FINISH_INTERVAL = 500;  // How long to hold button when making final selection for time
unsigned long previousButtonHoldTime = 0;           // Used for determining long button hold time

// Flag set in ISR to indicate a button press
volatile boolean buttonPressed = false;


/************************************************************
   ISR: Action to take on Rotary Endcode switch press
 ***********************************************************/
void buttonPress() {
  buttonPressed = true;  //flag that button was pressed
}

/************************************************************
   ISR: Get rotary encoder position
 ***********************************************************/
void ICACHE_RAM_ATTR readEncoderStatus() {
  byte A_Output = digitalRead(ROTARY_ENC_PIN_A);
  byte B_Output = digitalRead(ROTARY_ENC_PIN_B);
  byte currState = (A_Output * 2) + B_Output;

  if (currState != lastState) {

    if (currState == aState[position + 1]) {
      position++;
      //Serial.println(position);
      if (position == 8) {
        count++;
        position = 4;
        encoderStatus = TURN_CW;
      }
    }
    else if (currState == aState[position - 1]) {
      position--;
      //Serial.println(position);
      if (position == 0) {
        count--;
        position = 4;
        encoderStatus = TURN_CCW;
      }
    }

    lastState = currState;
  }
}

/************************************************************
   LED Effect: Set all LEDs same color
 ***********************************************************/
void changeAllColorTo(int h, int s = 255, int v = 255) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(h, s, v);
  }
}

/************************************************************
   LED Effect: Blink Red
 ***********************************************************/
void blinkRed() {

  changeAllColorTo(255);
  FastLED.show();
  delay(300);

  changeAllColorTo(0, 0, 0);
  FastLED.show();
  delay(300);
}

/************************************************************
   LED Effect: Fade All
 ***********************************************************/
void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(100);
  }
}

/************************************************************
   LED Effect: Half Cyclon
 ***********************************************************/
void halfCylon(CHSV color) {
  static uint8_t hue = 0;

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
    FastLED.show();
    fadeall();
    delay(10);
  }
}

/************************************************************
   Signal a time unit has been selected
 ***********************************************************/
void signalTimeSelected(CHSV color) {
  for (int i = 0; i < 10; i++) {
    halfCylon(color);
  }
}

/************************************************************
   Select Turn Time Routine
 ***********************************************************/
int selectTime(CHSV uncountedColor, CHSV countedColor) {

  int tempCount = 0;                       // This tracks the button presses, each button press is a time unit
  boolean update = true;

  while (true) {

    // Get current position from rotary encoder
    if (encoderStatus != NO_CHANGE) {

      //Constrain count from 0-NUM_LEDs
      //Note: -> //constrain(count, 0, NUM_LEDS); is not working...
      if (count > NUM_LEDS) {
        count = NUM_LEDS;
      } else if (count < 0) {
        count = 0;
      }

      noInterrupts();
      tempCount = count;
      interrupts();

      encoderStatus = NO_CHANGE;
      update = true;
    }

    if (update) {
      // Set color of each LED based on counted or uncounted
      for (int i = 0; i < NUM_LEDS; i++) {
        //leds[NUM_LEDS - 1 - i] = i < tempCount  // Comment next line and uncomment this line for reverse LED installation
        leds[i] = i < tempCount                 
                  ? countedColor
                  : uncountedColor;
      }
      FastLED.show();
      update = false;
    }

    // Check if button held
    if (!digitalRead(ROTARY_ENC_SWITCH)) {

      signalTimeSelected(countedColor);  //Display cylon effect to show selection has been made
      buttonPressed = false;             // reset ISR button flag
      count = 0;                         // Reset count
      break;

    }
  }

  return tempCount;  //Returns the number of times the button was pressed (less the long hold)
}


/************************************************************
   Compute Turn Time
 ***********************************************************/
long computeTurnTime(long s = 0, long m = 0) {

  s = s * 5 * 1000;  // 5 seconds for every count
  m = m * 60 * 1000;  // 1 minute for every count

  return s + m;  // in milliseconds
}

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(84);

  pinMode(ROTARY_ENC_SWITCH, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ROTARY_ENC_SWITCH), buttonPress, RISING);
  attachInterrupt(digitalPinToInterrupt(ROTARY_ENC_PIN_A), readEncoderStatus, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_ENC_PIN_B), readEncoderStatus, CHANGE);

}

void loop() {

  static bool updateTime = true;
  static unsigned long timerIncrement = 0;
  static unsigned long previousMillisTimer = 0;
  unsigned long currentMillisTimer = millis();

  static int hue = START_HUE;
  boolean overTime = false;

  // Set turn time.  Select seconds, then minutes.
  if (updateTime) {

    long secondsCount = selectTime(UNCOUNTED_COLOR, SECONDS_COUNTED_COLOR);
    long minutesCount = selectTime(UNCOUNTED_COLOR, MINUTES_COUNTED_COLOR);

    turnTime = computeTurnTime(secondsCount, minutesCount); // get total turn time
    timerIncrement = turnTime / HUE_INCREMENT; // How much time to wait between each hue change

    hue = START_HUE;
    updateTime = false;
  }

  // As turn time elapses, show a fade from Green to Blue to Red
  if (currentMillisTimer - previousMillisTimer > timerIncrement) {
    previousMillisTimer = currentMillisTimer;
    changeAllColorTo(hue);
    FastLED.show();
    hue++;
  }

  // If long button press, set flag for new turn time
  unsigned long startPressButton = millis();

  while (!digitalRead(ROTARY_ENC_SWITCH)) {
    unsigned long currentPressButton = millis();
    if (currentPressButton - startPressButton > HOLD_TO_FINISH_INTERVAL) {
      updateTime = true;
      signalTimeSelected(RESET_COLOR);  //Display cylon effect to show selection has been made
    }
  }

  // If short button press, reset hue
  if (buttonPressed) {
    hue = START_HUE;
    buttonPressed = false;  // Reset button flag
  }

  // If hue increment all the way to end, LEDs go into "overtime" mode
  if (hue == END_HUE) {
    hue = START_HUE;
    overTime = true;
  }

  // Over Time Mode, All LEDs blink
  while (overTime) {

    blinkRed();

    // If button flag was set in ISR then exit
    if (buttonPressed) {
      buttonPressed = false;
      break;
    }
  }
}  // End loop()
