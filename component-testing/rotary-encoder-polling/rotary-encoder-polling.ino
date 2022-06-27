
/*------------------------------------------------------------------------------------
   RotaryEncoder_Lesson1

   This is a sample program to demonstrate the use of an incremental rotary encoder.
   It uses a simple method of polling the encoder's outputs and displaying the results
   on an Arduino monitor.
  ----------------------------------------------------------------------------------- */

const unsigned int ROTARY_ENC_PIN_A  = 33;
const unsigned int ROTARY_ENC_PIN_B  = 34;
const unsigned int ROTARY_ENC_SWITCH = 21;

// Direction    ┌─  ccw  ─┐  N  ┌─  cw  ─┐
// Index         0  1  2  3  4  5  6  7  8
byte aState[] = {3, 2, 0, 1, 3, 2, 0, 1, 3};
byte lastState = 3;
int count = 0;
unsigned int index_r = 4;

// The setup function runs once and configures the 3 pins used
// to process the signals received from the rotary encoder
void setup()
{
  Serial.begin(9600);
  Serial.println("\nStarting Rotary Encoder...");

  pinMode(ROTARY_ENC_PIN_A,  INPUT);
  pinMode(ROTARY_ENC_PIN_B,  INPUT);
  pinMode(ROTARY_ENC_SWITCH, INPUT_PULLUP);
}

void loop()
{
  byte currState;

  // We form our current state value by assigning the signal
  // from pin A to bit1 and signal from pin B to bit0
  currState = (digitalRead(ROTARY_ENC_PIN_A) * 2) + digitalRead(ROTARY_ENC_PIN_B);
  if (currState != lastState)
  {
    // New state detected.
    // Check if we're moving in the clockwise direction
    if (currState == aState[index_r + 1])
    {
      index_r++;
      Serial.println(index_r);
      if (8 == index_r)
      {
        // Successfully completed the sequence of 3->2->0->1->3 for cw.
        // Increment the count and reset the index to the nominal setting.
        count++;
        index_r = 4;
        Serial.print("CW   ");
        Serial.println(count);
      }
    }
    // Check if we're moving in the counterclockwise direction
    else if (currState == aState[index_r - 1])
    {
      index_r--;
      Serial.println(index_r);
      if (0 == index_r)
      {
        // Successfully completed the sequence of 3->1->0->2->3 for ccw.
        // Decrement the count and reset the index to the nominal setting.
        count--;
        index_r = 4;
        Serial.print("CCW  ");
        Serial.println(count);
      }
    }
    lastState = currState;
  }

/*
 * Delays for demonstration
 */
  //delay(5);
  
  if (!digitalRead(ROTARY_ENC_SWITCH))
  {
    // Switch was pushed (active LOW).
    // Reset the count and debounce the switch contacts.
    count = 0;
    Serial.println("Switch pushed - reset count to 0");
    delay(100);
    while (!digitalRead(ROTARY_ENC_SWITCH));
    delay(100);
  }
}
