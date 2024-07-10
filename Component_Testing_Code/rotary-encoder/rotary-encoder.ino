/* TODO
 * 
 *  Attach interrupts in setup (pin #s matter!)
 *  Add Interrupt Service Routine (ISR)
 *    Define flags
 *    Remove serial prints
 *  Check for flags in loop() 
 *    Qualify variables as volatile which are shared by ISRs
 *    Protect shared variable from being clobbered by ISR
 */

const unsigned int ROTARY_ENC_PIN_A = 33;//D2;
const unsigned int ROTARY_ENC_PIN_B = 34;//D3;
const unsigned int ROTARY_ENC_SWITCH = 21; //D4;



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

//void ARDUINO_ISR_ATTR readEncoderStatus() {
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

void setup() {
  Serial.begin(9600);
  
  attachInterrupt(digitalPinToInterrupt(ROTARY_ENC_PIN_A), readEncoderStatus, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_ENC_PIN_B), readEncoderStatus, CHANGE);
  
  pinMode(ROTARY_ENC_SWITCH, INPUT_PULLUP);
}

void loop() {

  
  if (encoderStatus != NO_CHANGE) {
    int tempCount;
    
    noInterrupts();
    tempCount = count;
    interrupts();
    
    Serial.print(encoderStatus == TURN_CW ? "CW   " : "CCW  ");
    Serial.println(tempCount);  // Use the saved variable
    encoderStatus = NO_CHANGE;
  }

  /***********  Rotary Encoder Switch **************/

  if (!digitalRead(ROTARY_ENC_SWITCH)) {

    count = 0;
    Serial.println("I can't believe you pressed that! Count reset to 0");

    //debounce switch press
    delay(100);
    while (!digitalRead(ROTARY_ENC_SWITCH));
    delay(100);
  }
}
