#include <Arduino.h>
#include "DigiKeyboard.h"

const uint8_t KEY_MUTE = 127;
const uint8_t KEY_VOLUME_UP = 128;
const uint8_t KEY_VOLUME_DOWN = 129;

const uint8_t KNOB_QUAD_LEFT = 1u;
const uint8_t KNOB_QUAD_RIGHT = 2u;  
const uint8_t BUTTON_PIN = 5u;

const uint16_t DEBOUNCE_DURATION_US = 1500u; /* 0.5 milliseconds */


void setup() {
    
    pinMode(KNOB_QUAD_LEFT, INPUT);
    pinMode(KNOB_QUAD_RIGHT, INPUT);
    pinMode(BUTTON_PIN, INPUT);

    digitalWrite(KNOB_QUAD_LEFT, HIGH);
    digitalWrite(KNOB_QUAD_RIGHT, HIGH);
    digitalWrite(BUTTON_PIN, HIGH);
    DigiKeyboard.delay(100);
}

// debounce a pin, used for mute button
// increase DEBOUNCE_DURATION_US if sometimes pressing mute button
// does the action too many times.
void debounce(byte pin, boolean state) {
  unsigned long time_now = micros();
  unsigned long time_since_wrong = time_now;
  
  while (time_since_wrong + DEBOUNCE_DURATION_US > time_now) {
    DigiKeyboard.update(); // keep updating the keyboard so we don't crash
    time_now = micros();
    
    if (digitalRead(pin) != state) {
      time_since_wrong = time_now;
    }
  }
}


void loop() {

    alter_volume(read_knob());

}



void alter_volume(int amount) {
  while (amount) {
    if (amount > 0) {
      DigiKeyboard.sendKeyStroke(KEY_VOLUME_UP);
      amount -= 1;
    } else if (amount < 0) {
      DigiKeyboard.sendKeyStroke(KEY_VOLUME_DOWN);
      amount += 1;
    }
  }
}

// returns the bits from the two knob wires as a 2-bit number
byte knob_bits() {
  return digitalRead(KNOB_QUAD_LEFT) << 1 | digitalRead(KNOB_QUAD_RIGHT);
}

char read_knob() {
  static byte previous_state; // these two variables keep their values between function calls
  static byte armed;
  byte state = knob_bits();
  char result = 0;
  
  // to ensure bouncy mechanical switch stuff only makes one positive or negative impulse for each mechanical click
  // we "arm" it when it gets half way through a click
  if (state == 0b00) {
    armed = true; // allow one click result
  }
  
  // then when it finishes a click, we consume the one "armed" token and deliver the result just once to the caller
  if (armed == true && state == 0b11) {
    armed = false; // consumed.
    if (previous_state == 0b01) {
      result = -1;
    } else if (previous_state == 0b10) {
      result = +1;
    }
  }
   
  previous_state = state;
  return result;
}